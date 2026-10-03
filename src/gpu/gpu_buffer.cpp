#include "nisaba/gpu/gpu_buffer.hpp"

namespace nisaba::gpu {

GpuBuffer::GpuBuffer(std::shared_ptr<GpuDevice> device, size_t v_cap, size_t i_cap)
    : device_(std::move(device)),
      vertex_capacity_(v_cap),
      index_capacity_(i_cap) {
    setup_vao();
}

GpuBuffer::~GpuBuffer() {
    if (device_) {
        if (vao_id_ != 0) device_->delete_vao(vao_id_);
        if (vbo_id_ != 0) device_->delete_buffer(vbo_id_);
        if (ibo_id_ != 0) device_->delete_buffer(ibo_id_);
    }
}

std::shared_ptr<GpuBuffer> GpuBuffer::create(
    std::shared_ptr<GpuDevice> device,
    size_t initial_vertex_capacity,
    size_t initial_index_capacity
) {
    if (!device) return nullptr;
    return std::shared_ptr<GpuBuffer>(new GpuBuffer(device, initial_vertex_capacity, initial_index_capacity));
}

void GpuBuffer::setup_vao() {
    if (!device_) return;

    vao_id_ = device_->create_vao();
    vbo_id_ = device_->create_buffer();
    ibo_id_ = device_->create_buffer();

    device_->bind_vao(vao_id_);

    // Bind VBO and allocate initial storage
    device_->bind_vertex_buffer(vbo_id_);
    device_->upload_buffer_data(0x8892 /* GL_ARRAY_BUFFER */, nullptr, vertex_capacity_ * sizeof(GpuVertex), true);

    // Bind IBO and allocate initial storage
    device_->bind_index_buffer(ibo_id_);
    device_->upload_buffer_data(0x8893 /* GL_ELEMENT_ARRAY_BUFFER */, nullptr, index_capacity_ * sizeof(uint32_t), true);

    constexpr int32_t stride = static_cast<int32_t>(sizeof(GpuVertex));

    // Location 0: vec2 pos
    device_->enable_vertex_attrib_array(0);
    device_->vertex_attrib_pointer(0, 2, 0x1406 /* GL_FLOAT */, false, stride, reinterpret_cast<const void*>(offsetof(GpuVertex, pos)));

    // Location 1: vec2 uv
    device_->enable_vertex_attrib_array(1);
    device_->vertex_attrib_pointer(1, 2, 0x1406, false, stride, reinterpret_cast<const void*>(offsetof(GpuVertex, uv)));

    // Location 2: vec4 color
    device_->enable_vertex_attrib_array(2);
    device_->vertex_attrib_pointer(2, 4, 0x1406, false, stride, reinterpret_cast<const void*>(offsetof(GpuVertex, color)));

    // Location 3: float coverage
    device_->enable_vertex_attrib_array(3);
    device_->vertex_attrib_pointer(3, 1, 0x1406, false, stride, reinterpret_cast<const void*>(offsetof(GpuVertex, coverage)));

    device_->bind_vao(0);
}

void GpuBuffer::bind() {
    if (device_ && vao_id_ != 0) {
        device_->bind_vao(vao_id_);
    }
}

void GpuBuffer::unbind() {
    if (device_) {
        device_->bind_vao(0);
    }
}

void GpuBuffer::upload_data(std::span<const GpuVertex> vertices, std::span<const uint32_t> indices) {
    if (!device_ || vertices.empty() || indices.empty()) return;

    bind();

    // Resize VBO if needed
    device_->bind_vertex_buffer(vbo_id_);
    if (vertices.size() > vertex_capacity_) {
        vertex_capacity_ = vertices.size() * 2;
        device_->upload_buffer_data(0x8892 /* GL_ARRAY_BUFFER */, vertices.data(), vertex_capacity_ * sizeof(GpuVertex), true);
    } else {
        device_->upload_buffer_subdata(0x8892, 0, vertices.data(), vertices.size_bytes());
    }

    // Resize IBO if needed
    device_->bind_index_buffer(ibo_id_);
    if (indices.size() > index_capacity_) {
        index_capacity_ = indices.size() * 2;
        device_->upload_buffer_data(0x8893 /* GL_ELEMENT_ARRAY_BUFFER */, indices.data(), index_capacity_ * sizeof(uint32_t), true);
    } else {
        device_->upload_buffer_subdata(0x8893, 0, indices.data(), indices.size_bytes());
    }
}

} // namespace nisaba::gpu
