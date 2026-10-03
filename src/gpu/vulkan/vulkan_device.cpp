#include "nisaba/gpu/vulkan_device.hpp"
#include "nisaba/gpu/vulkan_renderer.hpp"
#include <iostream>
#include <cstring>
#include <vector>
#include <algorithm>

namespace nisaba::gpu {

VulkanDevice::VulkanDevice() {
    backend_type_ = GpuBackendType::Vulkan;
}

VulkanDevice::~VulkanDevice() {
    if (device_ != VK_NULL_HANDLE) {
        vkDeviceWaitIdle(device_);
        if (command_pool_ != VK_NULL_HANDLE) {
            vkDestroyCommandPool(device_, command_pool_, nullptr);
            command_pool_ = VK_NULL_HANDLE;
        }
        vkDestroyDevice(device_, nullptr);
        device_ = VK_NULL_HANDLE;
    }
    if (instance_ != VK_NULL_HANDLE) {
        vkDestroyInstance(instance_, nullptr);
        instance_ = VK_NULL_HANDLE;
    }
}

bool VulkanDevice::init_vulkan() {
    // 1. Create Vulkan Instance
    VkApplicationInfo app_info{};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "Nisaba Graphics Engine";
    app_info.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
    app_info.pEngineName = "Nisaba";
    app_info.engineVersion = VK_MAKE_VERSION(0, 1, 0);
    app_info.apiVersion = VK_API_VERSION_1_2;

    std::vector<const char*> instance_extensions;

    // Check supported instance extensions
    uint32_t ext_count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &ext_count, nullptr);
    std::vector<VkExtensionProperties> available_exts(ext_count);
    vkEnumerateInstanceExtensionProperties(nullptr, &ext_count, available_exts.data());

    auto has_ext = [&](const char* name) {
        return std::any_of(available_exts.begin(), available_exts.end(),
            [&](const VkExtensionProperties& p) { return std::strcmp(p.extensionName, name) == 0; });
    };

    if (has_ext("VK_KHR_get_physical_device_properties2")) {
        instance_extensions.push_back("VK_KHR_get_physical_device_properties2");
    }
#if defined(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME)
    if (has_ext(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME)) {
        instance_extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    }
#endif

    VkInstanceCreateInfo inst_info{};
    inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    inst_info.pApplicationInfo = &app_info;
    inst_info.enabledExtensionCount = static_cast<uint32_t>(instance_extensions.size());
    inst_info.ppEnabledExtensionNames = instance_extensions.data();
#if defined(VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR)
    if (has_ext("VK_KHR_portability_enumeration")) {
        inst_info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    }
#endif

    VkResult res = vkCreateInstance(&inst_info, nullptr, &instance_);
    if (res != VK_SUCCESS || instance_ == VK_NULL_HANDLE) {
        // Fallback to API version 1.1 if 1.2 is unavailable
        app_info.apiVersion = VK_API_VERSION_1_1;
        res = vkCreateInstance(&inst_info, nullptr, &instance_);
        if (res != VK_SUCCESS || instance_ == VK_NULL_HANDLE) {
            std::cerr << "[Nisaba Vulkan] Failed to create VkInstance (error: " << res << ")\n";
            return false;
        }
    }

    // 2. Select Physical Device
    uint32_t phys_count = 0;
    vkEnumeratePhysicalDevices(instance_, &phys_count, nullptr);
    if (phys_count == 0) {
        std::cerr << "[Nisaba Vulkan] No Vulkan physical devices found\n";
        return false;
    }

    std::vector<VkPhysicalDevice> devices(phys_count);
    vkEnumeratePhysicalDevices(instance_, &phys_count, devices.data());

    int best_score = -1;
    VkPhysicalDevice best_device = VK_NULL_HANDLE;
    uint32_t best_queue_family = 0;

    for (const auto& dev : devices) {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(dev, &props);

        uint32_t qf_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(dev, &qf_count, nullptr);
        std::vector<VkQueueFamilyProperties> qf_props(qf_count);
        vkGetPhysicalDeviceQueueFamilyProperties(dev, &qf_count, qf_props.data());

        for (uint32_t i = 0; i < qf_count; ++i) {
            if (qf_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                int score = 0;
                if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) score += 1000;
                else if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) score += 500;
                else score += 100;

                if (score > best_score) {
                    best_score = score;
                    best_device = dev;
                    best_queue_family = i;
                }
                break;
            }
        }
    }

    if (best_device == VK_NULL_HANDLE) {
        std::cerr << "[Nisaba Vulkan] No suitable graphics-capable physical device found\n";
        return false;
    }

    physical_device_ = best_device;
    graphics_queue_family_ = best_queue_family;

    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(physical_device_, &props);
    renderer_name_ = props.deviceName;
    version_name_ = std::to_string(VK_VERSION_MAJOR(props.apiVersion)) + "." +
                    std::to_string(VK_VERSION_MINOR(props.apiVersion)) + "." +
                    std::to_string(VK_VERSION_PATCH(props.apiVersion));

    // 3. Create Logical Device
    float queue_priority = 1.0f;
    VkDeviceQueueCreateInfo queue_info{};
    queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_info.queueFamilyIndex = graphics_queue_family_;
    queue_info.queueCount = 1;
    queue_info.pQueuePriorities = &queue_priority;

    VkPhysicalDeviceFeatures device_features{};
    device_features.samplerAnisotropy = VK_FALSE;

    VkDeviceCreateInfo device_info{};
    device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_info.queueCreateInfoCount = 1;
    device_info.pQueueCreateInfos = &queue_info;
    device_info.pEnabledFeatures = &device_features;

    res = vkCreateDevice(physical_device_, &device_info, nullptr, &device_);
    if (res != VK_SUCCESS || device_ == VK_NULL_HANDLE) {
        std::cerr << "[Nisaba Vulkan] Failed to create logical VkDevice\n";
        return false;
    }

    vkGetDeviceQueue(device_, graphics_queue_family_, 0, &graphics_queue_);

    // 4. Create Command Pool
    VkCommandPoolCreateInfo pool_info{};
    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.queueFamilyIndex = graphics_queue_family_;
    pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    res = vkCreateCommandPool(device_, &pool_info, nullptr, &command_pool_);
    if (res != VK_SUCCESS) {
        std::cerr << "[Nisaba Vulkan] Failed to create VkCommandPool\n";
        return false;
    }

    is_valid_ = true;
    has_msaa_ = true;
    return true;
}

