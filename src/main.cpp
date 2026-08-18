#include <windows.h>
#include <d3d11.h>
#include <string>
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

#include "HotkeyManager.h"
#include "AutoClicker.h"
#include "Overlay.h"
#include "Themes.h"
#include "Config.h"
#include "InputSimulator.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

namespace {
    ID3D11Device* g_device = nullptr;
    ID3D11DeviceContext* g_context = nullptr;
    IDXGISwapChain* g_swapChain = nullptr;
    ID3D11RenderTargetView* g_rtv = nullptr;

    HotkeyManager g_hotkeys;
    ClickerSettings g_settings;
    AutoClicker* g_clicker = nullptr;
    Overlay g_overlay;
    Config::FullConfig g_config;

    bool CreateDeviceD3D(HWND hwnd) {
        DXGI_SWAP_CHAIN_DESC sd = {};
        sd.BufferCount = 2;
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow = hwnd;
        sd.SampleDesc.Count = 1;
        sd.Windowed = TRUE;
        sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

        D3D_FEATURE_LEVEL level;
        D3D_FEATURE_LEVEL levels[] = { D3D_FEATURE_LEVEL_11_0 };
        if (FAILED(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
                levels, 1, D3D11_SDK_VERSION, &sd, &g_swapChain, &g_device, &level, &g_context)))
            return false;

