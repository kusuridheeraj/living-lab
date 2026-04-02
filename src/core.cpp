#include <atomic>
#include <string>
#include <chrono>
#include <vector>
#include <mutex>
#include <deque>
#include <algorithm>
#include <random>
#include <thread>
#include <memory>
#include <cmath>

namespace living_limiter {

// --- 1. Token Bucket (Classic) ---
class TokenBucket {
private:
    std::atomic<long long> tokens;
    long long capacity;
    double refill_rate;
    std::atomic<long long> last_refill_time;

public:
    TokenBucket(long long capacity, double refill_rate)
        : tokens(capacity), capacity(capacity), refill_rate(refill_rate) {
        last_refill_time.store(std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count(), std::memory_order_relaxed);
    }

    bool check(long long requested = 1) {
        refill();
        long long current = tokens.load(std::memory_order_relaxed);
        while (current >= requested) {
            if (tokens.compare_exchange_weak(current, current - requested, std::memory_order_relaxed)) {
                return true;
            }
        }
        return false;
    }

    void refill() {
        long long now = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
        long long last = last_refill_time.load(std::memory_order_relaxed);
        while (now > last) {
            long long delta_ms = now - last;
            long long new_tokens = static_cast<long long>(delta_ms * (refill_rate / 1000.0));
            if (new_tokens <= 0) break;
            if (last_refill_time.compare_exchange_weak(last, now, std::memory_order_relaxed)) {
                long long current = tokens.load(std::memory_order_relaxed);
                while (current < capacity) {
                    long long target = std::min(capacity, current + new_tokens);
                    if (tokens.compare_exchange_weak(current, target, std::memory_order_relaxed)) break;
                }
                break;
            }
        }
    }

    long long get_tokens() { refill(); return tokens.load(std::memory_order_relaxed); }
};

// --- 2. Leased Token Bucket (LTC Core - Innovation 1) ---
class LeasedTokenBucket {
private:
    std::atomic<long long> local_tokens;
    long long batch_size;
    double jitter_factor;
    std::atomic<bool> renewal_in_progress;
    static thread_local std::mt19937 gen;

public:
    LeasedTokenBucket(long long batch_size, double jitter_factor = 0.1)
        : local_tokens(batch_size), batch_size(batch_size), 
          jitter_factor(jitter_factor), renewal_in_progress(false) {}

    bool check(long long requested = 1) {
        // ATOMIC SUBTRACT & CHECK
        // Using fetch_sub ensures atomicity. If result < 0, we exceeded limit.
        long long prev = local_tokens.fetch_sub(requested, std::memory_order_relaxed);
        
        if (prev >= requested) {
            // Success. Trigger refresh if needed.
            if (should_probabilistic_refresh(prev - requested)) trigger_background_renewal();
            return true;
        } else {
            // Failed. Revert the subtract.
            local_tokens.fetch_add(requested, std::memory_order_relaxed);
            return false;
        }
    }

    void top_up(long long new_tokens) {
        local_tokens.fetch_add(new_tokens, std::memory_order_relaxed);
        renewal_in_progress.store(false, std::memory_order_release);
    }

    bool is_renewal_needed() { return renewal_in_progress.load(std::memory_order_acquire); }
    long long get_tokens() { return local_tokens.load(std::memory_order_relaxed); }

private:
    bool should_probabilistic_refresh(long long current) {
        long long upper = static_cast<long long>(batch_size * 0.3);
        long long lower = static_cast<long long>(batch_size * 0.1);
        if (current > upper || renewal_in_progress.load(std::memory_order_acquire)) return false;
        if (current < lower) return true;
        double probability = static_cast<double>(upper - current) / (upper - lower);
        std::uniform_real_distribution<> dis(0.0, 1.0);
        return dis(gen) < probability;
    }

    void trigger_background_renewal() {
        bool expected = false;
        renewal_in_progress.compare_exchange_strong(expected, true, std::memory_order_acq_rel);
    }
};

thread_local std::mt19937 LeasedTokenBucket::gen{std::random_device{}()};

// --- 3. Sliding Window Log (Classic - Lock-Free) ---
class SlidingWindowLog {
private:
    struct Bucket { std::atomic<long long> timestamp{0}; std::atomic<long long> count{0}; };
    std::unique_ptr<Bucket[]> buckets;
    size_t num_buckets;
    long long limit;
    long long window_ms;
    long long bucket_width_ms;

public:
    SlidingWindowLog(long long limit, long long window_ms)
        : num_buckets(64), limit(limit), window_ms(window_ms) {
        buckets = std::make_unique<Bucket[]>(num_buckets);
        bucket_width_ms = window_ms / num_buckets;
        if (bucket_width_ms == 0) bucket_width_ms = 1;
    }

