#pragma once

#include "nisaba/compositor/geometry.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace nisaba::compositor {

/**
 * @brief Multi-buffer damage tracking ring for swapchains and double/triple buffering.
 * Accumulates damage regions between frames and calculates minimal redraw regions.
 */
class DamageRing {
public:
    DamageRing() noexcept = default;
    ~DamageRing() noexcept = default;

    DamageRing(const DamageRing&) = delete;
    DamageRing& operator=(const DamageRing&) = delete;

    /**
     * @brief Adds a damaged region to the current frame.
     */
    void add(const Region& region) noexcept;

    /**
     * @brief Adds a damaged rectangle to the current frame.
     */
    void addRect(const RectI& box) noexcept;

    /**
     * @brief Marks the entire output area as damaged.
     */
    void addWhole(int width, int height) noexcept;

    /**
     * @brief Calculates accumulated damage needed to update target buffer and rotates ring.
     * @param bufferId Unique identifier of buffer (pointer address or index).
     * @param width Buffer width in pixels.
     * @param height Buffer height in pixels.
     * @return Minimal region requiring redraw on this buffer.
     */
    [[nodiscard]] Region rotate(uintptr_t bufferId, int width, int height);

    /**
     * @brief Clears all damage state.
     */
    void clear() noexcept;

    [[nodiscard]] bool hasPendingDamage() const noexcept { return !m_current.isEmpty(); }

private:
    struct BufferEntry {
        uintptr_t bufferId{0};
        Region damage;
    };

    Region m_current;
    std::vector<BufferEntry> m_entries;
    static constexpr size_t kMaxRects = 20;
};

} // namespace nisaba::compositor
