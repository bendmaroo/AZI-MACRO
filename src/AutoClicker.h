#pragma once
#include <atomic>
#include <thread>
#include "InputSimulator.h"
#include "HotkeyManager.h"
#include "RateCounter.h"

enum class ClickMode { CPS, KPS };
enum class ActivationMode { HoldToRun, Toggle };

constexpr double kMaxCPS = 500.0;
constexpr double kMaxKPS = 300.0;

struct ClickerSettings {
    ClickMode mode = ClickMode::CPS;
    double targetRate = 10.0;          // clicks or presses per second
    InputBinding trigger;              // what starts/stops it
    InputBinding output;               // what gets spammed: mouse button (CPS) or key (KPS)
    ActivationMode activation = ActivationMode::HoldToRun;
    bool humanizeJitter = false;       // +/- small random timing variance
    double jitterPercent = 8.0;        // % variance when humanize is on
};

// Runs the actual click/keypress loop on a dedicated thread using
// QueryPerformanceCounter for sub-millisecond precision. Reports the
// *real measured* rate via RateCounter, separate from the target rate.
class AutoClicker {
public:
    AutoClicker(ClickerSettings& settings, HotkeyManager& hotkeys);
    ~AutoClicker();

    void Start();   // spins up worker thread
    void Stop();    // stops worker thread

    bool IsRunning() const { return running_.load(); }
    bool IsActive() const { return active_.load(); }  // currently emitting clicks
    double GetMeasuredRate() { return rateCounter_.GetRate(); }

private:
    void WorkerLoop();

    ClickerSettings& settings_;
    HotkeyManager& hotkeys_;
    RateCounter rateCounter_;

    std::thread worker_;
    std::atomic<bool> running_{false};
    std::atomic<bool> active_{false};
    std::atomic<bool> toggleState_{false};
};
