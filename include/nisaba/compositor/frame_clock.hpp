#pragma once

#include <chrono>
#include <cstdint>
#include <functional>
#include <vector>

namespace nisaba::compositor {

/**
 * @brief Sovereign FrameClock managing display refresh synchronization and frame callbacks.
 */
class FrameClock {
public:
    using Callback = std::function<void(uint64_t timestampNs)>;

    explicit FrameClock(uint32_t refreshRateMilliHz = 60000) noexcept;
    ~FrameClock() = default;

    void setRefreshRate(uint32_t refreshRateMilliHz) noexcept;
    [[nodiscard]] uint32_t refreshRateMilliHz() const noexcept { return m_refreshRateMilliHz; }

    [[nodiscard]] std::chrono::nanoseconds framePeriod() const noexcept {
        return m_framePeriod;
    }

    /**
     * @brief Registers a client frame callback (e.g. wl_surface.frame) for next presentation.
     */
    void addCallback(Callback cb);

    /**
     * @brief Dispatches all accumulated frame callbacks with the presentation timestamp.
     */
    void dispatch(uint64_t timestampNs);

    [[nodiscard]] size_t pendingCallbacksCount() const noexcept {
        return m_callbacks.size();
    }

    [[nodiscard]] uint64_t lastFrameTimestampNs() const noexcept {
        return m_lastFrameTimestampNs;
    }

    [[nodiscard]] uint64_t frameIndex() const noexcept {
        return m_frameIndex;
    }

private:
    uint32_t m_refreshRateMilliHz{60000};
    std::chrono::nanoseconds m_framePeriod{16'666'666};
    std::vector<Callback> m_callbacks;
    uint64_t m_lastFrameTimestampNs{0};
    uint64_t m_frameIndex{0};
};

} // namespace nisaba::compositor
