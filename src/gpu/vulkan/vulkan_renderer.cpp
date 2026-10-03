#include "nisaba/gpu/vulkan_renderer.hpp"
#include "nisaba/gpu/vulkan_surface.hpp"
#include "vulkan_shaders.hpp"
#include <iostream>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <array>
#include <vector>

namespace nisaba::gpu {

namespace {

enum ShaderType {
    ShaderFillGrad = 0,
    ShaderFillImg  = 1,
    ShaderSimple   = 2,
    ShaderImg      = 3,
    ShaderSolid    = 4
};

enum class CallType {
    None = 0,
    Fill,
    ConvexFill,
    Stroke,
    Triangles
};

struct Call {
    CallType type{CallType::None};
    int image{0};
    int pathOffset{0};
    int pathCount{0};
    int triangleOffset{0};
    int triangleCount{0};
    int uniformOffset{0};
    CompositeOperationState compOp{};
    int scissorRect[4]{-1, -1, -1, -1};
};

struct VkPath {
    int fillOffset{0};
    int fillCount{0};
    int strokeOffset{0};
    int strokeCount{0};
};

// std140 layout matching NisabaShaderParams in SPIR-V
struct NisabaShaderUniforms {
    float scissorMat[12]; // 3 x vec4
    float paintMat[12];   // 3 x vec4
    Color innerCol;       // 4 floats (premultiplied)
    Color outerCol;       // 4 floats (premultiplied)
    float scissorExt[2];
    float scissorScale[2];
    float extent[2];
    float radius;
    float feather;
    float strokeMult;
    float strokeThr;
    int texType;
    int type;
};

using FragUniforms = NisabaShaderUniforms;

static void xformToMat3x4(float* m3, const Transform2D& t) {
    m3[0]  = t[0]; m3[1]  = t[1]; m3[2]  = 0.0f; m3[3]  = 0.0f;
    m3[4]  = t[2]; m3[5]  = t[3]; m3[6]  = 0.0f; m3[7]  = 0.0f;
    m3[8]  = t[4]; m3[9]  = t[5]; m3[10] = 1.0f; m3[11] = 0.0f;
}

struct VulkanTexture {
    int id{0};
    VkImage image{VK_NULL_HANDLE};
    VkDeviceMemory memory{VK_NULL_HANDLE};
    VkImageView view{VK_NULL_HANDLE};
    VkSampler sampler{VK_NULL_HANDLE};
    VkDescriptorSet descriptorSet{VK_NULL_HANDLE};
    int width{0};
    int height{0};
    TextureType type{TextureType::RGBA};
    int flags{0};
    bool ownsImage{true};
    VkImageLayout layout{VK_IMAGE_LAYOUT_UNDEFINED};
};

} // namespace

struct VulkanRenderer::Impl {
    VkDevice device{VK_NULL_HANDLE};
    VkPhysicalDevice physicalDevice{VK_NULL_HANDLE};
    VkQueue queue{VK_NULL_HANDLE};
    uint32_t queueFamilyIndex{0};
    VkRenderPass renderPass{VK_NULL_HANDLE};
    VkSampleCountFlagBits samples{VK_SAMPLE_COUNT_1_BIT};
    int flags{0};

    float view[2]{0.0f, 0.0f};

    VkShaderModule vertShaderModule{VK_NULL_HANDLE};
    VkShaderModule fragShaderModule{VK_NULL_HANDLE};

    VkDescriptorSetLayout descSetLayout{VK_NULL_HANDLE};
    VkPipelineLayout pipelineLayout{VK_NULL_HANDLE};
    VkDescriptorPool descPool{VK_NULL_HANDLE};

    // Pipelines
    VkPipeline pipelineConvexFill{VK_NULL_HANDLE};
    VkPipeline pipelineFillStencil{VK_NULL_HANDLE};
    VkPipeline pipelineFillCover{VK_NULL_HANDLE};
    VkPipeline pipelineStroke{VK_NULL_HANDLE};
    VkPipeline pipelineTriangles{VK_NULL_HANDLE};

    // Vertex & Uniform Memory Buffers
    VkBuffer vertBuf{VK_NULL_HANDLE};
    VkDeviceMemory vertMem{VK_NULL_HANDLE};
    size_t vertBufCap{0};

    VkBuffer fragBuf{VK_NULL_HANDLE};
    VkDeviceMemory fragMem{VK_NULL_HANDLE};
    size_t fragBufCap{0};
    size_t fragAlign{256};
    size_t fragSize{256};

    // Textures
    std::vector<VulkanTexture> textures;
    int nextTextureId{1};
    int dummyTex{0};

    // Batch recording state
    std::vector<Vertex> verts;
    std::vector<VkPath> paths;
    std::vector<Call> calls;
    std::vector<uint8_t> uniforms;

    VulkanSurface* targetSurface{nullptr};
    VkCommandBuffer externalCmd{VK_NULL_HANDLE};

    VulkanTexture* allocTexture() {
        for (auto& t : textures) {
            if (t.id == 0) {
                t.id = nextTextureId++;
                return &t;
            }
        }
        VulkanTexture tex;
        tex.id = nextTextureId++;
        textures.push_back(tex);
        return &textures.back();
    }

