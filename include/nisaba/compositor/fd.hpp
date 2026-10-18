#pragma once

#include <unistd.h>
#include <utility>

namespace nisaba::compositor {

/**
 * @brief RAII wrapper for POSIX file descriptors.
 * 
 * Guarantees that the underlying file descriptor is closed when the object
 * goes out of scope. Prevents descriptor leaks across DRM, GBM, and DMA-BUF operations.
 */
class UniqueFd {
public:
    constexpr UniqueFd() noexcept : m_fd(-1) {}
    explicit constexpr UniqueFd(int fd) noexcept : m_fd(fd) {}

    ~UniqueFd() noexcept {
        reset();
    }

    UniqueFd(const UniqueFd&) = delete;
    UniqueFd& operator=(const UniqueFd&) = delete;

    constexpr UniqueFd(UniqueFd&& other) noexcept : m_fd(other.m_fd) {
        other.m_fd = -1;
    }

    UniqueFd& operator=(UniqueFd&& other) noexcept {
        if (this != &other) {
            reset();
            m_fd = other.m_fd;
            other.m_fd = -1;
        }
        return *this;
    }

    [[nodiscard]] constexpr int get() const noexcept {
        return m_fd;
    }

    [[nodiscard]] constexpr bool isValid() const noexcept {
        return m_fd >= 0;
    }

    constexpr explicit operator bool() const noexcept {
        return isValid();
    }

    int release() noexcept {
        int fd = m_fd;
        m_fd = -1;
        return fd;
    }

    void reset(int newFd = -1) noexcept {
        if (m_fd >= 0) {
            close(m_fd);
        }
        m_fd = newFd;
    }

private:
    int m_fd{-1};
};

} // namespace nisaba::compositor
