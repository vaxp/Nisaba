/// @file input.cpp
/// @brief Input management implementation.

#include "nisaba/backend_os/input.hpp"
#include <algorithm>
#include <cstring>

namespace nisaba::backend_os {

// ============================================================
// InputManager — State queries
// ============================================================

bool InputManager::isKeyDown(int keycode) const {
    if (keycode >= 0 && keycode < 512) {
        return key_states_[keycode];
    }
    return false;
}

Point InputManager::getMousePosition() const {
    return mouse_position_;
}

bool InputManager::isMouseButtonDown(MouseButton btn) const {
    int idx = static_cast<int>(btn);
    if (idx >= 0 && idx < 4) {
        return mouse_buttons_[idx];
    }
    return false;
}

// ============================================================
// InputManager — Event dispatch
// ============================================================

void InputManager::dispatchPointer(const PointerEvent& event) {
    // Update state
    updateMousePosition(event.position);
    if (event.type == PointerEvent::Down) {
        setMouseButtonState(event.button, true);
    } else if (event.type == PointerEvent::Up) {
        setMouseButtonState(event.button, false);
    }

    // Emit signal
    on_pointer_.emit(event);
}

void InputManager::dispatchKey(const KeyEvent& event) {
    // Update state
    setKeyState(event.scancode, event.type == KeyEvent::Down);

    // Emit signal
    on_key_.emit(event);
}

void InputManager::dispatchTextInput(const TextInputEvent& event) {
    on_text_input_.emit(event);
}

void InputManager::dispatchScroll(const ScrollEvent& event) {
    on_scroll_.emit(event);
}

void InputManager::updateMousePosition(Point pos) {
    mouse_position_ = pos;
}

void InputManager::setKeyState(int keycode, bool down) {
    if (keycode >= 0 && keycode < 512) {
        key_states_[keycode] = down;
    }
}

void InputManager::setMouseButtonState(MouseButton btn, bool down) {
    int idx = static_cast<int>(btn);
    if (idx >= 0 && idx < 4) {
        mouse_buttons_[idx] = down;
    }
}

}  // namespace nisaba::backend_os
