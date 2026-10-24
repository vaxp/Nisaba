#include "orion/gpu/gpu_resource_cache.hpp"
#include "orion/ibl/dfg_lut.hpp"
#include <GL/glew.h>
#include <GL/gl.h>
#include <cstddef>
#include <cstring>

namespace orion {

GpuResourceCache::GpuResourceCache() {
    init_ubos();
    init_default_textures();
}

GpuResourceCache::~GpuResourceCache() {
    clear();
}

void GpuResourceCache::init_ubos() {
    if (m_camera_ubo == 0) {
        glGenBuffers(1, &m_camera_ubo);
        glBindBuffer(GL_UNIFORM_BUFFER, m_camera_ubo);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(GpuCameraUboBlock), nullptr, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_camera_ubo);
    }

    if (m_lighting_ubo == 0) {
        glGenBuffers(1, &m_lighting_ubo);
        glBindBuffer(GL_UNIFORM_BUFFER, m_lighting_ubo);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(GpuLightingUboBlock), nullptr, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_UNIFORM_BUFFER, 1, m_lighting_ubo);
    }

    if (m_material_ubo == 0) {
        glGenBuffers(1, &m_material_ubo);
        glBindBuffer(GL_UNIFORM_BUFFER, m_material_ubo);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(PbrMaterialUniforms), nullptr, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_UNIFORM_BUFFER, 2, m_material_ubo);
    }

    if (m_skinning_ubo == 0) {
        glGenBuffers(1, &m_skinning_ubo);
        glBindBuffer(GL_UNIFORM_BUFFER, m_skinning_ubo);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(GpuSkinningUboBlock), nullptr, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_UNIFORM_BUFFER, 3, m_skinning_ubo);
    }

    if (m_instancing_ubo == 0) {
        glGenBuffers(1, &m_instancing_ubo);
        glBindBuffer(GL_UNIFORM_BUFFER, m_instancing_ubo);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(GpuInstancingUboBlock), nullptr, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_UNIFORM_BUFFER, 4, m_instancing_ubo);
    }

    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void GpuResourceCache::init_default_textures() {
    if (m_default_white_tex == 0) {
        static constexpr uint8_t s_white[4] = {255, 255, 255, 255};
        glGenTextures(1, &m_default_white_tex);
        glBindTexture(GL_TEXTURE_2D, m_default_white_tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, s_white);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }
    if (m_default_normal_tex == 0) {
        static constexpr uint8_t s_normal[4] = {128, 128, 255, 255};
        glGenTextures(1, &m_default_normal_tex);
        glBindTexture(GL_TEXTURE_2D, m_default_normal_tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, s_normal);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }
    if (m_default_black_tex == 0) {
        static constexpr uint8_t s_black[4] = {0, 0, 0, 255};
        glGenTextures(1, &m_default_black_tex);
        glBindTexture(GL_TEXTURE_2D, m_default_black_tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, s_black);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }
    glBindTexture(GL_TEXTURE_2D, 0);
}

void GpuResourceCache::clear() noexcept {
    for (auto& [mesh_ptr, handle] : m_mesh_cache) {
        if (handle.vbo) glDeleteBuffers(1, &handle.vbo);
        if (handle.skin_vbo) glDeleteBuffers(1, &handle.skin_vbo);
        if (handle.ebo) glDeleteBuffers(1, &handle.ebo);
        if (handle.vao) glDeleteVertexArrays(1, &handle.vao);
    }
    m_mesh_cache.clear();

    for (auto& [tex_ptr, handle] : m_texture_cache) {
        if (handle.texture_id) {
            glDeleteTextures(1, &handle.texture_id);
        }
    }
    m_texture_cache.clear();

    if (m_camera_ubo) {
        glDeleteBuffers(1, &m_camera_ubo);
        m_camera_ubo = 0;
    }
    if (m_lighting_ubo) {
        glDeleteBuffers(1, &m_lighting_ubo);
        m_lighting_ubo = 0;
    }
    if (m_material_ubo) {
        glDeleteBuffers(1, &m_material_ubo);
        m_material_ubo = 0;
    }
    if (m_skinning_ubo) {
        glDeleteBuffers(1, &m_skinning_ubo);
        m_skinning_ubo = 0;
    }
    if (m_instancing_ubo) {
        glDeleteBuffers(1, &m_instancing_ubo);
        m_instancing_ubo = 0;
    }
    if (m_default_white_tex) {
        glDeleteTextures(1, &m_default_white_tex);
        m_default_white_tex = 0;
    }
    if (m_default_normal_tex) {
        glDeleteTextures(1, &m_default_normal_tex);
        m_default_normal_tex = 0;
    }
    if (m_default_black_tex) {
        glDeleteTextures(1, &m_default_black_tex);
        m_default_black_tex = 0;
    }
    if (m_dfg_lut_tex) {
        glDeleteTextures(1, &m_dfg_lut_tex);
        m_dfg_lut_tex = 0;
    }
    if (m_cluster_cells_tex) {
        glDeleteTextures(1, &m_cluster_cells_tex);
        m_cluster_cells_tex = 0;
    }
    if (m_cluster_indices_tex) {
        glDeleteTextures(1, &m_cluster_indices_tex);
        m_cluster_indices_tex = 0;
    }
    for (auto& pair : m_cubemap_cache) {
        if (pair.second) {
            glDeleteTextures(1, &pair.second);
        }
    }
    m_cubemap_cache.clear();
}

