# Project Charter: living-limiter (LTC)
**Vision:** Build an ultra-high-performance Local Traffic Controller (LTC) for 1M+ RPS microservices.

## 🏛️ Core Product Definition
`living-limiter` is a C++20-powered engine with Python bindings designed to move rate-limiting decisions from centralized bottlenecks (Redis/Nginx) directly to the application edge.

## 🧬 Key Innovations
1.  **Jittered Lease Renewal:** Eliminates "Thundering Herd" by randomizing lease refresh offsets across distributed nodes.
2.  **Probabilistic Early Refresh:** Reduces synchronized Redis load by ensuring only a subset of nodes hit the backing store under high contention.
3.  **Advanced Shared Memory Mode:** Employs **Versioned Generation Counters** (Seqlock-style) for sub-microsecond, lock-free limiting between co-located services.
4.  **Fault Tolerance:** Lazy rehydration from Redis for shared memory segments to recover from process crashes without kernel-space overhead (Robust Futexes).

## 🗺️ Roadmap & Milestones
- **Horizon 1 (MVP - 3 Months):** C++ core with Jittered Leases + FastAPI Middleware. Target: 1M RPS on a single node.
- **Horizon 2 (Scale - 6 Months):** Integrated **Circuit Breaking** and **Adaptive Throttling** sharing the same C++ decision path.
- **Horizon 3 (Vision - 2 Years):** Bare-metal LTC for K8s sidecars using the Advanced Shared Memory mode.

## 🚫 Non-Goals
- **Distributed Locking:** Avoided to prevent consistency-induced latency bloat. This project prioritizes availability and latency-first design.

## 🛠️ Engineering Standard
- **Lock-Free Atomics:** Mandatory for all hot-path counters.
- **Mechanical Sympathy:** Optimized for L1/L2 cache efficiency and minimal context switching.
- **Surgical Python Integration:** Zero-copy bindings via `pybind11`.
