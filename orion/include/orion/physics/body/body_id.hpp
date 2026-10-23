#pragma once

#include <cstdint>
#include <functional>

namespace orion::physics {

/// @brief Lightweight 32-bit identifier for bodies in the physics simulation.
/// Composed of a 24-bit body array index and an 8-bit generation/sequence counter
/// to prevent ABA handle reuse bugs.
class BodyID {
public:
    static constexpr uint32_t INDEX_BITS = 24;
    static constexpr uint32_t SEQUENCE_BITS = 8;
    static constexpr uint32_t INDEX_MASK = (1u << INDEX_BITS) - 1u;
    static constexpr uint32_t SEQUENCE_MASK = (1u << SEQUENCE_BITS) - 1u;
    static constexpr uint32_t INVALID_ID = 0xFFFFFFFF;

    constexpr BodyID() noexcept : m_id(INVALID_ID) {}

    constexpr explicit BodyID(uint32_t raw_id) noexcept : m_id(raw_id) {}

    constexpr BodyID(uint32_t index, uint8_t sequence) noexcept
        : m_id((index & INDEX_MASK) | (static_cast<uint32_t>(sequence) << INDEX_BITS)) {}

    [[nodiscard]] constexpr bool is_valid() const noexcept {
        return m_id != INVALID_ID;
    }

    [[nodiscard]] constexpr uint32_t index() const noexcept {
        return m_id & INDEX_MASK;
    }

    [[nodiscard]] constexpr uint8_t sequence() const noexcept {
        return static_cast<uint8_t>((m_id >> INDEX_BITS) & SEQUENCE_MASK);
    }

    [[nodiscard]] constexpr uint32_t raw_id() const noexcept {
        return m_id;
    }

    [[nodiscard]] constexpr bool operator==(const BodyID& rhs) const noexcept {
        return m_id == rhs.m_id;
    }

    [[nodiscard]] constexpr bool operator!=(const BodyID& rhs) const noexcept {
        return m_id != rhs.m_id;
    }

    [[nodiscard]] constexpr bool operator<(const BodyID& rhs) const noexcept {
        return m_id < rhs.m_id;
    }

    [[nodiscard]] constexpr bool operator>(const BodyID& rhs) const noexcept {
        return m_id > rhs.m_id;
    }

    [[nodiscard]] constexpr bool operator<=(const BodyID& rhs) const noexcept {
        return m_id <= rhs.m_id;
    }

    [[nodiscard]] constexpr bool operator>=(const BodyID& rhs) const noexcept {
        return m_id >= rhs.m_id;
    }

private:
    uint32_t m_id;
};

} // namespace orion::physics

// std::hash specialization for BodyID
template <>
struct std::hash<orion::physics::BodyID> {
    std::size_t operator()(const orion::physics::BodyID& id) const noexcept {
        return std::hash<uint32_t>{}(id.raw_id());
    }
};