    bool check() {
        long long now = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
        long long idx = (now / bucket_width_ms) % num_buckets;
        long long expected_ts = (now / bucket_width_ms) * bucket_width_ms;
        if (buckets[idx].timestamp.exchange(expected_ts, std::memory_order_acq_rel) != expected_ts) {
            buckets[idx].count.store(0, std::memory_order_relaxed);
        }
        long long total = 0;
        for (size_t i = 0; i < num_buckets; ++i) {
            if (now - buckets[i].timestamp.load(std::memory_order_acquire) < window_ms) {
                total += buckets[i].count.load(std::memory_order_relaxed);
            }
        }
        if (total < limit) { buckets[idx].count.fetch_add(1, std::memory_order_relaxed); return true; }
        return false;
    }
};

// --- 4. Leaky Bucket (Classic) ---
class LeakyBucket {
private:
    std::atomic<long long> level;
    long long capacity;
    double leak_rate;
    std::atomic<long long> last_leak_time;

public:
    LeakyBucket(long long capacity, double leak_rate)
        : level(0), capacity(capacity), leak_rate(leak_rate) {
        last_leak_time.store(std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count(), std::memory_order_relaxed);
    }

    bool check() {
        leak();
        long long current = level.load(std::memory_order_relaxed);
        while (current < capacity) {
            if (level.compare_exchange_weak(current, current + 1, std::memory_order_relaxed)) return true;
        }
        return false;
    }

    void leak() {
        long long now = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
        long long last = last_leak_time.load(std::memory_order_relaxed);
        while (now > last) {
            long long delta_ms = now - last;
            long long leaked = static_cast<long long>(delta_ms * (leak_rate / 1000.0));
            if (leaked <= 0) break;
            if (last_leak_time.compare_exchange_weak(last, now, std::memory_order_relaxed)) {
                long long current = level.load(std::memory_order_relaxed);
                while (current > 0) {
                    long long target = std::max(0LL, current - leaked);
                    if (level.compare_exchange_weak(current, target, std::memory_order_relaxed)) break;
                }
                break;
            }
        }
    }
    long long get_level() { leak(); return level.load(std::memory_order_relaxed); }
};

// --- 5. Fixed Window (Classic) ---
class FixedWindow {
private:
    std::atomic<long long> count;
    long long limit;
    long long window_ms;
    std::atomic<long long> window_start;

public:
    FixedWindow(long long limit, long long window_ms)
        : count(0), limit(limit), window_ms(window_ms) {
        window_start.store(std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count(), std::memory_order_relaxed);
    }

    bool check() {
        long long now = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
        long long start = window_start.load(std::memory_order_relaxed);
        if (now - start >= window_ms) {
            if (window_start.compare_exchange_strong(start, now, std::memory_order_relaxed)) {
                count.store(0, std::memory_order_relaxed);
            }
        }
        if (count.load(std::memory_order_relaxed) < limit) {
            count.fetch_add(1, std::memory_order_relaxed);
            return true;
        }
        return false;
    }
};

// --- 6. Probabilistic Shield (Innovation 2 - Bloom/CMS Mix) ---
class ProbabilisticShield {
private:
    std::vector<std::atomic<int>> counters;
    int limit;
    size_t size;

public:
    ProbabilisticShield(size_t size, int limit) : limit(limit), size(size) {
        counters = std::vector<std::atomic<int>>(size);
        for (size_t i = 0; i < size; ++i) counters[i].store(0, std::memory_order_relaxed);
    }

    bool check(const std::string& key) {
        size_t hash = std::hash<std::string>{}(key);
        size_t idx = hash % size;
        if (counters[idx].load(std::memory_order_relaxed) < limit) {
            counters[idx].fetch_add(1, std::memory_order_relaxed);
            return true;
        }
        return false;
    }
    
    void reset() { for (size_t i = 0; i < size; ++i) counters[i].store(0, std::memory_order_relaxed); }
};

} // namespace living_limiter
