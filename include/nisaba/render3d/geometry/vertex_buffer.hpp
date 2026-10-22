#pragma once

#include "nisaba/render3d/geometry/vertex_layout.hpp"
#include <vector>
#include <cstdint>
#include <cstring>
#include <stdexcept>

namespace nisaba::render3d {

enum class IndexType : uint8_t {
    UInt16 = 0,
    UInt32 = 1
};

/// @brief Host-side memory buffer holding interleaved vertex data matching a VertexLayout.
class VertexBuffer3D {
public:
    VertexBuffer3D() noexcept : m_layout(VertexLayout::standard_pbr()) {}
    explicit VertexBuffer3D(const VertexLayout& layout) noexcept : m_layout(layout) {}

    void set_data(const void* raw_data, size_t byte_size, uint32_t vertex_count) {
        m_data.resize(byte_size);
        if (raw_data && byte_size > 0) {
            std::memcpy(m_data.data(), raw_data, byte_size);
        }
        m_vertex_count = vertex_count;
    }

    void set_vertices(const std::vector<Vertex3D>& vertices) {
        m_layout = VertexLayout::standard_pbr();
        const size_t byte_size = vertices.size() * sizeof(Vertex3D);
        set_data(vertices.data(), byte_size, static_cast<uint32_t>(vertices.size()));
    }

    [[nodiscard]] const void* data() const noexcept { return m_data.data(); }
    [[nodiscard]] void* data() noexcept { return m_data.data(); }
    [[nodiscard]] size_t byte_size() const noexcept { return m_data.size(); }
    [[nodiscard]] uint32_t vertex_count() const noexcept { return m_vertex_count; }
    [[nodiscard]] const VertexLayout& layout() const noexcept { return m_layout; }

    void reserve(uint32_t count) {
        m_data.reserve(count * m_layout.stride);
    }

    void clear() noexcept {
        m_data.clear();
        m_vertex_count = 0;
    }

private:
    VertexLayout m_layout;
    std::vector<uint8_t> m_data;
    uint32_t m_vertex_count{0};
};

/// @brief Host-side index buffer supporting both 16-bit and 32-bit index sequences.
class IndexBuffer3D {
public:
    IndexBuffer3D() noexcept : m_index_type(IndexType::UInt32) {}
    explicit IndexBuffer3D(IndexType type) noexcept : m_index_type(type) {}

    void set_indices(const std::vector<uint32_t>& indices) {
        m_index_type = IndexType::UInt32;
        const size_t byte_size = indices.size() * sizeof(uint32_t);
        set_data(indices.data(), byte_size, static_cast<uint32_t>(indices.size()), IndexType::UInt32);
    }

    void set_indices(const std::vector<uint16_t>& indices) {
        m_index_type = IndexType::UInt16;
        const size_t byte_size = indices.size() * sizeof(uint16_t);
        set_data(indices.data(), byte_size, static_cast<uint32_t>(indices.size()), IndexType::UInt16);
    }

    void set_data(const void* raw_data, size_t byte_size, uint32_t count, IndexType type) {
        m_index_type = type;
        m_data.resize(byte_size);
        if (raw_data && byte_size > 0) {
            std::memcpy(m_data.data(), raw_data, byte_size);
        }
        m_index_count = count;
    }

    [[nodiscard]] const void* data() const noexcept { return m_data.data(); }
    [[nodiscard]] void* data() noexcept { return m_data.data(); }
    [[nodiscard]] size_t byte_size() const noexcept { return m_data.size(); }
    [[nodiscard]] uint32_t index_count() const noexcept { return m_index_count; }
    [[nodiscard]] IndexType index_type() const noexcept { return m_index_type; }
    [[nodiscard]] size_t index_stride() const noexcept { return m_index_type == IndexType::UInt16 ? 2 : 4; }

    [[nodiscard]] uint32_t get_index(uint32_t idx) const noexcept {
        if (idx >= m_index_count) return 0;
        if (m_index_type == IndexType::UInt16) {
            const auto* ptr = reinterpret_cast<const uint16_t*>(m_data.data());
            return ptr[idx];
        } else {
            const auto* ptr = reinterpret_cast<const uint32_t*>(m_data.data());
            return ptr[idx];
        }
    }

    void clear() noexcept {
        m_data.clear();
        m_index_count = 0;
    }

private:
    IndexType m_index_type{IndexType::UInt32};
    std::vector<uint8_t> m_data;
    uint32_t m_index_count{0};
};

} // namespace nisaba::render3d