    VulkanTexture* findTexture(int id) {
        for (auto& t : textures) {
            if (t.id == id) return &t;
        }
        return nullptr;
    }

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
        for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i) {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }
        return 0;
    }

    bool createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                      VkBuffer& outBuf, VkDeviceMemory& outMem) {
        VkBufferCreateInfo bufInfo{};
        bufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufInfo.size = size;
        bufInfo.usage = usage;
        bufInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        if (vkCreateBuffer(device, &bufInfo, nullptr, &outBuf) != VK_SUCCESS) return false;

        VkMemoryRequirements memReqs;
        vkGetBufferMemoryRequirements(device, outBuf, &memReqs);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memReqs.size;
        allocInfo.memoryTypeIndex = findMemoryType(memReqs.memoryTypeBits, properties);

        if (vkAllocateMemory(device, &allocInfo, nullptr, &outMem) != VK_SUCCESS) {
            vkDestroyBuffer(device, outBuf, nullptr);
            outBuf = VK_NULL_HANDLE;
            return false;
        }
        vkBindBufferMemory(device, outBuf, outMem, 0);
        return true;
    }

    FragUniforms* fragUniformPtr(int offset) {
        return reinterpret_cast<FragUniforms*>(&uniforms[offset]);
    }

    bool getHardwareScissor(const Scissor& scissor, int outRect[4]) const {
        if (scissor.extent[0] < -0.5f || scissor.extent[1] < -0.5f) {
            outRect[0] = -1; outRect[1] = -1; outRect[2] = -1; outRect[3] = -1;
            return false;
        }
        if (std::abs(scissor.xform[1]) > 1e-4f || std::abs(scissor.xform[2]) > 1e-4f) {
            outRect[0] = -1; return false;
        }
        float sx = scissor.xform[0];
        float sy = scissor.xform[3];
        if (sx <= 0.0f || sy <= 0.0f) {
            outRect[0] = -1; return false;
        }
        float cx = scissor.xform[4];
        float cy = scissor.xform[5];
        float hw = scissor.extent[0] * sx;
        float hh = scissor.extent[1] * sy;

        float minX = cx - hw;
        float maxX = cx + hw;
        float minY = cy - hh;
        float maxY = cy + hh;

        int ix = static_cast<int>(std::floor(minX));
        int iy = static_cast<int>(std::floor(minY));
        int iw = static_cast<int>(std::ceil(maxX - minX));
        int ih = static_cast<int>(std::ceil(maxY - minY));

        outRect[0] = std::max(0, ix);
        outRect[1] = std::max(0, iy);
        outRect[2] = std::max(0, iw);
        outRect[3] = std::max(0, ih);
        return true;
    }

    bool convertPaint(FragUniforms* frag, const Paint& paint, const Scissor& scissor,
                      float width, float fringe, float strokeThr, bool hasHwScissor = false) {
        *frag = FragUniforms{};

        frag->innerCol = paint.innerColor.premultiplied();
        frag->outerCol = paint.outerColor.premultiplied();

        if (hasHwScissor || scissor.extent[0] < -0.5f || scissor.extent[1] < -0.5f) {
            std::memset(frag->scissorMat, 0, sizeof(frag->scissorMat));
            frag->scissorExt[0] = -1.0f;
            frag->scissorExt[1] = -1.0f;
            frag->scissorScale[0] = 1.0f;
            frag->scissorScale[1] = 1.0f;
        } else {
            Transform2D invxform;
            scissor.xform.inverse(invxform);
            xformToMat3x4(frag->scissorMat, invxform);
            frag->scissorExt[0] = scissor.extent[0];
            frag->scissorExt[1] = scissor.extent[1];
            frag->scissorScale[0] = std::sqrt(scissor.xform[0] * scissor.xform[0] + scissor.xform[2] * scissor.xform[2]) / fringe;
            frag->scissorScale[1] = std::sqrt(scissor.xform[1] * scissor.xform[1] + scissor.xform[3] * scissor.xform[3]) / fringe;
        }

        frag->extent[0] = paint.extent[0];
        frag->extent[1] = paint.extent[1];
        frag->strokeMult = (width * 0.5f + fringe * 0.5f) / fringe;
        frag->strokeThr = strokeThr;

        Transform2D invxform;
        if (paint.image != 0) {
            VulkanTexture* tex = findTexture(paint.image);
            if (tex == nullptr) return false;

            if ((tex->flags & ImageFlags::ImageFlipY) != 0) {
                Transform2D m1 = Transform2D::translate(0.0f, frag->extent[1] * 0.5f).multiply(paint.xform);
                Transform2D m2 = Transform2D::scale(1.0f, -1.0f).multiply(m1);
                Transform2D m3 = Transform2D::translate(0.0f, -frag->extent[1] * 0.5f).multiply(m2);
                m3.inverse(invxform);
            } else {
                paint.xform.inverse(invxform);
            }
            frag->type = ShaderFillImg;

            if (tex->type == TextureType::RGBA) {
                frag->texType = (tex->flags & ImageFlags::ImagePremultiplied) ? 0 : 1;
            } else {
                frag->texType = 2;
            }
            xformToMat3x4(frag->paintMat, invxform);
        } else if (paint.extent[0] == 0.0f && paint.extent[1] == 0.0f && paint.radius == 0.0f &&
                   paint.innerColor.r == paint.outerColor.r &&
                   paint.innerColor.g == paint.outerColor.g &&
                   paint.innerColor.b == paint.outerColor.b &&
                   paint.innerColor.a == paint.outerColor.a) {
            frag->type = ShaderSolid;
            frag->radius = 0.0f;
            frag->feather = 1.0f;
            frag->innerCol = Color(1.0f, 1.0f, 1.0f, 1.0f);
            frag->outerCol = Color(1.0f, 1.0f, 1.0f, 1.0f);
        } else {
            frag->type = ShaderFillGrad;
            frag->radius = paint.radius;
            frag->feather = paint.feather;
            paint.xform.inverse(invxform);
            xformToMat3x4(frag->paintMat, invxform);
        }

        return true;
    }

    bool createPipelines(VkRenderPass rp) {
        if (rp == VK_NULL_HANDLE || pipelineConvexFill != VK_NULL_HANDLE || pipelineLayout == VK_NULL_HANDLE) return true;
        renderPass = rp;

        VkPipelineShaderStageCreateInfo shaderStages[2]{};
        shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        shaderStages[0].module = vertShaderModule;
        shaderStages[0].pName = "main";

        shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        shaderStages[1].module = fragShaderModule;
        shaderStages[1].pName = "main";

        VkVertexInputBindingDescription bindingDesc{};
        bindingDesc.binding = 0;
        bindingDesc.stride = sizeof(Vertex);
        bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        std::array<VkVertexInputAttributeDescription, 3> attribDescs{};
        attribDescs[0].location = 0;
        attribDescs[0].binding = 0;
        attribDescs[0].format = VK_FORMAT_R32G32_SFLOAT;
        attribDescs[0].offset = 0;

        attribDescs[1].location = 1;
        attribDescs[1].binding = 0;
        attribDescs[1].format = VK_FORMAT_R32G32_SFLOAT;
        attribDescs[1].offset = 8;

        attribDescs[2].location = 2;
        attribDescs[2].binding = 0;
        attribDescs[2].format = VK_FORMAT_R8G8B8A8_UNORM;
        attribDescs[2].offset = 16;

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.pVertexBindingDescriptions = &bindingDesc;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribDescs.size());
        vertexInputInfo.pVertexAttributeDescriptions = attribDescs.data();

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_NONE;
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = samples;

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                              VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_TRUE;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;

        std::array<VkDynamicState, 2> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };
        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        VkPipelineDepthStencilStateCreateInfo depthStencil{};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = VK_FALSE;
        depthStencil.depthWriteEnable = VK_FALSE;
        depthStencil.depthCompareOp = VK_COMPARE_OP_ALWAYS;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.stencilTestEnable = VK_FALSE;

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = pipelineLayout;
        pipelineInfo.renderPass = renderPass;
        pipelineInfo.subpass = 0;

        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
        vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipelineConvexFill);

        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
        vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipelineStroke);

        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipelineTriangles);

        return true;
    }
};