GpuMeshHandle GpuResourceCache::get_or_upload_mesh(const Mesh3D& mesh) {
    auto it = m_mesh_cache.find(&mesh);
    if (it != m_mesh_cache.end()) {
        return it->second;
    }

    GpuMeshHandle handle;
    if (mesh.vertices.empty()) {
        return handle;
    }

    glGenVertexArrays(1, &handle.vao);
    glBindVertexArray(handle.vao);

    glGenBuffers(1, &handle.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, handle.vbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(mesh.vertices.size() * sizeof(Vertex3D)),
        mesh.vertices.data(),
        GL_STATIC_DRAW
    );

    if (!mesh.indices.empty()) {
        glGenBuffers(1, &handle.ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle.ebo);
        glBufferData(
            GL_ELEMENT_ARRAY_BUFFER,
            static_cast<GLsizeiptr>(mesh.indices.size() * sizeof(uint32_t)),
            mesh.indices.data(),
            GL_STATIC_DRAW
        );
        handle.index_count = static_cast<uint32_t>(mesh.indices.size());
    } else {
        handle.index_count = static_cast<uint32_t>(mesh.vertices.size());
    }

    // Vertex attributes with 64-byte stride
    // 0: Position (Vec3, offset 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE,
        sizeof(Vertex3D),
        reinterpret_cast<const void*>(offsetof(Vertex3D, position))
    );

    // 1: Normal (Vec3, offset 16)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1, 3, GL_FLOAT, GL_FALSE,
        sizeof(Vertex3D),
        reinterpret_cast<const void*>(offsetof(Vertex3D, normal))
    );

    // 2: Tangent (float4, offset 32)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(
        2, 4, GL_FLOAT, GL_FALSE,
        sizeof(Vertex3D),
        reinterpret_cast<const void*>(offsetof(Vertex3D, tangent))
    );

    // 3: UV0 (float2, offset 48)
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(
        3, 2, GL_FLOAT, GL_FALSE,
        sizeof(Vertex3D),
        reinterpret_cast<const void*>(offsetof(Vertex3D, uv))
    );

    // 4: UV1 (float2, offset 56)
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(
        4, 2, GL_FLOAT, GL_FALSE,
        sizeof(Vertex3D),
        reinterpret_cast<const void*>(offsetof(Vertex3D, uv1))
    );

    // Optional Skeletal Skinning attributes (5: BoneIndices, 6: BoneWeights)
    if (!mesh.skin_data.empty() && mesh.skin_data.size() >= mesh.vertices.size()) {
        glGenBuffers(1, &handle.skin_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, handle.skin_vbo);
        glBufferData(
            GL_ARRAY_BUFFER,
            static_cast<GLsizeiptr>(mesh.skin_data.size() * sizeof(VertexSkinData)),
            mesh.skin_data.data(),
            GL_STATIC_DRAW
        );

        glEnableVertexAttribArray(5);
        glVertexAttribPointer(
            5, 4, GL_UNSIGNED_SHORT, GL_FALSE,
            sizeof(VertexSkinData),
            reinterpret_cast<const void*>(offsetof(VertexSkinData, joints))
        );

        glEnableVertexAttribArray(6);
        glVertexAttribPointer(
            6, 4, GL_FLOAT, GL_FALSE,
            sizeof(VertexSkinData),
            reinterpret_cast<const void*>(offsetof(VertexSkinData, weights))
        );
        handle.has_skinning = true;
    }

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    handle.is_valid = true;
    m_mesh_cache[&mesh] = handle;
    return handle;
}