std::shared_ptr<VulkanDevice> VulkanDevice::create() {
    return create_headless();
}

std::shared_ptr<VulkanDevice> VulkanDevice::create_headless(uint32_t width, uint32_t height) {
    (void)width; (void)height;
    auto dev = std::make_shared<VulkanDevice>();
    if (!dev->init_vulkan()) {
        return nullptr;
    }
    return dev;
}

uint32_t VulkanDevice::find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags properties) const {
    VkPhysicalDeviceMemoryProperties mem_props;
    vkGetPhysicalDeviceMemoryProperties(physical_device_, &mem_props);

    for (uint32_t i = 0; i < mem_props.memoryTypeCount; ++i) {
        if ((type_filter & (1 << i)) &&
            (mem_props.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    return 0;
}

bool VulkanDevice::create_buffer(
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VkBuffer& out_buffer,
    VkDeviceMemory& out_memory
) const {
    VkBufferCreateInfo buf_info{};
    buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buf_info.size = size;
    buf_info.usage = usage;
    buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device_, &buf_info, nullptr, &out_buffer) != VK_SUCCESS) {
        return false;
    }

    VkMemoryRequirements mem_reqs;
    vkGetBufferMemoryRequirements(device_, out_buffer, &mem_reqs);

    VkMemoryAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = mem_reqs.size;
    alloc_info.memoryTypeIndex = find_memory_type(mem_reqs.memoryTypeBits, properties);

    if (vkAllocateMemory(device_, &alloc_info, nullptr, &out_memory) != VK_SUCCESS) {
        vkDestroyBuffer(device_, out_buffer, nullptr);
        out_buffer = VK_NULL_HANDLE;
        return false;
    }

    vkBindBufferMemory(device_, out_buffer, out_memory, 0);
    return true;
}

bool VulkanDevice::create_image(
    uint32_t width,
    uint32_t height,
    VkFormat format,
    VkImageTiling tiling,
    VkImageUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VkSampleCountFlagBits samples,
    VkImage& out_image,
    VkDeviceMemory& out_memory
) const {
    VkImageCreateInfo img_info{};
    img_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    img_info.imageType = VK_IMAGE_TYPE_2D;
    img_info.extent.width = width;
    img_info.extent.height = height;
    img_info.extent.depth = 1;
    img_info.mipLevels = 1;
    img_info.arrayLayers = 1;
    img_info.format = format;
    img_info.tiling = tiling;
    img_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    img_info.usage = usage;
    img_info.samples = samples;
    img_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(device_, &img_info, nullptr, &out_image) != VK_SUCCESS) {
        return false;
    }

    VkMemoryRequirements mem_reqs;
    vkGetImageMemoryRequirements(device_, out_image, &mem_reqs);

    VkMemoryAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = mem_reqs.size;
    alloc_info.memoryTypeIndex = find_memory_type(mem_reqs.memoryTypeBits, properties);

    if (vkAllocateMemory(device_, &alloc_info, nullptr, &out_memory) != VK_SUCCESS) {
        vkDestroyImage(device_, out_image, nullptr);
        out_image = VK_NULL_HANDLE;
        return false;
    }

    vkBindImageMemory(device_, out_image, out_memory, 0);
    return true;
}

VkCommandBuffer VulkanDevice::begin_single_time_commands() const {
    VkCommandBufferAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandPool = command_pool_;
    alloc_info.commandBufferCount = 1;

    VkCommandBuffer cmd;
    vkAllocateCommandBuffers(device_, &alloc_info, &cmd);

    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(cmd, &begin_info);
    return cmd;
}

void VulkanDevice::end_single_time_commands(VkCommandBuffer cmd) const {
    vkEndCommandBuffer(cmd);

    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &cmd;

    vkQueueSubmit(graphics_queue_, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphics_queue_);

    vkFreeCommandBuffers(device_, command_pool_, 1, &cmd);
}

void VulkanDevice::make_current() {}

std::unique_ptr<Context> VulkanDevice::create_context(int flags) {
    return createContextVulkan(device_, physical_device_, graphics_queue_, graphics_queue_family_, VK_NULL_HANDLE, VK_SAMPLE_COUNT_1_BIT, flags);
}

void VulkanDevice::set_viewport(uint32_t, uint32_t, uint32_t, uint32_t) {}
void VulkanDevice::set_scissor(const std::optional<ScreenIntRect>&) {}
void VulkanDevice::set_blend_mode(BlendMode) {}
void VulkanDevice::clear(nisaba::Color) {}
void VulkanDevice::read_pixels(int32_t, int32_t, uint32_t, uint32_t, void*) {}

} // namespace nisaba::gpu
