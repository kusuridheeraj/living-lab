# The Nanosecond Tax: Architecting `living-limiter` for 1M RPS

I benchmarked 4 rate limiting algorithms so you don't have to. 

Engineering is not the art of building; it is the art of trade-offs. 

When I scale to **1 million requests per second**, the "common" architectural patterns I use for 10k RPS don't just slow down—they become liabilities. This is the story of how I built `living-limiter` to pay the "nanosecond tax" on every request without going bankrupt.

## 1. The Distributed Bottleneck: Why Redis Melts
The standard rate-limiting pattern is simple: `INCR` a key in Redis, check the result, and return. At 1M RPS, this pattern costs:
- **Network Latency:** Even at 0.1ms, you’re limited by the speed of light.
- **I/O Saturation:** Redis is single-threaded; at 1M commands/sec, its event loop spends more time parsing TCP packets than enforcing logic.

I solved this with **Leased Batching**. Instead of asking Redis for permission every time, my C++ core "leases" a batch of 500 tokens. For the next 500 requests, the app is a sovereign authority. The latency drops from **0.1ms** (Network) to **0.5ns** (L1 Cache). 

## 2. Lock-Free or Bust
In high-concurrency environments, **Mutexes are an anti-pattern.** A mutex is a "Stop" sign. When 16 cores hit the same mutex, they spend more time waiting in a "Context Switch" than they do performing actual work.

In `living-limiter`, I used **C++20 std::atomic** with `memory_order_relaxed`. This allows the CPU to increment counters directly in the L3 cache without a global lock. I am not just fast; I am mechanically sympathetic.

## 3. The "Hybrid Vault" Strategy
Most developers think rate limiting is a binary: Strict or Approximate. I disagree. 
- **The Vault (Strict):** For critical paths like `/checkout`, I enforce 100% accuracy. I accept the latency cost because overselling is a business failure.
- **The Shield (Probabilistic):** For `/search`, I use **Count-Min Sketches**. I don't store every UserID (O(N) memory is a trap). I use sub-linear memory to identify "Heavy Hitters" and shield the backend from scrapers.

## 4. Why C++ as the Engine?
People ask why I didn't just use Python. The answer is the **Memory Wall.** 
Python’s Garbage Collector (GC) and GIL (Global Interpreter Lock) introduce "Stop-the-World" pauses. At 1M RPS, a 10ms GC pause drops 10,000 requests. By building the engine in **C++** and bridging it to **Python via pybind11**, I get the agility of an AI-native stack with the surgical precision of native code.

## 5. Conclusion
`living-limiter` is more than a package; it's a demonstration that infrastructure should be invisible. If you are paying more than a few nanoseconds for your rate limiter, you are paying too much.

---
*Authored by Kusuri Dheeraj Kumar.*
