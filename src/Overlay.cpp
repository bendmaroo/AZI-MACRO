#include "Overlay.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <dwmapi.h>

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

namespace {
    LRESULT CALLBACK OverlayWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam)) return true;
        if (msg == WM_DESTROY) { PostQuitMessage(0); return 0; }
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}

bool Overlay::Create(int x, int y) {
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, OverlayWndProc, 0, 0,
        GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr,
        L"BladeBallClickerHUD", nullptr };
    RegisterClassEx(&wc);

    // WS_EX_LAYERED + WS_EX_TRANSPARENT + WS_EX_TOPMOST + WS_EX_TOOLWINDOW:
    // always on top, invisible to Alt+Tab, and clicks pass straight through
    // to the game window underneath.
    hwnd_ = CreateWindowEx(
        WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        wc.lpszClassName, L"HUD", WS_POPUP,
        x, y, 260, 70, nullptr, nullptr, wc.hInstance, nullptr);

    if (!hwnd_) return false;

    // Extend the DWM frame so the window is truly transparent (not just
    // color-keyed), which lets anti-aliased ImGui text render cleanly over
    // whatever's behind it.
    MARGINS margins = { -1 };
    DwmExtendFrameIntoClientArea(hwnd_, &margins);
    SetLayeredWindowAttributes(hwnd_, 0, 255, LWA_ALPHA);

    if (!CreateDeviceD3D()) return false;

    ShowWindow(hwnd_, SW_SHOWNOACTIVATE);
    UpdateWindow(hwnd_);
    return true;
}

bool Overlay::CreateDeviceD3D() {
    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferCount = 2;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hwnd_;
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    sd.AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED;

    D3D_FEATURE_LEVEL level;
    D3D_FEATURE_LEVEL levels[] = { D3D_FEATURE_LEVEL_11_0 };
    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
        levels, 1, D3D11_SDK_VERSION, &sd, &swapChain_,
        &device_, &level, &context_);
    if (FAILED(hr)) return false;

    CreateRenderTarget();
    return true;
}

void Overlay::CreateRenderTarget() {
    ID3D11Texture2D* backBuffer = nullptr;
    swapChain_->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
    device_->CreateRenderTargetView(backBuffer, nullptr, &rtv_);
    backBuffer->Release();
}

void Overlay::SetPosition(int x, int y) {
    if (hwnd_) SetWindowPos(hwnd_, HWND_TOPMOST, x, y, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE);
}

void Overlay::Render(const std::string& label, double value, const float color[4], bool visible) {
    if (!visible || !hwnd_) {
        if (hwnd_) ShowWindow(hwnd_, SW_HIDE);
        return;
    }
    ShowWindow(hwnd_, SW_SHOWNOACTIVATE);

    const float clearColor[4] = { 0, 0, 0, 0 }; // fully transparent clear
    context_->OMSetRenderTargets(1, &rtv_, nullptr);
    context_->ClearRenderTargetView(rtv_, clearColor);

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowBgAlpha(0.55f);
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoNav | ImGuiWindowFlags_AlwaysAutoResize;

    ImGui::Begin("##hud", nullptr, flags);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(color[0], color[1], color[2], color[3]));
    ImGui::SetWindowFontScale(1.6f);
    ImGui::Text("%s: %.0f", label.c_str(), value);
    ImGui::SetWindowFontScale(1.0f);
    ImGui::PopStyleColor();
    ImGui::End();

    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    swapChain_->Present(1, 0);
}

void Overlay::Destroy() {
    if (rtv_) { rtv_->Release(); rtv_ = nullptr; }
    if (swapChain_) { swapChain_->Release(); swapChain_ = nullptr; }
    if (context_) { context_->Release(); context_ = nullptr; }
    if (device_) { device_->Release(); device_ = nullptr; }
    if (hwnd_) { DestroyWindow(hwnd_); hwnd_ = nullptr; }
}
