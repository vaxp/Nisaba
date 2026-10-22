#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <memory>
#include <cstdint>
#include "nisaba/physics/math/mat44.hpp"
#include "nisaba/physics/math/vec3.hpp"

namespace nisaba::render3d {

using Mat44 = nisaba::physics::Mat44;
using Vec3 = nisaba::physics::Vec3;

/// @brief Sovereign GPU Shader Program managing compilation, linkage, and uniform introspection.
class GlShaderProgram {
public:
    GlShaderProgram();
    ~GlShaderProgram();

    GlShaderProgram(const GlShaderProgram&) = delete;
    GlShaderProgram& operator=(const GlShaderProgram&) = delete;

    GlShaderProgram(GlShaderProgram&& rhs) noexcept;
    GlShaderProgram& operator=(GlShaderProgram&& rhs) noexcept;

    /// @brief Compiles vertex and fragment shader source code and links the program.
    bool compile_and_link(std::string_view vertex_src, std::string_view fragment_src);

    void bind() const noexcept;
    void unbind() const noexcept;

    [[nodiscard]] uint32_t program_id() const noexcept { return m_program_id; }
    [[nodiscard]] bool is_valid() const noexcept { return m_is_valid; }
    [[nodiscard]] const std::string& log() const noexcept { return m_log; }

    // Uniform location lookup & caching
    [[nodiscard]] int get_uniform_location(std::string_view name);
    bool set_uniform(std::string_view name, float value);
    bool set_uniform(std::string_view name, int value);
    bool set_uniform(std::string_view name, const Vec3& v);
    bool set_uniform(std::string_view name, float x, float y, float z, float w);
    bool set_uniform(std::string_view name, const Mat44& mat, bool transpose = false);

    // Uniform Block (UBO) Binding index assignment
    bool set_uniform_block_binding(std::string_view block_name, uint32_t binding_point);

    /// @brief Creates the standard sovereign PBR metallic-roughness forward rendering shader.
    [[nodiscard]] static std::unique_ptr<GlShaderProgram> create_pbr_shader();

    /// @brief Creates the directional / CSM light depth map generation shader.
    [[nodiscard]] static std::unique_ptr<GlShaderProgram> create_shadow_depth_shader();

private:
    uint32_t m_program_id{0};
    bool m_is_valid{false};
    std::string m_log;
    std::unordered_map<std::string, int> m_uniform_cache;

    void cleanup() noexcept;
};

} // namespace nisaba::render3d
