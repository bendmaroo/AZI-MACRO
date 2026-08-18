#pragma once
#include <windows.h>
#include <d3d11.h>
#include <string>

// A second, borderless, transparent, click-through, always-on-top window
// that renders just the live CPS/KPS readout so it's visible over the game.
// Click-through (WS_EX_TRANSPARENT) means it never steals mouse focus from
// the game window underneath it.
class Overlay {
public:
    bool Create(int x, int y);
    void Destroy();

    // Call once per frame from the main app loop.
    void Render(const std::string& label, double value, const float color[4], bool visible);

    void SetPosition(int x, int y);
    HWND Handle() const { return hwnd_; }

private:
    HWND hwnd_ = nullptr;
    ID3D11Device* device_ = nullptr;
    ID3D11DeviceContext* context_ = nullptr;
    IDXGISwapChain* swapChain_ = nullptr;
    ID3D11RenderTargetView* rtv_ = nullptr;

    bool CreateDeviceD3D();
    void CreateRenderTarget();
};