VulkanRenderer::VulkanRenderer(
    VkDevice device,
    VkPhysicalDevice physicalDevice,
    VkQueue queue,
    uint32_t queueFamilyIndex,
    VkRenderPass renderPass,
    VkSampleCountFlagBits samples,
    int flags
) : m_impl(std::make_unique<Impl>()) {
    m_impl->device = device;
    m_impl->physicalDevice = physicalDevice;
    m_impl->queue = queue;
    m_impl->queueFamilyIndex = queueFamilyIndex;
    m_impl->renderPass = renderPass;
    m_impl->samples = samples;
    m_impl->flags = flags;
}

VulkanRenderer::~VulkanRenderer() {
    shutdown();
}

bool VulkanRenderer::edgeAntiAlias() const {
    return (m_impl->flags & CreateFlags::Antialias) != 0;
}

bool VulkanRenderer::init() {
    if (m_impl->device == VK_NULL_HANDLE) return false;
    VkDevice dev = m_impl->device;

    // 1. Query physical device uniform buffer alignment requirement
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(m_impl->physicalDevice, &props);
    m_impl->fragAlign = std::max<size_t>(props.limits.minUniformBufferOffsetAlignment, 16);
    m_impl->fragSize = (sizeof(FragUniforms) + m_impl->fragAlign - 1) & ~(m_impl->fragAlign - 1);

    // 2. Create Shader Modules from pre-compiled SPIR-V 1.0
    VkShaderModuleCreateInfo vertModInfo{};
    vertModInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    vertModInfo.codeSize = vulkan::NISABA_VERT_SPV_SIZE;
    vertModInfo.pCode = vulkan::NISABA_VERT_SPV;
    if (vkCreateShaderModule(dev, &vertModInfo, nullptr, &m_impl->vertShaderModule) != VK_SUCCESS) {
        return false;
    }

    VkShaderModuleCreateInfo fragModInfo{};
    fragModInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    fragModInfo.codeSize = vulkan::NISABA_FRAG_SPV_SIZE;
    fragModInfo.pCode = vulkan::NISABA_FRAG_SPV;
    if (vkCreateShaderModule(dev, &fragModInfo, nullptr, &m_impl->fragShaderModule) != VK_SUCCESS) {
        return false;
    }

    // 3. Create Descriptor Set Layout
    std::array<VkDescriptorSetLayoutBinding, 2> bindings{};
    // Binding 0: Dynamic Uniform Buffer for shader params
    bindings[0].binding = 0;
    bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    bindings[0].descriptorCount = 1;
    bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    // Binding 1: Texture Sampler
    bindings[1].binding = 1;
    bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[1].descriptorCount = 1;
    bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(dev, &layoutInfo, nullptr, &m_impl->descSetLayout) != VK_SUCCESS) {
        return false;
    }

    // 4. Create Pipeline Layout with Push Constants (viewSize)
    VkPushConstantRange pushRange{};
    pushRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pushRange.offset = 0;
    pushRange.size = sizeof(float) * 2; // viewSize

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &m_impl->descSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushRange;

    if (vkCreatePipelineLayout(dev, &pipelineLayoutInfo, nullptr, &m_impl->pipelineLayout) != VK_SUCCESS) {
        return false;
    }

    // 5. Create Descriptor Pool
    std::array<VkDescriptorPoolSize, 2> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    poolSizes[0].descriptorCount = 128;
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = 512;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.maxSets = 512;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();

    if (vkCreateDescriptorPool(dev, &poolInfo, nullptr, &m_impl->descPool) != VK_SUCCESS) {
        return false;
    }

    // 6. Pre-allocate dynamic buffers
    m_impl->vertBufCap = 65536 * sizeof(Vertex);
    m_impl->createBuffer(m_impl->vertBufCap, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                         m_impl->vertBuf, m_impl->vertMem);

    m_impl->fragBufCap = 65536;
    m_impl->createBuffer(m_impl->fragBufCap, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                         m_impl->fragBuf, m_impl->fragMem);

    // 7. Create Dummy 1x1 White Texture
    const unsigned char whitePixel[4] = {255, 255, 255, 255};
    m_impl->dummyTex = createTexture(TextureType::RGBA, 1, 1, 0, whitePixel);

    if (m_impl->renderPass != VK_NULL_HANDLE) {
        m_impl->createPipelines(m_impl->renderPass);
    }

    return true;
}

