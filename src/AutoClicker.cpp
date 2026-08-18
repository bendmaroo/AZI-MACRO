#include "AutoClicker.h"
#include <windows.h>
#include <random>
#include <algorithm>

namespace {
    // Hybrid precise wait: Sleep() for the bulk of the duration (cheap on CPU),
    // then busy-spin the last ~1-2ms for accuracy. Needed because Sleep()
    // alone has ~1-15ms OS scheduler granularity, which is useless at 500 CPS
    // where the whole interval is 2ms.
    void PreciseWaitUntil(LARGE_INTEGER targetTicks, LARGE_INTEGER freq) {
        LARGE_INTEGER now;
        QueryPerformanceCounter(&now);

        double msRemaining = (double)(targetTicks.QuadPart - now.QuadPart) * 1000.0 / freq.QuadPart;
        if (msRemaining > 2.0) {
            Sleep((DWORD)(msRemaining - 1.5));
        }
        // Spin for the remainder for tight accuracy.
        do {
            QueryPerformanceCounter(&now);
        } while (now.QuadPart < targetTicks.QuadPart);
    }
}

AutoClicker::AutoClicker(ClickerSettings& settings, HotkeyManager& hotkeys)
    : settings_(settings), hotkeys_(hotkeys) {}

AutoClicker::~AutoClicker() { Stop(); }

void AutoClicker::Start() {
    if (running_.load()) return;
    running_.store(true);
    worker_ = std::thread(&AutoClicker::WorkerLoop, this);
}

void AutoClicker::Stop() {
    running_.store(false);
    if (worker_.joinable()) worker_.join();
    active_.store(false);
    rateCounter_.Reset();
}

void AutoClicker::WorkerLoop() {
    // Boost thread + process priority so the OS scheduler doesn't starve us,
    // and shrink the global timer period for finer Sleep() granularity.
    timeBeginPeriod(1);
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
    SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);

    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);

    std::mt19937 rng(std::random_device{}());

    bool wasTriggerDown = false;

    while (running_.load()) {
        bool triggerDown = settings_.trigger.isBound && hotkeys_.IsBindingDown(settings_.trigger);

        bool shouldRun;
        if (settings_.activation == ActivationMode::HoldToRun) {
            shouldRun = triggerDown;
        } else {
            // Toggle: flip on the down-edge of the trigger press.
            if (triggerDown && !wasTriggerDown) {
                toggleState_.store(!toggleState_.load());
            }
            shouldRun = toggleState_.load();
        }
        wasTriggerDown = triggerDown;

        active_.store(shouldRun);

        if (!shouldRun) {
            Sleep(1);
            continue;
        }

        // Clamp target rate to the hard caps every iteration (settings can
        // change live from the UI thread).
        double rate = settings_.mode == ClickMode::CPS
            ? std::clamp(settings_.targetRate, 1.0, kMaxCPS)
            : std::clamp(settings_.targetRate, 1.0, kMaxKPS);

        double intervalMs = 1000.0 / rate;

        if (settings_.humanizeJitter) {
            double jitterRange = intervalMs * (settings_.jitterPercent / 100.0);
            std::uniform_real_distribution<double> dist(-jitterRange, jitterRange);
            intervalMs = std::max(0.5, intervalMs + dist(rng));
        }

        LARGE_INTEGER start;
        QueryPerformanceCounter(&start);

        if (settings_.mode == ClickMode::CPS) {
            InputKind outKind = settings_.output.isBound ? settings_.output.kind : InputKind::MouseLeft;
            InputSimulator::SendMouseClick(outKind);
        } else {
            WORD vk = settings_.output.isBound ? settings_.output.vkCode : 0;
            if (vk != 0) InputSimulator::SendKeyPress(vk);
        }
        rateCounter_.Tick();

        LARGE_INTEGER target;
        target.QuadPart = start.QuadPart + (LONGLONG)(intervalMs / 1000.0 * freq.QuadPart);
        PreciseWaitUntil(target, freq);
    }

    timeEndPeriod(1);
}
