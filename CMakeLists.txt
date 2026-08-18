cmake_minimum_required(VERSION 3.20)
project(BladeBallClicker LANGUAGES CXX)

if(NOT WIN32)
    message(FATAL_ERROR "This project targets Windows only (uses Win32 + DirectX11 + Raw Input APIs).")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(imgui CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

add_executable(BladeBallClicker WIN32
    src/main.cpp
    src/Overlay.cpp
    src/InputSimulator.cpp
    src/HotkeyManager.cpp
    src/AutoClicker.cpp
    src/Themes.cpp
    src/Config.cpp
)

target_link_libraries(BladeBallClicker PRIVATE
    imgui::imgui
    nlohmann_json::nlohmann_json
    d3d11.lib
    d3dcompiler.lib
    dwmapi.lib
)

target_compile_definitions(BladeBallClicker PRIVATE UNICODE _UNICODE)
