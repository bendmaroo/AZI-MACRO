#pragma once
#include <windows.h>
#include <functional>
#include "InputSimulator.h"

// Captures ALL keyboard keys and ALL mouse buttons (including X1/X2 side buttons)
// via the Raw Input API, so any physical input can be assigned as the trigger --
// not just standard keyboard hotkeys. Must be hooked into a window's WndProc
// via ProcessRawInput() when a WM_INPUT message arrives.
class HotkeyManager {
public:
    // Registers the calling window to receive raw keyboard + mouse input.
    bool Init(HWND hwnd);

    // Feed WM_INPUT messages here from your WndProc.
    void ProcessRawInput(LPARAM lParam);

    // While true, the next detected key/button press is captured as a new
    // binding and reported via the callback, then binding mode turns off.
    void BeginBindCapture(std::function<void(InputBinding)> onCaptured);
    bool IsCapturing() const { return capturing_; }

    // Live down/up state for whatever binding is currently configured, so the
    // clicker thread can check "is my bound trigger currently held".
    bool IsBindingDown(const InputBinding& binding) const;

private:
    bool capturing_ = false;
    std::function<void(InputBinding)> captureCallback_;

    // Down-state tracking, keyed loosely: we just track the last binding's state.
    volatile bool keyStates_[256] = {};
    volatile bool mouseLeftDown_ = false;
    volatile bool mouseRightDown_ = false;
    volatile bool mouseMiddleDown_ = false;
    volatile bool mouseX1Down_ = false;
    volatile bool mouseX2Down_ = false;
};