void VulkanRenderer::shutdown() {
    if (!m_impl || m_impl->device == VK_NULL_HANDLE) return;
    VkDevice dev = m_impl->device;
    vkDeviceWaitIdle(dev);

    cancel();

    for (auto& t : m_impl->textures) {
        if (t.id != 0) {
            deleteTexture(t.id);
        }
    }
    m_impl->textures.clear();

    if (m_impl->pipelineConvexFill != VK_NULL_HANDLE) {
        vkDestroyPipeline(dev, m_impl->pipelineConvexFill, nullptr);
        m_impl->pipelineConvexFill = VK_NULL_HANDLE;
    }
    if (m_impl->pipelineFillStencil != VK_NULL_HANDLE) {
        vkDestroyPipeline(dev, m_impl->pipelineFillStencil, nullptr);
        m_impl->pipelineFillStencil = VK_NULL_HANDLE;
    }
    if (m_impl->pipelineFillCover != VK_NULL_HANDLE) {
        vkDestroyPipeline(dev, m_impl->pipelineFillCover, nullptr);
        m_impl->pipelineFillCover = VK_NULL_HANDLE;
    }
    if (m_impl->pipelineStroke != VK_NULL_HANDLE) {
        vkDestroyPipeline(dev, m_impl->pipelineStroke, nullptr);
        m_impl->pipelineStroke = VK_NULL_HANDLE;
    }
    if (m_impl->pipelineTriangles != VK_NULL_HANDLE) {
        vkDestroyPipeline(dev, m_impl->pipelineTriangles, nullptr);
        m_impl->pipelineTriangles = VK_NULL_HANDLE;
    }

    if (m_impl->pipelineLayout != VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(dev, m_impl->pipelineLayout, nullptr);
        m_impl->pipelineLayout = VK_NULL_HANDLE;
    }
    if (m_impl->descSetLayout != VK_NULL_HANDLE) {
        vkDestroyDescriptorSetLayout(dev, m_impl->descSetLayout, nullptr);
        m_impl->descSetLayout = VK_NULL_HANDLE;
    }
    if (m_impl->descPool != VK_NULL_HANDLE) {
        vkDestroyDescriptorPool(dev, m_impl->descPool, nullptr);
        m_impl->descPool = VK_NULL_HANDLE;
    }

    if (m_impl->vertBuf != VK_NULL_HANDLE) {
        vkDestroyBuffer(dev, m_impl->vertBuf, nullptr);
        m_impl->vertBuf = VK_NULL_HANDLE;
    }
    if (m_impl->vertMem != VK_NULL_HANDLE) {
        vkFreeMemory(dev, m_impl->vertMem, nullptr);
        m_impl->vertMem = VK_NULL_HANDLE;
    }
    if (m_impl->fragBuf != VK_NULL_HANDLE) {
        vkDestroyBuffer(dev, m_impl->fragBuf, nullptr);
        m_impl->fragBuf = VK_NULL_HANDLE;
    }
    if (m_impl->fragMem != VK_NULL_HANDLE) {
        vkFreeMemory(dev, m_impl->fragMem, nullptr);
        m_impl->fragMem = VK_NULL_HANDLE;
    }

    if (m_impl->vertShaderModule != VK_NULL_HANDLE) {
        vkDestroyShaderModule(dev, m_impl->vertShaderModule, nullptr);
        m_impl->vertShaderModule = VK_NULL_HANDLE;
    }
    if (m_impl->fragShaderModule != VK_NULL_HANDLE) {
        vkDestroyShaderModule(dev, m_impl->fragShaderModule, nullptr);
        m_impl->fragShaderModule = VK_NULL_HANDLE;
    }

    m_impl->device = VK_NULL_HANDLE;
}

void VulkanRenderer::viewport(float width, float height, float devicePixelRatio) {
    (void)devicePixelRatio;
    m_impl->view[0] = width;
    m_impl->view[1] = height;
}

void VulkanRenderer::cancel() {
    m_impl->verts.clear();
    m_impl->paths.clear();
    m_impl->calls.clear();
    m_impl->uniforms.clear();
}

