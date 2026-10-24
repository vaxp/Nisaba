#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <memory>
#include <cstdint>
#include "orion/math/mat44.hpp"
#include "orion/math/vec3.hpp"

namespace orion {


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
    bool set_uniform(std::string_view name, float x, float y);
    bool set_uniform(std::string_view name, const Vec3& v);
    bool set_uniform(std::string_view name, float x, float y, float z, float w);
    bool set_uniform(std::string_view name, const Mat44& mat, bool transpose = false);

    // Zero-overhead pre-cached uniform location setters for inner draw loop
    static void set_uniform_mat4(int loc, const Mat44& mat, bool transpose = false) noexcept;
    static void set_uniform_vec3(int loc, const Vec3& v) noexcept;
    static void set_uniform_int(int loc, int value) noexcept;

    [[nodiscard]] int loc_model() const noexcept { return m_loc_model; }
    [[nodiscard]] int loc_normal_matrix() const noexcept { return m_loc_normal_matrix; }
    [[nodiscard]] int loc_is_instanced() const noexcept { return m_loc_is_instanced; }
    [[nodiscard]] int loc_has_skinning() const noexcept { return m_loc_has_skinning; }

    // Uniform Block (UBO) Binding index assignment
    bool set_uniform_block_binding(std::string_view block_name, uint32_t binding_point);

    /// @brief Creates the standard sovereign PBR metallic-roughness forward rendering shader.
    [[nodiscard]] static std::unique_ptr<GlShaderProgram> create_pbr_shader();

    /// @brief Creates the high-end cinematic PBR shader with multi-light UBO, extended shading models, and IBL.
    [[nodiscard]] static std::unique_ptr<GlShaderProgram> create_advanced_pbr_shader();

    /// @brief Creates the directional / CSM light depth map generation shader.
    [[nodiscard]] static std::unique_ptr<GlShaderProgram> create_shadow_depth_shader();

private:
    uint32_t m_program_id{0};
    bool m_is_valid{false};
    std::string m_log;
    std::unordered_map<std::string, int> m_uniform_cache;

    int m_loc_model{-1};
    int m_loc_normal_matrix{-1};
    int m_loc_is_instanced{-1};
    int m_loc_has_skinning{-1};

    void cleanup() noexcept;
};

} // namespace orion
