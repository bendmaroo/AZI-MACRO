#pragma once
#include <vector>
#include <string>

struct ThemeInfo {
    std::string name;
    // Core accent colors, used both for ImGui::GetStyle() and HUD text color.
    float accent[4];      // primary accent (buttons, highlights)
    float accentHover[4];
    float bgTint[4];      // subtle background tint
    float hudColor[4];    // HUD overlay text color
};

namespace Themes {
    const std::vector<ThemeInfo>& All();
    void Apply(int index); // applies to ImGui::GetStyle()
}
