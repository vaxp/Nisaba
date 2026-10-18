#include "nisaba/compositor/frame_clock.hpp"

namespace nisaba::compositor {

FrameClock::FrameClock(uint32_t refreshRateMilliHz) noexcept {
    setRefreshRate(refreshRateMilliHz);
}

void FrameClock::setRefreshRate(uint32_t refreshRateMilliHz) noexcept {
    if (refreshRateMilliHz == 0) {
        refreshRateMilliHz = 60000;
    }
    m_refreshRateMilliHz = refreshRateMilliHz;
    // 1,000,000,000,000 nanoseconds / (refreshRate in mHz) = nanoseconds per frame
    const uint64_t periodNs = 1'000'000'000'000ULL / static_cast<uint64_t>(refreshRateMilliHz);
    m_framePeriod = std::chrono::nanoseconds(periodNs);
}

void FrameClock::addCallback(Callback cb) {
    if (cb) {
        m_callbacks.push_back(std::move(cb));
    }
}

void FrameClock::dispatch(uint64_t timestampNs) {
    m_lastFrameTimestampNs = timestampNs;
    m_frameIndex++;

    if (m_callbacks.empty()) return;

    std::vector<Callback> pending;
    pending.swap(m_callbacks);

    for (const auto& cb : pending) {
        if (cb) {
            cb(timestampNs);
        }
    }
}

} // namespace nisaba::compositor
