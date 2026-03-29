#include <atomic>
#include <string>
#include <chrono>
#include <vector>
#include <mutex>
#include <deque>
#include <algorithm>

namespace living_limiter {

// --- Token Bucket (Original) ---
class TokenBucket {
private:
    std::atomic<long long> tokens;
    long long capacity;
    double refill_rate;
    std::atomic<long long> last_refill_time;

public:
    TokenBucket(long long capacity, double refill_rate)
        : tokens(capacity), capacity(capacity), refill_rate(refill_rate) {
        last_refill_time = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
    }

    bool check(long long requested = 1) {
        refill();
        long long current = tokens.load(std::memory_order_relaxed);
        while (current >= requested) {
            if (tokens.compare_exchange_weak(current, current - requested, 
                                            std::memory_order_relaxed)) {
                return true;
            }
        }
        return false;
    }

    void refill() {
        long long now = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
        long long last = last_refill_time.load(std::memory_order_relaxed);
        long long delta_ms = now - last;
        if (delta_ms <= 0) return;

        long long new_tokens = static_cast<long long>(delta_ms * (refill_rate / 1000.0));
        if (new_tokens > 0) {
            if (last_refill_time.compare_exchange_weak(last, now, std::memory_order_relaxed)) {
                long long current = tokens.load(std::memory_order_relaxed);
                long long target = std::min(capacity, current + new_tokens);
                tokens.store(target, std::memory_order_relaxed);
            }
        }
    }

    long long get_tokens() const { return tokens.load(std::memory_order_relaxed); }
};

// --- Sliding Window Log (High Precision) ---
class SlidingWindowLog {
private:
    std::mutex mtx;
    std::deque<long long> timestamps;
    long long limit;
    long long window_ms;

public:
    SlidingWindowLog(long long limit, long long window_ms)
        : limit(limit), window_ms(window_ms) {}

    bool check() {
        std::lock_guard<std::mutex> lock(mtx);
        long long now = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
        
        // Remove outdated timestamps
        while (!timestamps.empty() && timestamps.front() <= now - window_ms) {
            timestamps.pop_front();
        }

        if (timestamps.size() < static_cast<size_t>(limit)) {
            timestamps.push_back(now);
            return true;
        }
        return false;
    }
};

// --- Leaky Bucket (Traffic Shaping) ---
class LeakyBucket {
private:
    std::atomic<long long> level;
    long long capacity;
    double leak_rate; // tokens per second
    std::atomic<long long> last_leak_time;

public:
    LeakyBucket(long long capacity, double leak_rate)
        : level(0), capacity(capacity), leak_rate(leak_rate) {
        last_leak_time = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
    }

    bool check() {
        leak();
        long long current = level.load(std::memory_order_relaxed);
        while (current < capacity) {
            if (level.compare_exchange_weak(current, current + 1, 
                                           std::memory_order_relaxed)) {
                return true;
            }
        }
        return false;
    }

    void leak() {
        long long now = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
        long long last = last_leak_time.load(std::memory_order_relaxed);
        long long delta_ms = now - last;
        if (delta_ms <= 0) return;

        long long leaked = static_cast<long long>(delta_ms * (leak_rate / 1000.0));
        if (leaked > 0) {
            if (last_leak_time.compare_exchange_weak(last, now, std::memory_order_relaxed)) {
                long long current = level.load(std::memory_order_relaxed);
                long long target = std::max(0LL, current - leaked);
                level.store(target, std::memory_order_relaxed);
            }
        }
    }
};

} // namespace living_limiter
