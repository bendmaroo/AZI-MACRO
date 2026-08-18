#pragma once
#include <windows.h>
#include <cstdint>

// Identifies any bindable trigger or output action: a keyboard VK code,
// or a mouse button (including side buttons X1/X2), in one flat enum-ish type.
enum class InputKind : uint8_t { Keyboard, MouseLeft, MouseRight, MouseMiddle, MouseX1, MouseX2 };

struct InputBinding {
    InputKind kind = InputKind::Keyboard;
    WORD vkCode = 0; // only meaningful when kind == Keyboard
    bool isBound = false;

    bool operator==(const InputBinding& o) const {
        return kind == o.kind && vkCode == o.vkCode && isBound == o.isBound;
    }
};

namespace InputSimulator {
    // Sends a single click (down+up) for the given mouse button.
    void SendMouseClick(InputKind button);

    // Sends a single key press (down+up) for the given VK code.
    void SendKeyPress(WORD vkCode);

    // Human-readable label for a binding, e.g. "F", "Mouse Button 4 (X1)", "Middle Click"
    const char* DescribeBinding(const InputBinding& binding);
}
