#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <span>
#include "nisaba/gpu/gpu_types.hpp"
#include "nisaba/math/rect.hpp"
#include "nisaba/math/screen_int_rect.hpp"

namespace nisaba::gpu {

class Context;

/// Sovereign GPU Device abstraction managing hardware GPU context lifecycle (OpenGL / Vulkan).
class GpuDevice {
public:
    static std::shared_ptr<GpuDevice> create(GpuBackendType type = GpuBackendType::Auto);
    static std::shared_ptr<GpuDevice> create_headless(uint32_t width = 800, uint32_t height = 600, GpuBackendType type = GpuBackendType::Auto);

    virtual ~GpuDevice();

    [[nodiscard]] virtual GpuBackendType backend_type() const noexcept { return backend_type_; }
    [[nodiscard]] bool is_valid() const noexcept { return is_valid_; }
    [[nodiscard]] bool is_mock() const noexcept { return is_mock_; }
    [[nodiscard]] const std::string& renderer_name() const noexcept { return renderer_name_; }
    [[nodiscard]] const std::string& version_name() const noexcept { return version_name_; }

    virtual void make_current();
    virtual std::unique_ptr<Context> create_context(int flags = 0);

    // State management
    virtual void set_viewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height);
    virtual void set_scissor(const std::optional<ScreenIntRect>& scissor);
    virtual void set_blend_mode(BlendMode blend_mode);
    virtual void clear(nisaba::Color color);

    // Buffers and VAO (convenience for tests)
    uint32_t create_buffer();
    void delete_buffer(uint32_t buffer_id);
    void bind_vertex_buffer(uint32_t buffer_id);
    void bind_index_buffer(uint32_t buffer_id);
    void upload_buffer_data(uint32_t target, const void* data, size_t size, bool dynamic = true);
    void upload_buffer_subdata(uint32_t target, size_t offset, const void* data, size_t size);

    uint32_t create_vao();
    void delete_vao(uint32_t vao_id);
    void bind_vao(uint32_t vao_id);
    void enable_vertex_attrib_array(uint32_t index);
    void vertex_attrib_pointer(uint32_t index, int32_t size, uint32_t type, bool normalized, int32_t stride, const void* pointer);

    // Textures
    uint32_t create_texture();
    void delete_texture(uint32_t texture_id);
    void bind_texture(uint32_t unit, uint32_t texture_id);
    void upload_texture_image_2d(
        uint32_t width, uint32_t height,
        const void* pixels,
        GpuTextureFormat format = GpuTextureFormat::RGBA8,
        GpuTextureFilter filter = GpuTextureFilter::Linear,
        GpuTextureWrap wrap = GpuTextureWrap::Clamp
    );
    void update_texture_sub_image_2d(
        uint32_t x, uint32_t y,
        uint32_t width, uint32_t height,
        const void* pixels,
        GpuTextureFormat format = GpuTextureFormat::RGBA8
    );

    // Framebuffer operations
    uint32_t create_framebuffer();
    void delete_framebuffer(uint32_t fbo_id);
    void bind_framebuffer(uint32_t fbo_id);
    bool check_framebuffer_complete();
    void attach_texture_to_framebuffer(uint32_t texture_id);

    uint32_t create_renderbuffer();
    void delete_renderbuffer(uint32_t rb_id);
    void bind_renderbuffer(uint32_t rb_id);
    void renderbuffer_storage_multisample(uint32_t samples, uint32_t format, uint32_t width, uint32_t height);
    void attach_renderbuffer_to_framebuffer(uint32_t attachment, uint32_t rb_id);
    void blit_framebuffer(uint32_t src_fbo, uint32_t dst_fbo, uint32_t width, uint32_t height);
    void blit_framebuffer(uint32_t src_fbo, uint32_t dst_fbo,
                          int32_t src_x0, int32_t src_y0, int32_t src_x1, int32_t src_y1,
                          int32_t dst_x0, int32_t dst_y0, int32_t dst_x1, int32_t dst_y1);

    [[nodiscard]] bool has_msaa_support() const noexcept { return has_msaa_; }
    virtual void read_pixels(int32_t x, int32_t y, uint32_t width, uint32_t height, void* out_rgba);

protected:
    GpuDevice();

    GpuBackendType backend_type_{GpuBackendType::OpenGL};
    bool is_valid_{false};
    bool is_mock_{false};
    bool has_msaa_{false};
    std::string renderer_name_{"Hardware GL3"};
    std::string version_name_{"3.0"};

    void* egl_display_{nullptr};
    void* egl_context_{nullptr};
    void* egl_surface_{nullptr};

    void init_glew();
};

} // namespace nisaba::gpu