int VulkanRenderer::createTexture(TextureType type, int width, int height, int imageFlags, const unsigned char* data) {
    VulkanTexture* tex = m_impl->allocTexture();
    if (!tex) return 0;

    tex->type = type;
    tex->width = width;
    tex->height = height;
    tex->flags = imageFlags;
    tex->ownsImage = true;

    VkFormat format = (type == TextureType::RGBA) ? VK_FORMAT_R8G8B8A8_UNORM : VK_FORMAT_R8_UNORM;
    VkDevice dev = m_impl->device;

    // Create Image
    VkImageCreateInfo imgInfo{};
    imgInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imgInfo.imageType = VK_IMAGE_TYPE_2D;
    imgInfo.extent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1};
    imgInfo.mipLevels = 1;
    imgInfo.arrayLayers = 1;
    imgInfo.format = format;
    imgInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imgInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imgInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    imgInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imgInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(dev, &imgInfo, nullptr, &tex->image) != VK_SUCCESS) {
        tex->id = 0; return 0;
    }

    VkMemoryRequirements memReqs;
    vkGetImageMemoryRequirements(dev, tex->image, &memReqs);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReqs.size;
    allocInfo.memoryTypeIndex = m_impl->findMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if (vkAllocateMemory(dev, &allocInfo, nullptr, &tex->memory) != VK_SUCCESS) {
        vkDestroyImage(dev, tex->image, nullptr);
        tex->id = 0; return 0;
    }
    vkBindImageMemory(dev, tex->image, tex->memory, 0);

    // Create Image View
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = tex->image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(dev, &viewInfo, nullptr, &tex->view) != VK_SUCCESS) {
        vkDestroyImage(dev, tex->image, nullptr);
        vkFreeMemory(dev, tex->memory, nullptr);
        tex->id = 0; return 0;
    }

    // Create Sampler
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = (imageFlags & ImageFlags::ImageNearest) ? VK_FILTER_NEAREST : VK_FILTER_LINEAR;
    samplerInfo.minFilter = samplerInfo.magFilter;
    samplerInfo.addressModeU = (imageFlags & ImageFlags::ImageRepeatX) ? VK_SAMPLER_ADDRESS_MODE_REPEAT : VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = (imageFlags & ImageFlags::ImageRepeatY) ? VK_SAMPLER_ADDRESS_MODE_REPEAT : VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.maxLod = 0.0f;

    if (vkCreateSampler(dev, &samplerInfo, nullptr, &tex->sampler) != VK_SUCCESS) {
        vkDestroyImageView(dev, tex->view, nullptr);
        vkDestroyImage(dev, tex->image, nullptr);
        vkFreeMemory(dev, tex->memory, nullptr);
        tex->id = 0; return 0;
    }

    // Allocate Descriptor Set
    VkDescriptorSetAllocateInfo dsAlloc{};
    dsAlloc.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    dsAlloc.descriptorPool = m_impl->descPool;
    dsAlloc.descriptorSetCount = 1;
    dsAlloc.pSetLayouts = &m_impl->descSetLayout;
    vkAllocateDescriptorSets(dev, &dsAlloc, &tex->descriptorSet);

    // Update Descriptor Set with Uniform buffer + Image sampler
    VkDescriptorBufferInfo dbi{};
    dbi.buffer = m_impl->fragBuf;
    dbi.offset = 0;
    dbi.range = sizeof(FragUniforms);

    VkDescriptorImageInfo dii{};
    dii.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    dii.imageView = tex->view;
    dii.sampler = tex->sampler;

    std::array<VkWriteDescriptorSet, 2> writes{};
    writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[0].dstSet = tex->descriptorSet;
    writes[0].dstBinding = 0;
    writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    writes[0].descriptorCount = 1;
    writes[0].pBufferInfo = &dbi;

    writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[1].dstSet = tex->descriptorSet;
    writes[1].dstBinding = 1;
    writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writes[1].descriptorCount = 1;
    writes[1].pImageInfo = &dii;

    vkUpdateDescriptorSets(dev, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);

    if (data != nullptr) {
        updateTexture(tex->id, 0, 0, width, height, data);
    }

    return tex->id;
}

bool VulkanRenderer::updateTexture(int image, int x, int y, int width, int height, const unsigned char* data) {
    VulkanTexture* tex = m_impl->findTexture(image);
    if (!tex || !data) return false;

    size_t bpp = (tex->type == TextureType::RGBA) ? 4 : 1;
    VkDeviceSize uploadSize = static_cast<VkDeviceSize>(width) * height * bpp;

    VkBuffer stagingBuf = VK_NULL_HANDLE;
    VkDeviceMemory stagingMem = VK_NULL_HANDLE;
    if (!m_impl->createBuffer(uploadSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                              stagingBuf, stagingMem)) {
        return false;
    }

    void* mapped = nullptr;
    vkMapMemory(m_impl->device, stagingMem, 0, uploadSize, 0, &mapped);
    if (width == tex->width && height == tex->height && x == 0 && y == 0) {
        std::memcpy(mapped, data, uploadSize);
    } else {
        const uint8_t* src_ptr = data + (static_cast<size_t>(y) * tex->width + x) * bpp;
        uint8_t* dst_ptr = static_cast<uint8_t*>(mapped);
        size_t row_bytes = static_cast<size_t>(width) * bpp;
        size_t src_stride = static_cast<size_t>(tex->width) * bpp;
        for (int r = 0; r < height; ++r) {
            std::memcpy(dst_ptr + r * row_bytes, src_ptr + r * src_stride, row_bytes);
        }
    }
    vkUnmapMemory(m_impl->device, stagingMem);

    // Submit copy command
    VkCommandPool pool;
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = m_impl->queueFamilyIndex;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    vkCreateCommandPool(m_impl->device, &poolInfo, nullptr, &pool);

    VkCommandBufferAllocateInfo cbAlloc{};
    cbAlloc.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cbAlloc.commandPool = pool;
    cbAlloc.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cbAlloc.commandBufferCount = 1;
    VkCommandBuffer cmd;
    vkAllocateCommandBuffers(m_impl->device, &cbAlloc, &cmd);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(cmd, &beginInfo);

    // Transition image to TRANSFER_DST_OPTIMAL
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = tex->layout;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = tex->image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = (tex->layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) ? VK_ACCESS_SHADER_READ_BIT : 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    VkPipelineStageFlags srcStage = (tex->layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        ? VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
        : VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

    vkCmdPipelineBarrier(cmd, srcStage, VK_PIPELINE_STAGE_TRANSFER_BIT,
                         0, 0, nullptr, 0, nullptr, 1, &barrier);

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {x, y, 0};
    region.imageExtent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1};

    vkCmdCopyBufferToImage(cmd, stagingBuf, tex->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    // Transition image to SHADER_READ_ONLY_OPTIMAL
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                         0, 0, nullptr, 0, nullptr, 1, &barrier);

    tex->layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    vkEndCommandBuffer(cmd);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd;
    vkQueueSubmit(m_impl->queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_impl->queue);

    vkFreeCommandBuffers(m_impl->device, pool, 1, &cmd);
    vkDestroyCommandPool(m_impl->device, pool, nullptr);

    vkDestroyBuffer(m_impl->device, stagingBuf, nullptr);
    vkFreeMemory(m_impl->device, stagingMem, nullptr);

    return true;
}

bool VulkanRenderer::deleteTexture(int image) {
    VulkanTexture* tex = m_impl->findTexture(image);
    if (!tex) return false;

    VkDevice dev = m_impl->device;
    if (tex->sampler != VK_NULL_HANDLE) vkDestroySampler(dev, tex->sampler, nullptr);
    if (tex->view != VK_NULL_HANDLE) vkDestroyImageView(dev, tex->view, nullptr);
    if (tex->ownsImage) {
        if (tex->image != VK_NULL_HANDLE) vkDestroyImage(dev, tex->image, nullptr);
        if (tex->memory != VK_NULL_HANDLE) vkFreeMemory(dev, tex->memory, nullptr);
    }
    tex->id = 0;
    return true;
}

