#include "nisaba/render3d/gpu/gpu_resource_cache.hpp"
#include <GL/glew.h>
#include <GL/gl.h>
#include <cstddef>
#include <cstring>

namespace nisaba::render3d {

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

    if (m_material_ubo == 0) {
        glGenBuffers(1, &m_material_ubo);
        glBindBuffer(GL_UNIFORM_BUFFER, m_material_ubo);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(PbrMaterialUniforms), nullptr, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_UNIFORM_BUFFER, 2, m_material_ubo);
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
    if (m_material_ubo) {
        glDeleteBuffers(1, &m_material_ubo);
        m_material_ubo = 0;
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

void GpuResourceCache::update_material_ubo(const PbrMaterialUniforms& material_uniforms) {
    if (m_material_ubo == 0) {
        init_ubos();
    }

    glBindBuffer(GL_UNIFORM_BUFFER, m_material_ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(PbrMaterialUniforms), &material_uniforms);
    glBindBufferBase(GL_UNIFORM_BUFFER, 2, m_material_ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

} // namespace nisaba::render3d
