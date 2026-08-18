#include "InputSimulator.h"
#include <cstdio>

namespace {
    void SendMouseEvent(DWORD downFlag, DWORD upFlag, DWORD mouseData = 0) {
        INPUT inputs[2] = {};

        inputs[0].type = INPUT_MOUSE;
        inputs[0].mi.dwFlags = downFlag;
        inputs[0].mi.mouseData = mouseData;

        inputs[1].type = INPUT_MOUSE;
        inputs[1].mi.dwFlags = upFlag;
        inputs[1].mi.mouseData = mouseData;

        SendInput(2, inputs, sizeof(INPUT));
    }
}

void InputSimulator::SendMouseClick(InputKind button) {
    switch (button) {
        case InputKind::MouseLeft:
            SendMouseEvent(MOUSEEVENTF_LEFTDOWN, MOUSEEVENTF_LEFTUP);
            break;
        case InputKind::MouseRight:
            SendMouseEvent(MOUSEEVENTF_RIGHTDOWN, MOUSEEVENTF_RIGHTUP);
            break;
        case InputKind::MouseMiddle:
            SendMouseEvent(MOUSEEVENTF_MIDDLEDOWN, MOUSEEVENTF_MIDDLEUP);
            break;
        case InputKind::MouseX1:
            SendMouseEvent(MOUSEEVENTF_XDOWN, MOUSEEVENTF_XUP, XBUTTON1);
            break;
        case InputKind::MouseX2:
            SendMouseEvent(MOUSEEVENTF_XDOWN, MOUSEEVENTF_XUP, XBUTTON2);
            break;
        default:
            break;
    }
}

void InputSimulator::SendKeyPress(WORD vkCode) {
    INPUT inputs[2] = {};

    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = vkCode;
    inputs[0].ki.dwFlags = 0; // key down

    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = vkCode;
    inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;

    SendInput(2, inputs, sizeof(INPUT));
}

const char* InputSimulator::DescribeBinding(const InputBinding& binding) {
    static char buf[64];
    if (!binding.isBound) return "Unbound";

    switch (binding.kind) {
        case InputKind::MouseLeft:   return "Mouse Left (M1)";
        case InputKind::MouseRight:  return "Mouse Right (M2)";
        case InputKind::MouseMiddle: return "Mouse Middle";
        case InputKind::MouseX1:     return "Mouse Side Button (X1)";
        case InputKind::MouseX2:     return "Mouse Side Button (X2)";
        case InputKind::Keyboard: {
            UINT scan = MapVirtualKeyA(binding.vkCode, MAPVK_VK_TO_VSC);
            LONG lParam = scan << 16;
            if (GetKeyNameTextA(lParam, buf, sizeof(buf)) > 0) return buf;
            snprintf(buf, sizeof(buf), "VK 0x%02X", binding.vkCode);
            return buf;
        }
    }
    return "Unknown";
}