bool VulkanRenderer::getTextureSize(int image, int& outWidth, int& outHeight) {
    VulkanTexture* tex = m_impl->findTexture(image);
    if (!tex) return false;
    outWidth = tex->width;
    outHeight = tex->height;
    return true;
}

int VulkanRenderer::createTextureFromNativeHandle(uint64_t handle, int w, int h, int imageFlags) {
    if (handle == 0) return 0;
    VulkanTexture* tex = m_impl->allocTexture();
    if (!tex) return 0;

    tex->type = TextureType::RGBA;
    tex->width = w;
    tex->height = h;
    tex->flags = imageFlags;
    tex->ownsImage = false;
    tex->view = reinterpret_cast<VkImageView>(handle);

    VkDevice dev = m_impl->device;

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    vkCreateSampler(dev, &samplerInfo, nullptr, &tex->sampler);

    VkDescriptorSetAllocateInfo dsAlloc{};
    dsAlloc.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    dsAlloc.descriptorPool = m_impl->descPool;
    dsAlloc.descriptorSetCount = 1;
    dsAlloc.pSetLayouts = &m_impl->descSetLayout;
    vkAllocateDescriptorSets(dev, &dsAlloc, &tex->descriptorSet);

    VkDescriptorBufferInfo dbi{};
    dbi.buffer = m_impl->fragBuf;
    dbi.offset = 0;
    dbi.range = sizeof(FragUniforms);

    VkDescriptorImageInfo dii{};
    dii.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    dii.imageView = tex->view;
    dii.sampler = tex->sampler;

    std::array<VkWriteDescriptorSet, 2> writes{};
    writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[0].dstSet = tex->descriptorSet;
    writes[0].dstBinding = 0;
    writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    writes[0].descriptorCount = 1;
    writes[0].pBufferInfo = &dbi;

    writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[1].dstSet = tex->descriptorSet;
    writes[1].dstBinding = 1;
    writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writes[1].descriptorCount = 1;
    writes[1].pImageInfo = &dii;

    vkUpdateDescriptorSets(dev, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
    return tex->id;
}

void VulkanRenderer::setTargetSurface(VulkanSurface* surface) {
    m_impl->targetSurface = surface;
    if (surface && surface->render_pass() != VK_NULL_HANDLE && m_impl->pipelineLayout != VK_NULL_HANDLE) {
        m_impl->createPipelines(surface->render_pass());
    }
}

void VulkanRenderer::setCommandBuffer(VkCommandBuffer cmd) {
    m_impl->externalCmd = cmd;
}

void VulkanRenderer::renderFill(const Paint& paint, const CompositeOperationState& compOp,
                                const Scissor& scissor, float fringe, const float* bounds,
                                const RenderPath* paths, int npaths) {
    (void)bounds;
    int hwScissor[4]{-1, -1, -1, -1};
    bool hasHwScissor = m_impl->getHardwareScissor(scissor, hwScissor);

    CallType callType = CallType::Fill;
    int triangleCount = 4;
    if (npaths == 1 && paths[0].convex) {
        callType = CallType::ConvexFill;
        triangleCount = 0;
    }

    FragUniforms tempFrag;
    m_impl->convertPaint(&tempFrag, paint, scissor, fringe, fringe, -1.0f, hasHwScissor);
    uint32_t c = paint.innerColor.premultiplied().toRGBA8();

    Call call;
    call.type = callType;
    call.triangleCount = triangleCount;
    call.pathOffset = static_cast<int>(m_impl->paths.size());
    call.pathCount = npaths;
    call.image = paint.image;
    call.compOp = compOp;
    if (hasHwScissor) {
        std::memcpy(call.scissorRect, hwScissor, sizeof(hwScissor));
    }

    int pathOffset = static_cast<int>(m_impl->paths.size());
    m_impl->paths.resize(pathOffset + npaths);

    int maxverts = 0;
    for (int i = 0; i < npaths; i++) {
        maxverts += paths[i].fillCount + paths[i].strokeCount;
    }

    int offset = static_cast<int>(m_impl->verts.size());
    m_impl->verts.resize(offset + maxverts);

    int curOffset = offset;
    for (int i = 0; i < npaths; i++) {
        const RenderPath& p = paths[i];
        VkPath& copy = m_impl->paths[pathOffset + i];
        copy = VkPath{};
        if (p.fillCount > 0 && p.fill != nullptr) {
            copy.fillOffset = curOffset;
            copy.fillCount = p.fillCount;
            std::memcpy(&m_impl->verts[curOffset], p.fill, p.fillCount * sizeof(Vertex));
            for (int k = 0; k < p.fillCount; ++k) m_impl->verts[curOffset + k].color = c;
            curOffset += p.fillCount;
        }
        if (p.strokeCount > 0 && p.stroke != nullptr) {
            copy.strokeOffset = curOffset;
            copy.strokeCount = p.strokeCount;
            std::memcpy(&m_impl->verts[curOffset], p.stroke, p.strokeCount * sizeof(Vertex));
            for (int k = 0; k < p.strokeCount; ++k) m_impl->verts[curOffset + k].color = c;
            curOffset += p.strokeCount;
        }
    }
    m_impl->verts.resize(curOffset);

    call.uniformOffset = static_cast<int>(m_impl->uniforms.size());
    m_impl->uniforms.resize(call.uniformOffset + m_impl->fragSize);
    std::memcpy(m_impl->fragUniformPtr(call.uniformOffset), &tempFrag, sizeof(FragUniforms));

    m_impl->calls.push_back(call);
}

void VulkanRenderer::renderStroke(const Paint& paint, const CompositeOperationState& compOp,
                                  const Scissor& scissor, float fringe, float strokeWidth,
                                  const RenderPath* paths, int npaths) {
    int hwScissor[4]{-1, -1, -1, -1};
    bool hasHwScissor = m_impl->getHardwareScissor(scissor, hwScissor);

    FragUniforms tempFrag;
    m_impl->convertPaint(&tempFrag, paint, scissor, strokeWidth, fringe, -1.0f, hasHwScissor);
    uint32_t c = paint.innerColor.premultiplied().toRGBA8();

    Call call;
    call.type = CallType::Stroke;
    call.pathOffset = static_cast<int>(m_impl->paths.size());
    call.pathCount = npaths;
    call.image = paint.image;
    call.compOp = compOp;
    if (hasHwScissor) {
        std::memcpy(call.scissorRect, hwScissor, sizeof(hwScissor));
    }

    int pathOffset = static_cast<int>(m_impl->paths.size());
    m_impl->paths.resize(pathOffset + npaths);

    int maxverts = 0;
    for (int i = 0; i < npaths; i++) {
        maxverts += paths[i].strokeCount;
    }

    int offset = static_cast<int>(m_impl->verts.size());
    m_impl->verts.resize(offset + maxverts);

    int curOffset = offset;
    for (int i = 0; i < npaths; i++) {
        const RenderPath& p = paths[i];
        VkPath& copy = m_impl->paths[pathOffset + i];
        copy = VkPath{};
        if (p.strokeCount > 0 && p.stroke != nullptr) {
            copy.strokeOffset = curOffset;
            copy.strokeCount = p.strokeCount;
            std::memcpy(&m_impl->verts[curOffset], p.stroke, p.strokeCount * sizeof(Vertex));
            for (int k = 0; k < p.strokeCount; ++k) m_impl->verts[curOffset + k].color = c;
            curOffset += p.strokeCount;
        }
    }
    m_impl->verts.resize(curOffset);

    call.uniformOffset = static_cast<int>(m_impl->uniforms.size());
    m_impl->uniforms.resize(call.uniformOffset + m_impl->fragSize);
    std::memcpy(m_impl->fragUniformPtr(call.uniformOffset), &tempFrag, sizeof(FragUniforms));

    m_impl->calls.push_back(call);
}

void VulkanRenderer::renderTriangles(const Paint& paint, const CompositeOperationState& compOp,
                                     const Scissor& scissor, const Vertex* verts, int nverts,
                                     float fringe, int /*shaderType*/) {
    int hwScissor[4]{-1, -1, -1, -1};
    bool hasHwScissor = m_impl->getHardwareScissor(scissor, hwScissor);

    FragUniforms tempFrag;
    m_impl->convertPaint(&tempFrag, paint, scissor, 1.0f, fringe, -1.0f, hasHwScissor);
    tempFrag.type = (paint.image != 0) ? ShaderImg : ShaderSolid;

    Call call;
    call.type = CallType::Triangles;
    call.image = paint.image;
    call.compOp = compOp;
    if (hasHwScissor) {
        std::memcpy(call.scissorRect, hwScissor, sizeof(hwScissor));
    }

    call.triangleOffset = static_cast<int>(m_impl->verts.size());
    call.triangleCount = nverts;

    m_impl->verts.resize(call.triangleOffset + nverts);
    std::memcpy(&m_impl->verts[call.triangleOffset], verts, nverts * sizeof(Vertex));

    call.uniformOffset = static_cast<int>(m_impl->uniforms.size());
    m_impl->uniforms.resize(call.uniformOffset + m_impl->fragSize);
    std::memcpy(m_impl->fragUniformPtr(call.uniformOffset), &tempFrag, sizeof(FragUniforms));

    m_impl->calls.push_back(call);
}

void VulkanRenderer::flush() {
    if (m_impl->calls.empty()) return;

    VkCommandBuffer cmd = m_impl->externalCmd;
    if (cmd == VK_NULL_HANDLE && m_impl->targetSurface != nullptr) {
        cmd = m_impl->targetSurface->current_command_buffer();
    }
    if (cmd == VK_NULL_HANDLE) {
        cancel();
        return;
    }

    // 1. Upload dynamic vertex buffer
    size_t vertBytes = m_impl->verts.size() * sizeof(Vertex);
    if (vertBytes > m_impl->vertBufCap) {
        vkDestroyBuffer(m_impl->device, m_impl->vertBuf, nullptr);
        vkFreeMemory(m_impl->device, m_impl->vertMem, nullptr);
        m_impl->vertBufCap = vertBytes + 65536;
        m_impl->createBuffer(m_impl->vertBufCap, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                             m_impl->vertBuf, m_impl->vertMem);
    }
    void* mappedVerts = nullptr;
    vkMapMemory(m_impl->device, m_impl->vertMem, 0, vertBytes, 0, &mappedVerts);
    std::memcpy(mappedVerts, m_impl->verts.data(), vertBytes);
    vkUnmapMemory(m_impl->device, m_impl->vertMem);

    // 2. Upload dynamic uniform buffer
    size_t uniformBytes = m_impl->uniforms.size();
    if (uniformBytes > m_impl->fragBufCap) {
        vkDestroyBuffer(m_impl->device, m_impl->fragBuf, nullptr);
        vkFreeMemory(m_impl->device, m_impl->fragMem, nullptr);
        m_impl->fragBufCap = uniformBytes + 16384;
        m_impl->createBuffer(m_impl->fragBufCap, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                             m_impl->fragBuf, m_impl->fragMem);
    }
    void* mappedUniforms = nullptr;
    vkMapMemory(m_impl->device, m_impl->fragMem, 0, uniformBytes, 0, &mappedUniforms);
    std::memcpy(mappedUniforms, m_impl->uniforms.data(), uniformBytes);
    vkUnmapMemory(m_impl->device, m_impl->fragMem);

    // 3. Set Dynamic Viewport
    VkViewport vp{};
    vp.x = 0.0f;
    vp.y = 0.0f;
    vp.width = m_impl->view[0];
    vp.height = m_impl->view[1];
    vp.minDepth = 0.0f;
    vp.maxDepth = 1.0f;
    vkCmdSetViewport(cmd, 0, 1, &vp);

    // Push constants (viewSize)
    vkCmdPushConstants(cmd, m_impl->pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(float) * 2, m_impl->view);

    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(cmd, 0, 1, &m_impl->vertBuf, &offset);

    // 4. Execute calls
    VulkanTexture* dummy = m_impl->findTexture(m_impl->dummyTex);

    for (const auto& call : m_impl->calls) {
        // Scissor
        VkRect2D sci{};
        if (call.scissorRect[0] >= 0) {
            sci.offset = {call.scissorRect[0], call.scissorRect[1]};
            sci.extent = {static_cast<uint32_t>(call.scissorRect[2]), static_cast<uint32_t>(call.scissorRect[3])};
        } else {
            sci.offset = {0, 0};
            sci.extent = {static_cast<uint32_t>(m_impl->view[0]), static_cast<uint32_t>(m_impl->view[1])};
        }
        vkCmdSetScissor(cmd, 0, 1, &sci);

        VulkanTexture* tex = (call.image != 0) ? m_impl->findTexture(call.image) : dummy;
        if (!tex) tex = dummy;

        uint32_t dynamicOffset = static_cast<uint32_t>(call.uniformOffset);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_impl->pipelineLayout,
                                0, 1, &tex->descriptorSet, 1, &dynamicOffset);

        if (call.type == CallType::ConvexFill) {
            if (m_impl->pipelineConvexFill != VK_NULL_HANDLE) {
                vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_impl->pipelineConvexFill);
            }
            for (int i = 0; i < call.pathCount; ++i) {
                const auto& path = m_impl->paths[call.pathOffset + i];
                if (path.fillCount > 0) {
                    vkCmdDraw(cmd, path.fillCount, 1, path.fillOffset, 0);
                }
            }
            if (m_impl->pipelineStroke != VK_NULL_HANDLE) {
                vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_impl->pipelineStroke);
            }
            for (int i = 0; i < call.pathCount; ++i) {
                const auto& path = m_impl->paths[call.pathOffset + i];
                if (path.strokeCount > 0) {
                    vkCmdDraw(cmd, path.strokeCount, 1, path.strokeOffset, 0);
                }
            }
        } else if (call.type == CallType::Stroke) {
            if (m_impl->pipelineStroke != VK_NULL_HANDLE) {
                vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_impl->pipelineStroke);
            }
            for (int i = 0; i < call.pathCount; ++i) {
                const auto& path = m_impl->paths[call.pathOffset + i];
                if (path.strokeCount > 0) {
                    vkCmdDraw(cmd, path.strokeCount, 1, path.strokeOffset, 0);
                }
            }
        } else if (call.type == CallType::Triangles) {
            if (call.triangleCount > 0) {
                if (m_impl->pipelineTriangles != VK_NULL_HANDLE) {
                    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_impl->pipelineTriangles);
                }
                vkCmdDraw(cmd, call.triangleCount, 1, call.triangleOffset, 0);
            }
        } else if (call.type == CallType::Fill) {
            if (m_impl->pipelineConvexFill != VK_NULL_HANDLE) {
                vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_impl->pipelineConvexFill);
            }
            for (int i = 0; i < call.pathCount; ++i) {
                const auto& path = m_impl->paths[call.pathOffset + i];
                if (path.fillCount > 0) {
                    vkCmdDraw(cmd, path.fillCount, 1, path.fillOffset, 0);
                }
            }
            if (m_impl->pipelineStroke != VK_NULL_HANDLE) {
                vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_impl->pipelineStroke);
            }
            for (int i = 0; i < call.pathCount; ++i) {
                const auto& path = m_impl->paths[call.pathOffset + i];
                if (path.strokeCount > 0) {
                    vkCmdDraw(cmd, path.strokeCount, 1, path.strokeOffset, 0);
                }
            }
        }
    }

    cancel();
}

