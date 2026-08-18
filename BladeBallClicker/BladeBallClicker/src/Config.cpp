#include "Config.h"
#include <nlohmann/json.hpp>
#include <windows.h>
#include <shlobj.h>
#include <fstream>
#include <filesystem>

using json = nlohmann::json;

namespace {
    std::filesystem::path ConfigPath() {
        PWSTR appData = nullptr;
        SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &appData);
        std::filesystem::path dir = std::filesystem::path(appData) / L"BladeBallClicker";
        CoTaskMemFree(appData);
        std::filesystem::create_directories(dir);
        return dir / L"config.json";
    }

    json BindingToJson(const InputBinding& b) {
        return json{ {"kind", (int)b.kind}, {"vk", b.vkCode}, {"bound", b.isBound} };
    }

    InputBinding BindingFromJson(const json& j) {
        InputBinding b;
        if (j.contains("kind")) b.kind = (InputKind)j.at("kind").get<int>();
        if (j.contains("vk")) b.vkCode = j.at("vk").get<WORD>();
        if (j.contains("bound")) b.isBound = j.at("bound").get<bool>();
        return b;
    }
}

void Config::Save(const FullConfig& cfg) {
    json j;
    j["mode"] = (int)cfg.clicker.mode;
    j["targetRate"] = cfg.clicker.targetRate;
    j["trigger"] = BindingToJson(cfg.clicker.trigger);
    j["output"] = BindingToJson(cfg.clicker.output);
    j["activation"] = (int)cfg.clicker.activation;
    j["humanize"] = cfg.clicker.humanizeJitter;
    j["jitterPercent"] = cfg.clicker.jitterPercent;
    j["theme"] = cfg.themeIndex;
    j["hudEnabled"] = cfg.hudEnabled;
    j["hudX"] = cfg.hudX;
    j["hudY"] = cfg.hudY;

    std::ofstream out(ConfigPath());
    out << j.dump(2);
}

bool Config::Load(FullConfig& cfg) {
    auto path = ConfigPath();
    if (!std::filesystem::exists(path)) return false;

    std::ifstream in(path);
    json j;
    try {
        in >> j;
    } catch (...) {
        return false;
    }

    cfg.clicker.mode = (ClickMode)j.value("mode", 0);
    cfg.clicker.targetRate = j.value("targetRate", 10.0);
    if (j.contains("trigger")) cfg.clicker.trigger = BindingFromJson(j.at("trigger"));
    if (j.contains("output")) cfg.clicker.output = BindingFromJson(j.at("output"));
    cfg.clicker.activation = (ActivationMode)j.value("activation", 0);
    cfg.clicker.humanizeJitter = j.value("humanize", false);
    cfg.clicker.jitterPercent = j.value("jitterPercent", 8.0);
    cfg.themeIndex = j.value("theme", 0);
    cfg.hudEnabled = j.value("hudEnabled", true);
    cfg.hudX = j.value("hudX", 40);
    cfg.hudY = j.value("hudY", 40);
    return true;
}