        ID3D11Texture2D* backBuffer;
        g_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
        g_device->CreateRenderTargetView(backBuffer, nullptr, &g_rtv);
        backBuffer->Release();
        return true;
    }

    void CleanupDeviceD3D() {
        if (g_rtv) { g_rtv->Release(); g_rtv = nullptr; }
        if (g_swapChain) { g_swapChain->Release(); g_swapChain = nullptr; }
        if (g_context) { g_context->Release(); g_context = nullptr; }
        if (g_device) { g_device->Release(); g_device = nullptr; }
    }

    LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam)) return true;

        switch (msg) {
            case WM_INPUT:
                g_hotkeys.ProcessRawInput(lParam);
                return 0;
            case WM_SIZE:
                if (g_device && wParam != SIZE_MINIMIZED) {
                    if (g_rtv) { g_rtv->Release(); g_rtv = nullptr; }
                    g_swapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
                    ID3D11Texture2D* backBuffer;
                    g_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
                    g_device->CreateRenderTargetView(backBuffer, nullptr, &g_rtv);
                    backBuffer->Release();
                }
                return 0;
            case WM_DESTROY:
                PostQuitMessage(0);
                return 0;
        }
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    const char* ModeLabel(ClickMode m) { return m == ClickMode::CPS ? "CPS" : "KPS"; }
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int) {
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0, 0, hInstance,
        nullptr, nullptr, nullptr, nullptr, L"BladeBallClickerMain", nullptr };
    RegisterClassEx(&wc);
    HWND hwnd = CreateWindow(wc.lpszClassName, L"Blade Ball Clicker", WS_OVERLAPPEDWINDOW,
        100, 100, 480, 640, nullptr, nullptr, wc.hInstance, nullptr);

    if (!CreateDeviceD3D(hwnd)) { CleanupDeviceD3D(); UnregisterClass(wc.lpszClassName, wc.hInstance); return 1; }

    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);

    g_hotkeys.Init(hwnd);

    // Load saved settings if present, else sensible defaults.
    if (!Config::Load(g_config)) {
        g_config.clicker.mode = ClickMode::CPS;
        g_config.clicker.targetRate = 12.0;
        g_config.clicker.activation = ActivationMode::HoldToRun;
        g_config.themeIndex = 0;
        g_config.hudEnabled = true;
    }
    g_settings = g_config.clicker;

    g_clicker = new AutoClicker(g_settings, g_hotkeys);
    g_clicker->Start();

    g_overlay.Create(g_config.hudX, g_config.hudY);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_device, g_context);
    Themes::Apply(g_config.themeIndex);

    bool bindingTrigger = false;
    bool bindingOutput = false;
    bool done = false;

    while (!done) {
        MSG msg;
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT) done = true;
        }
        if (done) break;

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        ImGui::Begin("BladeBallClicker", nullptr,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

        ImGui::TextColored(ImVec4(1,1,1,0.55f), "Blade Ball Clicker");
        ImGui::Separator();
        ImGui::Spacing();

        // ---- Mode ----
        int modeIdx = g_settings.mode == ClickMode::CPS ? 0 : 1;
        const char* modes[] = { "CPS (mouse clicks)", "KPS (key presses)" };
        if (ImGui::Combo("Mode", &modeIdx, modes, 2)) {
            g_settings.mode = modeIdx == 0 ? ClickMode::CPS : ClickMode::KPS;
        }

        // ---- Rate slider ----
        double maxRate = g_settings.mode == ClickMode::CPS ? kMaxCPS : kMaxKPS;
        float rateF = (float)g_settings.targetRate;
        char label[32];
        snprintf(label, sizeof(label), "Target %s", ModeLabel(g_settings.mode));
        if (ImGui::SliderFloat(label, &rateF, 1.0f, (float)maxRate, "%.0f")) {
            g_settings.targetRate = rateF;
        }
        ImGui::TextDisabled("Hard cap: %d %s", (int)maxRate, ModeLabel(g_settings.mode));

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // ---- Trigger bind ----
        ImGui::Text("Trigger key/button:");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.6f,0.9f,1.0f,1), "%s", InputSimulator::DescribeBinding(g_settings.trigger));
        if (ImGui::Button(bindingTrigger ? "Press any key/button..." : "Rebind Trigger")) {
            bindingTrigger = true;
            g_hotkeys.BeginBindCapture([](InputBinding b) {
                g_settings.trigger = b;
            });
        }

        // ---- Output bind (only meaningful for KPS, or to pick which mouse button for CPS) ----
        ImGui::Text("Output %s:", g_settings.mode == ClickMode::CPS ? "mouse button" : "key");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.6f,0.9f,1.0f,1), "%s", InputSimulator::DescribeBinding(g_settings.output));
        if (ImGui::Button(bindingOutput ? "Press any key/button..." : "Rebind Output")) {
            bindingOutput = true;
            g_hotkeys.BeginBindCapture([](InputBinding b) {
                g_settings.output = b;
            });
        }
        if (bindingTrigger && g_settings.trigger.isBound) bindingTrigger = false;
        if (bindingOutput && g_settings.output.isBound) bindingOutput = false;

        ImGui::Spacing();

        // ---- Activation mode ----
        int actIdx = g_settings.activation == ActivationMode::HoldToRun ? 0 : 1;
        const char* acts[] = { "Hold to run", "Toggle on/off" };
        if (ImGui::Combo("Activation", &actIdx, acts, 2)) {
            g_settings.activation = actIdx == 0 ? ActivationMode::HoldToRun : ActivationMode::Toggle;
        }

        // ---- Humanize jitter ----
        ImGui::Checkbox("Humanize timing (small random jitter)", &g_settings.humanizeJitter);
        if (g_settings.humanizeJitter) {
            float jitter = (float)g_settings.jitterPercent;
            if (ImGui::SliderFloat("Jitter %", &jitter, 1.0f, 30.0f, "%.0f%%")) {
                g_settings.jitterPercent = jitter;
            }
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // ---- Live readout ----
        double measured = g_clicker->GetMeasuredRate();
        bool active = g_clicker->IsActive();
        ImGui::TextColored(active ? ImVec4(0.3f,1,0.4f,1) : ImVec4(0.6f,0.6f,0.6f,1),
            active ? "● ACTIVE" : "○ idle");
        ImGui::SameLine();
        ImGui::Text("   Real %s: %.0f", ModeLabel(g_settings.mode), measured);

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // ---- HUD controls ----
        ImGui::Checkbox("Show HUD overlay", &g_config.hudEnabled);
        int hudPos[2] = { g_config.hudX, g_config.hudY };
        if (ImGui::DragInt2("HUD position", hudPos, 1, 0, 3000)) {
            g_config.hudX = hudPos[0];
            g_config.hudY = hudPos[1];
            g_overlay.SetPosition(g_config.hudX, g_config.hudY);
        }

        ImGui::Spacing();

        // ---- Theme picker ----
        static int themeIdx = g_config.themeIndex;
        std::string preview = Themes::All()[themeIdx].name;
        if (ImGui::BeginCombo("Theme", preview.c_str())) {
            for (int i = 0; i < (int)Themes::All().size(); i++) {
                bool selected = (i == themeIdx);
                if (ImGui::Selectable(Themes::All()[i].name.c_str(), selected)) {
                    themeIdx = i;
                    g_config.themeIndex = i;
                    Themes::Apply(i);
                }
            }
            ImGui::EndCombo();
        }

        ImGui::Spacing();
        ImGui::Separator();
        if (ImGui::Button("Save Settings", ImVec2(140, 32))) {
            g_config.clicker = g_settings;
            Config::Save(g_config);
        }

        ImGui::End();

        ImGui::Render();
        const float clearColor[4] = { 0.05f, 0.05f, 0.06f, 1.0f };
        g_context->OMSetRenderTargets(1, &g_rtv, nullptr);
        g_context->ClearRenderTargetView(g_rtv, clearColor);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        g_swapChain->Present(1, 0);

        // Render HUD overlay each frame too.
        const ThemeInfo& theme = Themes::All()[g_config.themeIndex];
        g_overlay.Render(ModeLabel(g_settings.mode), measured, theme.hudColor, g_config.hudEnabled);
    }

    g_config.clicker = g_settings;
    Config::Save(g_config);

    g_clicker->Stop();
    delete g_clicker;
    g_overlay.Destroy();

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    DestroyWindow(hwnd);
    UnregisterClass(wc.lpszClassName, wc.hInstance);
    return 0;
}
