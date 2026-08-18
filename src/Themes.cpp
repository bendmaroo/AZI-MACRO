#include "Themes.h"
#include "imgui.h"

namespace {
    const std::vector<ThemeInfo> kThemes = {
        { "Neon Violet",   {0.62f,0.32f,1.00f,1.0f}, {0.75f,0.45f,1.00f,1.0f}, {0.09f,0.07f,0.14f,1.0f}, {0.78f,0.55f,1.00f,1.0f} },
        { "Blade Crimson", {0.95f,0.15f,0.25f,1.0f}, {1.00f,0.30f,0.35f,1.0f}, {0.12f,0.05f,0.06f,1.0f}, {1.00f,0.35f,0.35f,1.0f} },
        { "Cyber Teal",    {0.10f,0.85f,0.75f,1.0f}, {0.25f,1.00f,0.90f,1.0f}, {0.05f,0.10f,0.11f,1.0f}, {0.30f,1.00f,0.90f,1.0f} },
        { "Solar Amber",   {1.00f,0.65f,0.10f,1.0f}, {1.00f,0.75f,0.30f,1.0f}, {0.13f,0.10f,0.05f,1.0f}, {1.00f,0.75f,0.30f,1.0f} },
        { "Minimal Slate", {0.55f,0.60f,0.68f,1.0f}, {0.70f,0.75f,0.82f,1.0f}, {0.11f,0.11f,0.12f,1.0f}, {0.85f,0.87f,0.90f,1.0f} },
    };
}

const std::vector<ThemeInfo>& Themes::All() { return kThemes; }

void Themes::Apply(int index) {
    if (index < 0 || index >= (int)kThemes.size()) index = 0;
    const ThemeInfo& t = kThemes[index];

    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // Base dark backdrop, tinted per-theme.
    colors[ImGuiCol_WindowBg]  = ImVec4(t.bgTint[0], t.bgTint[1], t.bgTint[2], 0.97f);
    colors[ImGuiCol_ChildBg]   = ImVec4(t.bgTint[0], t.bgTint[1], t.bgTint[2], 0.0f);
    colors[ImGuiCol_FrameBg]   = ImVec4(t.bgTint[0]+0.05f, t.bgTint[1]+0.05f, t.bgTint[2]+0.06f, 1.0f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(t.accent[0], t.accent[1], t.accent[2], 0.25f);
    colors[ImGuiCol_FrameBgActive]  = ImVec4(t.accent[0], t.accent[1], t.accent[2], 0.4f);

    colors[ImGuiCol_TitleBgActive] = ImVec4(t.bgTint[0]+0.03f, t.bgTint[1]+0.03f, t.bgTint[2]+0.04f, 1.0f);
    colors[ImGuiCol_CheckMark]  = ImVec4(t.accent[0], t.accent[1], t.accent[2], 1.0f);
    colors[ImGuiCol_SliderGrab] = ImVec4(t.accent[0], t.accent[1], t.accent[2], 1.0f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(t.accentHover[0], t.accentHover[1], t.accentHover[2], 1.0f);

    colors[ImGuiCol_Button]        = ImVec4(t.accent[0], t.accent[1], t.accent[2], 0.55f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(t.accentHover[0], t.accentHover[1], t.accentHover[2], 0.8f);
    colors[ImGuiCol_ButtonActive]  = ImVec4(t.accentHover[0], t.accentHover[1], t.accentHover[2], 1.0f);

    colors[ImGuiCol_Header]        = ImVec4(t.accent[0], t.accent[1], t.accent[2], 0.35f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(t.accent[0], t.accent[1], t.accent[2], 0.6f);
    colors[ImGuiCol_HeaderActive]  = ImVec4(t.accent[0], t.accent[1], t.accent[2], 0.8f);

    colors[ImGuiCol_Tab]        = ImVec4(t.bgTint[0]+0.04f, t.bgTint[1]+0.04f, t.bgTint[2]+0.05f, 1.0f);
    colors[ImGuiCol_TabHovered] = ImVec4(t.accent[0], t.accent[1], t.accent[2], 0.7f);
    colors[ImGuiCol_TabActive]  = ImVec4(t.accent[0], t.accent[1], t.accent[2], 0.5f);

    colors[ImGuiCol_PlotHistogram] = ImVec4(t.accent[0], t.accent[1], t.accent[2], 1.0f);
    colors[ImGuiCol_Border] = ImVec4(t.accent[0], t.accent[1], t.accent[2], 0.3f);

    style.WindowRounding = 10.0f;
    style.FrameRounding = 6.0f;
    style.GrabRounding = 6.0f;
    style.TabRounding = 6.0f;
    style.PopupRounding = 8.0f;
    style.WindowPadding = ImVec2(14, 14);
    style.FramePadding = ImVec2(8, 5);
    style.ItemSpacing = ImVec2(10, 8);
}