void GpuResourceCache::update_mesh_buffer(const Mesh3D& mesh) {
    auto it = m_mesh_cache.find(&mesh);
    if (it == m_mesh_cache.end()) {
        (void)get_or_upload_mesh(mesh);
        return;
    }

    if (it->second.vbo != 0 && !mesh.vertices.empty()) {
        glBindBuffer(GL_ARRAY_BUFFER, it->second.vbo);
        glBufferSubData(
            GL_ARRAY_BUFFER, 0,
            static_cast<GLsizeiptr>(mesh.vertices.size() * sizeof(Vertex3D)),
            mesh.vertices.data()
        );
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}

GpuTextureHandle GpuResourceCache::get_or_upload_texture(const Texture2D& texture) {
    auto it = m_texture_cache.find(&texture);
    if (it != m_texture_cache.end()) {
        return it->second;
    }

    GpuTextureHandle handle;
    if (texture.pixels.empty() || texture.width == 0 || texture.height == 0) {
        return handle;
    }

    GLint prev_tex = 0;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &prev_tex);

    glGenTextures(1, &handle.texture_id);
    glBindTexture(GL_TEXTURE_2D, handle.texture_id);

    GLenum internal_format = GL_RGBA8;
    GLenum format = GL_RGBA;
    if (texture.channels == 1) {
        internal_format = GL_R8;
        format = GL_RED;
    } else if (texture.channels == 3) {
        internal_format = GL_RGB8;
        format = GL_RGB;
    }

    glTexImage2D(
        GL_TEXTURE_2D, 0, internal_format,
        static_cast<GLsizei>(texture.width),
        static_cast<GLsizei>(texture.height),
        0, format, GL_UNSIGNED_BYTE,
        texture.pixels.data()
    );

    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // Anisotropic filtering for sharp textures at glancing angles without sparkling noise
    GLfloat max_aniso = 1.0f;
    glGetFloatv(0x84FF /* GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT */, &max_aniso);
    if (max_aniso > 1.0f) {
        glTexParameterf(GL_TEXTURE_2D, 0x84FE /* GL_TEXTURE_MAX_ANISOTROPY_EXT */, std::min(max_aniso, 8.0f));
    }

    glBindTexture(GL_TEXTURE_2D, prev_tex);

    handle.is_valid = true;
    m_texture_cache[&texture] = handle;
    return handle;
}

