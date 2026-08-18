#pragma once
#include <deque>
#include <mutex>
#include <chrono>

// Measures ACTUAL events-per-second over a trailing 1-second window.
// This is the "real CPS/KPS" the user asked for -- not the target/configured
// rate, but what's actually being sent, sampled live.
class RateCounter {
public:
    void Tick() {
        std::lock_guard<std::mutex> lock(mutex_);
        auto now = std::chrono::steady_clock::now();
        timestamps_.push_back(now);
        Prune(now);
    }

    // Call periodically (e.g. every UI frame) to get the current rate.
    double GetRate() {
        std::lock_guard<std::mutex> lock(mutex_);
        auto now = std::chrono::steady_clock::now();
        Prune(now);
        return static_cast<double>(timestamps_.size());
    }

    void Reset() {
        std::lock_guard<std::mutex> lock(mutex_);
        timestamps_.clear();
    }

private:
    void Prune(const std::chrono::steady_clock::time_point& now) {
        while (!timestamps_.empty() &&
               std::chrono::duration_cast<std::chrono::milliseconds>(now - timestamps_.front()).count() > 1000) {
            timestamps_.pop_front();
        }
    }

    std::deque<std::chrono::steady_clock::time_point> timestamps_;
    std::mutex mutex_;
};
