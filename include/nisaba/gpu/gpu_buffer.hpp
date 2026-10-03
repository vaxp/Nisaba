#pragma once

#include <cstdint>
#include <memory>
#include <span>
#include <vector>
#include "nisaba/gpu/gpu_types.hpp"
#include "nisaba/gpu/gpu_device.hpp"

namespace nisaba::gpu {

/// Manages streaming Vertex Array Object (VAO), Vertex Buffer (VBO), and Index Buffer (IBO).
class GpuBuffer {
public:
    static std::shared_ptr<GpuBuffer> create(
        std::shared_ptr<GpuDevice> device,
        size_t initial_vertex_capacity = 4096,
        size_t initial_index_capacity = 8192
    );

    ~GpuBuffer();

    void bind();
    void unbind();

    void upload_data(std::span<const GpuVertex> vertices, std::span<const uint32_t> indices);

    [[nodiscard]] size_t vertex_capacity() const noexcept { return vertex_capacity_; }
    [[nodiscard]] size_t index_capacity() const noexcept { return index_capacity_; }

private:
    GpuBuffer(std::shared_ptr<GpuDevice> device, size_t v_cap, size_t i_cap);

    std::shared_ptr<GpuDevice> device_;
    uint32_t vao_id_{0};
    uint32_t vbo_id_{0};
    uint32_t ibo_id_{0};
    size_t vertex_capacity_{0};
    size_t index_capacity_{0};

    void setup_vao();
};

} // namespace nisaba::gpu
