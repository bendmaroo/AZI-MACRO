#pragma once
#include "AutoClicker.h"
#include <string>

namespace Config {
    struct FullConfig {
        ClickerSettings clicker;
        int themeIndex = 0;
        bool hudEnabled = true;
        int hudX = 40;
        int hudY = 40;
    };

    // Both use %APPDATA%\BladeBallClicker\config.json
    void Save(const FullConfig& cfg);
    bool Load(FullConfig& cfg); // returns false if no config existed yet
}
