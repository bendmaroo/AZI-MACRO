#include "HotkeyManager.h"
#include <vector>

bool HotkeyManager::Init(HWND hwnd) {
    RAWINPUTDEVICE devices[2] = {};

    // Keyboard
    devices[0].usUsagePage = 0x01;
    devices[0].usUsage = 0x06;
    devices[0].dwFlags = RIDEV_INPUTSINK; // receive input even when not focused
    devices[0].hwndTarget = hwnd;

    // Mouse
    devices[1].usUsagePage = 0x01;
    devices[1].usUsage = 0x02;
    devices[1].dwFlags = RIDEV_INPUTSINK;
    devices[1].hwndTarget = hwnd;

    return RegisterRawInputDevices(devices, 2, sizeof(RAWINPUTDEVICE)) == TRUE;
}

void HotkeyManager::BeginBindCapture(std::function<void(InputBinding)> onCaptured) {
    captureCallback_ = std::move(onCaptured);
    capturing_ = true;
}

void HotkeyManager::ProcessRawInput(LPARAM lParam) {
    UINT size = 0;
    GetRawInputData((HRAWINPUT)lParam, RID_INPUT, nullptr, &size, sizeof(RAWINPUTHEADER));
    if (size == 0) return;

    std::vector<BYTE> buffer(size);
    if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, buffer.data(), &size, sizeof(RAWINPUTHEADER)) != size)
        return;

    RAWINPUT* raw = reinterpret_cast<RAWINPUT*>(buffer.data());

    auto fireCapture = [&](InputBinding b) {
        if (capturing_) {
            capturing_ = false;
            if (captureCallback_) captureCallback_(b);
        }
    };

    if (raw->header.dwType == RIM_TYPEKEYBOARD) {
        const RAWKEYBOARD& kb = raw->data.keyboard;
        if (kb.VKey == 0xFF) return; // no mapping
        bool isDown = !(kb.Flags & RI_KEY_BREAK);
        if (kb.VKey < 256) keyStates_[kb.VKey] = isDown;

        if (isDown) {
            InputBinding b;
            b.kind = InputKind::Keyboard;
            b.vkCode = kb.VKey;
            b.isBound = true;
            fireCapture(b);
        }
    } else if (raw->header.dwType == RIM_TYPEMOUSE) {
        const RAWMOUSE& mouse = raw->data.mouse;
        USHORT flags = mouse.usButtonFlags;

        auto handleButton = [&](USHORT downFlag, USHORT upFlag, volatile bool& state, InputKind kind) {
            if (flags & downFlag) {
                state = true;
                InputBinding b; b.kind = kind; b.isBound = true;
                fireCapture(b);
            } else if (flags & upFlag) {
                state = false;
            }
        };

        handleButton(RI_MOUSE_LEFT_BUTTON_DOWN, RI_MOUSE_LEFT_BUTTON_UP, mouseLeftDown_, InputKind::MouseLeft);
        handleButton(RI_MOUSE_RIGHT_BUTTON_DOWN, RI_MOUSE_RIGHT_BUTTON_UP, mouseRightDown_, InputKind::MouseRight);
        handleButton(RI_MOUSE_MIDDLE_BUTTON_DOWN, RI_MOUSE_MIDDLE_BUTTON_UP, mouseMiddleDown_, InputKind::MouseMiddle);
        handleButton(RI_MOUSE_BUTTON_4_DOWN, RI_MOUSE_BUTTON_4_UP, mouseX1Down_, InputKind::MouseX1);
        handleButton(RI_MOUSE_BUTTON_5_DOWN, RI_MOUSE_BUTTON_5_UP, mouseX2Down_, InputKind::MouseX2);
    }
}

bool HotkeyManager::IsBindingDown(const InputBinding& binding) const {
    if (!binding.isBound) return false;
    switch (binding.kind) {
        case InputKind::Keyboard:    return binding.vkCode < 256 && keyStates_[binding.vkCode];
        case InputKind::MouseLeft:   return mouseLeftDown_;
        case InputKind::MouseRight:  return mouseRightDown_;
        case InputKind::MouseMiddle: return mouseMiddleDown_;
        case InputKind::MouseX1:     return mouseX1Down_;
        case InputKind::MouseX2:     return mouseX2Down_;
    }
    return false;
}
