# Living Lab 🔬
> "We benchmark the hard stuff so you don't have to."

Living Lab is an engineering organization dedicated to building ultra-high-performance, low-latency infrastructure components for modern distributed systems. We specialize in C++-core engines with native Python/Java/Go bindings.

## 🏛️ Our Projects
- **[living-limiter](./rate-limiter-toolkit/):** A 1,000,000+ req/s rate limiting engine using C++ atomics and hybrid lease protocols.

## 🧬 The Living Lab Principles
1. **Performance First:** Everything is built in C++/Java with a focus on lock-free concurrency.
2. **The Living Lab:** Every project includes a suite of benchmarks comparing at least 4 classic algorithms against our own innovations.
3. **Surgical Integration:** Native bindings for popular languages (Python, Java) to ensure zero-effort adoption for high-scale teams.
4. **Transparency:** Real-world metrics, p99 latency reports, and failure-mode analysis are built into every PRD.

## 🛠️ Tech Stack
- **Core:** C++20 / Atomics / Lock-free Data Structures
- **Bindings:** pybind11 (C++), JNI (Java)
- **Scale:** Target throughput of 1M+ transactions per second.