std::unique_ptr<Context> createContextVulkan(
    std::shared_ptr<VulkanSurface> surface,
    int flags
) {
    if (!surface) return nullptr;
    auto renderer = std::make_unique<VulkanRenderer>(
        surface->device()->backend_type() == GpuBackendType::Vulkan ?
            std::static_pointer_cast<VulkanDevice>(surface->device())->device() : VK_NULL_HANDLE,
        surface->device()->backend_type() == GpuBackendType::Vulkan ?
            std::static_pointer_cast<VulkanDevice>(surface->device())->physical_device() : VK_NULL_HANDLE,
        surface->device()->backend_type() == GpuBackendType::Vulkan ?
            std::static_pointer_cast<VulkanDevice>(surface->device())->graphics_queue() : VK_NULL_HANDLE,
        surface->device()->backend_type() == GpuBackendType::Vulkan ?
            std::static_pointer_cast<VulkanDevice>(surface->device())->graphics_queue_family() : 0,
        surface->render_pass(),
        (surface->samples() >= 4) ? VK_SAMPLE_COUNT_4_BIT : VK_SAMPLE_COUNT_1_BIT,
        flags
    );
    renderer->setTargetSurface(surface.get());
    if (!renderer->init()) return nullptr;
    return std::make_unique<Context>(std::move(renderer), flags);
}

std::unique_ptr<Context> createContextVulkan(
    VkDevice device,
    VkPhysicalDevice physicalDevice,
    VkQueue queue,
    uint32_t queueFamilyIndex,
    VkRenderPass renderPass,
    VkSampleCountFlagBits samples,
    int flags
) {
    auto renderer = std::make_unique<VulkanRenderer>(
        device, physicalDevice, queue, queueFamilyIndex, renderPass, samples, flags
    );
    if (!renderer->init()) return nullptr;
    return std::make_unique<Context>(std::move(renderer), flags);
}

} // namespace nisaba::gpu