void GpuResourceCache::update_camera_ubo(const Mat44& view, const Mat44& proj, const Vec3& eye_pos) {
    if (m_camera_ubo == 0) {
        init_ubos();
    }

    GpuCameraUboBlock block;
    // Transpose row-major Mat44 to column-major std140 array
    auto to_col_major = [](const Mat44& in, float* out) {
        for (int c = 0; c < 4; ++c) {
            for (int r = 0; r < 4; ++r) {
                out[c * 4 + r] = in.m[r * 4 + c];
            }
        }
    };

    to_col_major(view, block.view);
    to_col_major(proj, block.projection);
    to_col_major(proj * view, block.view_projection);

    block.camera_position[0] = eye_pos.x;
    block.camera_position[1] = eye_pos.y;
    block.camera_position[2] = eye_pos.z;
    block.padding = 0.0f;

    glBindBuffer(GL_UNIFORM_BUFFER, m_camera_ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(GpuCameraUboBlock), &block);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_camera_ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void GpuResourceCache::update_lighting_ubo(const GpuLightingUboBlock& lighting_block) {
    if (m_lighting_ubo == 0) {
        init_ubos();
    }

    glBindBuffer(GL_UNIFORM_BUFFER, m_lighting_ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(GpuLightingUboBlock), &lighting_block);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, m_lighting_ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void GpuResourceCache::update_material_ubo(const PbrMaterialUniforms& material_uniforms) {
    if (m_material_ubo == 0) {
        init_ubos();
    }

    glBindBuffer(GL_UNIFORM_BUFFER, m_material_ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(PbrMaterialUniforms), &material_uniforms);
    glBindBufferBase(GL_UNIFORM_BUFFER, 2, m_material_ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void GpuResourceCache::update_skinning_ubo(const Skeleton& skeleton) {
    if (m_skinning_ubo == 0) {
        init_ubos();
    }

    GpuSkinningUboBlock block{};
    const size_t count = std::min(skeleton.joint_matrices.size(), GpuSkinningUboBlock::MAX_BONES);
    for (size_t i = 0; i < count; ++i) {
        const Mat44& in = skeleton.joint_matrices[i];
        float* out = &block.bone_matrices[i * 16];
        for (int c = 0; c < 4; ++c) {
            for (int r = 0; r < 4; ++r) {
                out[c * 4 + r] = in.m[r * 4 + c];
            }
        }
    }

    glBindBuffer(GL_UNIFORM_BUFFER, m_skinning_ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, static_cast<GLsizeiptr>(count * 16 * sizeof(float)), block.bone_matrices);
    glBindBufferBase(GL_UNIFORM_BUFFER, 3, m_skinning_ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void GpuResourceCache::update_instancing_ubo(const GpuInstanceTransform* instances, size_t instance_count) {
    if (m_instancing_ubo == 0) {
        init_ubos();
    }
    const size_t count = std::min(instance_count, GpuInstancingUboBlock::MAX_INSTANCES);
    if (count == 0 || !instances) return;

    glBindBuffer(GL_UNIFORM_BUFFER, m_instancing_ubo);
    glBufferSubData(
        GL_UNIFORM_BUFFER, 0,
        static_cast<GLsizeiptr>(count * sizeof(GpuInstanceTransform)),
        instances
    );
    glBindBufferBase(GL_UNIFORM_BUFFER, 4, m_instancing_ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void GpuResourceCache::update_cluster_textures(const ClusterGrid& grid) {
    const uint32_t w = grid.dim_x() * grid.dim_y(); // 144
    const uint32_t h = grid.dim_z();                // 24
    const size_t total_cells = static_cast<size_t>(w) * h;

    if (m_cluster_cells_tex == 0) {
        glGenTextures(1, &m_cluster_cells_tex);
        glBindTexture(GL_TEXTURE_2D, m_cluster_cells_tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, static_cast<GLsizei>(w), static_cast<GLsizei>(h), 0, GL_RG, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    constexpr uint32_t IDX_W = 1024;
    constexpr uint32_t IDX_H = 64;
    constexpr size_t MAX_INDICES = IDX_W * IDX_H;

    if (m_cluster_indices_tex == 0) {
        glGenTextures(1, &m_cluster_indices_tex);
        glBindTexture(GL_TEXTURE_2D, m_cluster_indices_tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, IDX_W, IDX_H, 0, GL_RED, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    m_cluster_cells_scratch.resize(total_cells * 2);
    const auto cells = grid.cells();
    for (size_t i = 0; i < total_cells && i < cells.size(); ++i) {
        m_cluster_cells_scratch[i * 2 + 0] = static_cast<float>(cells[i].offset);
        m_cluster_cells_scratch[i * 2 + 1] = static_cast<float>(cells[i].count);
    }

    glBindTexture(GL_TEXTURE_2D, m_cluster_cells_tex);
    glTexSubImage2D(
        GL_TEXTURE_2D, 0, 0, 0,
        static_cast<GLsizei>(w), static_cast<GLsizei>(h),
        GL_RG, GL_FLOAT, m_cluster_cells_scratch.data()
    );

    const auto indices = grid.light_indices();
    const size_t idx_count = std::min(indices.size(), MAX_INDICES);
    if (idx_count > 0) {
        const uint32_t rows = static_cast<uint32_t>((idx_count + IDX_W - 1) / IDX_W);
        m_cluster_indices_scratch.resize(static_cast<size_t>(rows) * IDX_W, 0.0f);
        for (size_t i = 0; i < idx_count; ++i) {
            m_cluster_indices_scratch[i] = static_cast<float>(indices[i]);
        }
        glBindTexture(GL_TEXTURE_2D, m_cluster_indices_tex);
        glTexSubImage2D(
            GL_TEXTURE_2D, 0, 0, 0,
            IDX_W, static_cast<GLsizei>(rows),
            GL_RED, GL_FLOAT, m_cluster_indices_scratch.data()
        );
    }

    glBindTexture(GL_TEXTURE_2D, 0);
}

uint32_t GpuResourceCache::get_or_create_dfg_lut() {
    if (m_dfg_lut_tex != 0) {
        return m_dfg_lut_tex;
    }

    constexpr uint32_t res = 256;
    const std::vector<float> lut_data = DfgLutGenerator::generate_table(res, res, 256);

    GLint prev_tex = 0;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &prev_tex);

    glGenTextures(1, &m_dfg_lut_tex);
    glBindTexture(GL_TEXTURE_2D, m_dfg_lut_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, res, res, 0, GL_RGBA, GL_FLOAT, lut_data.data());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, prev_tex);
    return m_dfg_lut_tex;
}

uint32_t GpuResourceCache::get_or_upload_environment_map(const EnvironmentMap& env_map) {
    auto it = m_cubemap_cache.find(&env_map);
    if (it != m_cubemap_cache.end()) {
        return it->second;
    }

    uint32_t cubemap_tex = 0;
    glGenTextures(1, &cubemap_tex);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap_tex);

    const size_t num_mips = env_map.mip_levels_count();
    for (size_t m = 0; m < num_mips; ++m) {
        const auto& mip = env_map.get_mip_level(m);
        const uint32_t res = mip.resolution;

        for (int f = 0; f < 6; ++f) {
            const GLenum target = GL_TEXTURE_CUBE_MAP_POSITIVE_X + f;
            glTexImage2D(
                target,
                static_cast<GLint>(m),
                GL_RGBA16F,
                res, res, 0,
                GL_RGBA, GL_FLOAT,
                mip.faces[f].data()
            );
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, static_cast<GLint>(num_mips - 1));

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    m_cubemap_cache[&env_map] = cubemap_tex;
    return cubemap_tex;
}

} // namespace orion
