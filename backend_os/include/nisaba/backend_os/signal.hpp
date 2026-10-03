#pragma once
/// @file signal.hpp
/// @brief Type-safe Signal/Slot system for decoupled event handling.

#include <functional>
#include <unordered_map>
#include <cstdint>
#include <mutex>
#include <vector>
#include <algorithm>

namespace nisaba::backend_os {

/// A unique identifier for a slot connection.
using SlotId = uint64_t;

/// Invalid slot id sentinel.
inline constexpr SlotId InvalidSlotId = 0;

/// A signal that can be connected to multiple slots (callbacks).
template<typename... Args>
class Signal {
public:
    using SlotFn = std::function<void(Args...)>;

    Signal() = default;
    ~Signal() = default;

    Signal(const Signal&) = delete;
    Signal& operator=(const Signal&) = delete;

    Signal(Signal&& other) noexcept {
        std::lock_guard lock(other.mutex_);
        slots_ = std::move(other.slots_);
        next_id_ = other.next_id_;
    }

    Signal& operator=(Signal&& other) noexcept {
        if (this != &other) {
            std::lock_guard lock1(mutex_);
            std::lock_guard lock2(other.mutex_);
            slots_ = std::move(other.slots_);
            next_id_ = other.next_id_;
        }
        return *this;
    }

    SlotId connect(SlotFn slot) {
        std::lock_guard lock(mutex_);
        SlotId id = ++next_id_;
        slots_[id] = std::move(slot);
        return id;
    }

    template<typename T>
    SlotId connect(T* instance, void (T::*member_fn)(Args...)) {
        return connect([instance, member_fn](Args... args) {
            (instance->*member_fn)(std::forward<Args>(args)...);
        });
    }

    void disconnect(SlotId id) {
        std::lock_guard lock(mutex_);
        slots_.erase(id);
    }

    void disconnectAll() {
        std::lock_guard lock(mutex_);
        slots_.clear();
    }

    void emit(Args... args) {
        std::unordered_map<SlotId, SlotFn> slots_copy;
        {
            std::lock_guard lock(mutex_);
            slots_copy = slots_;
        }
        for (auto& [id, slot] : slots_copy) {
            slot(std::forward<Args>(args)...);
        }
    }

    [[nodiscard]] size_t connectionCount() const {
        std::lock_guard lock(mutex_);
        return slots_.size();
    }

    [[nodiscard]] bool empty() const {
        std::lock_guard lock(mutex_);
        return slots_.empty();
    }

private:
    mutable std::mutex mutex_;
    std::unordered_map<SlotId, SlotFn> slots_;
    SlotId next_id_ = 0;
};

/// RAII wrapper for a signal connection.
template<typename... Args>
class ScopedConnection {
public:
    ScopedConnection() = default;

    ScopedConnection(Signal<Args...>& signal, SlotId id)
        : signal_(&signal), id_(id) {}

    ~ScopedConnection() {
        disconnect();
    }

    ScopedConnection(const ScopedConnection&) = delete;
    ScopedConnection& operator=(const ScopedConnection&) = delete;

    ScopedConnection(ScopedConnection&& other) noexcept
        : signal_(other.signal_), id_(other.id_) {
        other.signal_ = nullptr;
        other.id_ = InvalidSlotId;
    }

    ScopedConnection& operator=(ScopedConnection&& other) noexcept {
        if (this != &other) {
            disconnect();
            signal_ = other.signal_;
            id_ = other.id_;
            other.signal_ = nullptr;
            other.id_ = InvalidSlotId;
        }
        return *this;
    }

    void disconnect() {
        if (signal_ && id_ != InvalidSlotId) {
            signal_->disconnect(id_);
            signal_ = nullptr;
            id_ = InvalidSlotId;
        }
    }

    [[nodiscard]] SlotId id() const { return id_; }
    [[nodiscard]] bool connected() const { return signal_ != nullptr && id_ != InvalidSlotId; }

private:
    Signal<Args...>* signal_ = nullptr;
    SlotId id_ = InvalidSlotId;
};

template<typename... Args>
ScopedConnection<Args...> scopedConnect(Signal<Args...>& signal,
                                         typename Signal<Args...>::SlotFn slot) {
    SlotId id = signal.connect(std::move(slot));
    return ScopedConnection<Args...>(signal, id);
}

}  // namespace nisaba::backend_os
