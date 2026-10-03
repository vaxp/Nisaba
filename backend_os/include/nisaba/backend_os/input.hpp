#pragma once
/// @file input.hpp
/// @brief Input event types and InputManager.

#include "nisaba/backend_os/types.hpp"
#include "nisaba/backend_os/signal.hpp"
#include <string>

namespace nisaba::backend_os {

/// System cursor shapes.
enum class SystemCursor {
    Default,
    Arrow,
    Pointer,          // Link / Hand
    Text,             // I-Beam
    Crosshair,
    Move,
    NotAllowed,
    ResizeHorizontal,
    ResizeVertical,
    ResizeTopLeft,     // NW-SE / Top-Left corner
    ResizeTopRight,    // NE-SW / Top-Right corner
    ResizeBottomLeft,  // SW-NE / Bottom-Left corner
    ResizeBottomRight, // SE-NW / Bottom-Right corner
    Wait
};

/// Window edge identifier for interactive window resizing.
enum class WindowEdge {
    NoneEdge    = 0,
    Top         = 1,
    Bottom      = 2,
    Left        = 3,
    Right       = 4,
    TopLeft     = 5,
    TopRight    = 6,
    BottomLeft  = 7,
    BottomRight = 8,
};

/// Mouse button identifiers.
enum class MouseButton { None = 0, Left = 1, Middle = 2, Right = 3 };

/// Keyboard modifier flags.
enum class KeyMod : int {
    None  = 0,
    Shift = 1 << 0,
    Ctrl  = 1 << 1,
    Alt   = 1 << 2,
    Super = 1 << 3,
};

inline KeyMod operator|(KeyMod a, KeyMod b) {
    return static_cast<KeyMod>(static_cast<int>(a) | static_cast<int>(b));
}
inline bool operator&(KeyMod a, KeyMod b) {
    return (static_cast<int>(a) & static_cast<int>(b)) != 0;
}

/// Pointer (mouse/touch) event.
struct PointerEvent {
    enum Type { Down, Up, Move } type;
    Point       position;
    Point       localPosition;
    MouseButton button = MouseButton::Left;
    int         modifiers = 0;
    double      timestamp = 0;
    int         click_count = 1;  // 1 = single, 2 = double
};

/// Keyboard event.
struct KeyEvent {
    enum Type { Down, Up } type;
    int  keycode   = 0;
    int  scancode  = 0;
    int  modifiers = 0;
    bool repeat    = false;
};

/// Text input event (IME composition result).
struct TextInputEvent {
    std::string text;  // UTF-8
};

/// Scroll (mouse wheel) event.
struct ScrollEvent {
    float dx = 0;
    float dy = 0;
    Point position;
};

/// Manages input state and dispatches input events via signals.
class InputManager {
public:
    InputManager() = default;
    ~InputManager() = default;

    // --- Event signals ---
    Signal<const PointerEvent&>&   onPointer()   { return on_pointer_; }
    Signal<const KeyEvent&>&       onKey()        { return on_key_; }
    Signal<const TextInputEvent&>& onTextInput()  { return on_text_input_; }
    Signal<const ScrollEvent&>&    onScroll()     { return on_scroll_; }

    // --- Immediate state queries ---
    [[nodiscard]] bool  isKeyDown(int keycode) const;
    [[nodiscard]] Point getMousePosition() const;
    [[nodiscard]] bool  isMouseButtonDown(MouseButton btn) const;

    // --- Internal: called by Platform ---
    void dispatchPointer(const PointerEvent& event);
    void dispatchKey(const KeyEvent& event);
    void dispatchTextInput(const TextInputEvent& event);
    void dispatchScroll(const ScrollEvent& event);
    void updateMousePosition(Point pos);
    void setKeyState(int keycode, bool down);
    void setMouseButtonState(MouseButton btn, bool down);

private:
    Signal<const PointerEvent&>   on_pointer_;
    Signal<const KeyEvent&>       on_key_;
    Signal<const TextInputEvent&> on_text_input_;
    Signal<const ScrollEvent&>    on_scroll_;

    Point mouse_position_;
    bool  mouse_buttons_[4] = {};  // indexed by MouseButton
    bool  key_states_[512] = {};   // indexed by scancode
};

}  // namespace nisaba::backend_os
