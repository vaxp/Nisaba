#pragma once
/// @file dnd.hpp
/// @brief Native Drag-and-Drop (DnD) types, actions, offers, and lifecycle events.

#include "nisaba/backend_os/types.hpp"
#include "nisaba/backend_os/clipboard.hpp"
#include <memory>
#include <vector>
#include <string>
#include <string_view>
#include <type_traits>

namespace nisaba::backend_os {

/// Allowed or accepted drag action flags.
enum class DragAction : uint32_t {
    NoAction = 0,
    Copy     = 1 << 0,
    Move     = 1 << 1,
    Link     = 1 << 2,
    Ask      = 1 << 3,
};

constexpr inline DragAction operator|(DragAction a, DragAction b) {
    return static_cast<DragAction>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

constexpr inline DragAction operator&(DragAction a, DragAction b) {
    return static_cast<DragAction>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}

constexpr inline bool hasDragAction(DragAction mask, DragAction action) {
    return (static_cast<uint32_t>(mask) & static_cast<uint32_t>(action)) != 0;
}

/// Abstract data offer received from external application during Drag & Drop or Clipboard.
class DataOffer {
public:
    virtual ~DataOffer() = default;

    /// Check if the offer contains a specific MIME format.
    [[nodiscard]] virtual bool hasFormat(std::string_view mime_type) const = 0;

    /// List all available MIME formats offered by the source.
    [[nodiscard]] virtual std::vector<std::string> formats() const = 0;

    /// Read plain text from the offer.
    [[nodiscard]] virtual std::string readText() = 0;

    /// Read URI/file list from the offer.
    [[nodiscard]] virtual std::vector<std::string> readUris() = 0;

    /// Read raw data for a specific MIME format.
    [[nodiscard]] virtual std::vector<uint8_t> readData(std::string_view mime_type) = 0;
};

/// In-memory DataOffer backed by a ClipboardData instance (for local/testing usage).
class MemoryDataOffer : public DataOffer {
public:
    explicit MemoryDataOffer(ClipboardData data) : data_(std::move(data)) {}

    [[nodiscard]] bool hasFormat(std::string_view mime_type) const override {
        return data_.hasFormat(mime_type);
    }

    [[nodiscard]] std::vector<std::string> formats() const override {
        return data_.formats();
    }

    [[nodiscard]] std::string readText() override {
        return data_.getText();
    }

    [[nodiscard]] std::vector<std::string> readUris() override {
        return data_.getUris();
    }

    [[nodiscard]] std::vector<uint8_t> readData(std::string_view mime_type) override {
        return data_.getRaw(mime_type);
    }

private:
    ClipboardData data_;
};

/// Data package to be dragged out to external applications or internal widgets.
struct DragData {
    ClipboardData payload;
    DragAction    allowed_actions = DragAction::Copy;
};

/// Event dispatched when a drag operation enters the window.
struct DragEnterEvent {
    Point                    position;
    std::vector<std::string> mime_types;
    DragAction               suggested_action = DragAction::Copy;
    DragAction               accepted_action  = DragAction::NoAction;

    /// Accept the drag with the specified action.
    void accept(DragAction action = DragAction::Copy) {
        accepted_action = action;
    }

    /// Reject the drag.
    void reject() {
        accepted_action = DragAction::NoAction;
    }

    [[nodiscard]] bool isAccepted() const {
        return accepted_action != DragAction::NoAction;
    }
};

/// Event dispatched as the drag moves across the window.
struct DragMotionEvent {
    Point      position;
    DragAction suggested_action = DragAction::Copy;
    DragAction accepted_action  = DragAction::NoAction;

    /// Accept the drag with the specified action.
    void accept(DragAction action = DragAction::Copy) {
        accepted_action = action;
    }

    /// Reject the drag.
    void reject() {
        accepted_action = DragAction::NoAction;
    }

    [[nodiscard]] bool isAccepted() const {
        return accepted_action != DragAction::NoAction;
    }
};

/// Event dispatched when a drag operation leaves the window without dropping.
struct DragLeaveEvent {
};

/// Event dispatched when data is dropped on the window.
struct DropEvent {
    Point                      position;
    std::shared_ptr<DataOffer> data;
    DragAction                 action = DragAction::Copy;
    bool                       handled = false;
};

} // namespace nisaba::backend_os
