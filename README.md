# Living Lab 🔬
> "I benchmarked 4 rate limiting algorithms so you don't have to."

Living Lab is my engineering organization dedicated to building ultra-high-performance, low-latency infrastructure components for modern distributed systems. I specialize in C++-core engines with native Python/Java/Go bindings.

## 🏛️ The Project: `living-limiter`
A 1,000,000+ req/s rate limiting engine using C++ atomics and my hybrid lease protocols.

### 📐 Project Architecture
```mermaid
graph TD
    Client[Incoming Request] -->|HTTP| Gateway(Spring Web Filter)
    
    subgraph "Rate Limiter Starter (Java 21 Virtual Threads)"
        Gateway -->|Extracts IP/User ID| Interceptor(RateLimit Interceptor)
        Interceptor --> Factory{Strategy Factory}
        
        Factory -->|"@RateLimit(strategy=TOKEN_BUCKET)"| TB[Token Bucket Engine]
        Factory -->|"@RateLimit(strategy=LEAKY_BUCKET)"| LB[Leaky Bucket Engine]
        Factory -->|"@RateLimit(strategy=FIXED_WINDOW)"| FW[Fixed Window Engine]
        Factory -->|"@RateLimit(strategy=SLIDING_WINDOW)"| SW[Sliding Window Engine]
    end

    subgraph "Distributed State"
        TB -->|"Lua Script / EVALSHA"| Redis[(Reactive Redis cluster)]
        LB -->|"Lua Script / EVALSHA"| Redis
        FW -->|"INCR, EXPIRE"| Redis
        SW -->|"Pipelined: ZREM, ZCARD, ZADD"| Redis
    end

    Redis -.->|"Decision (Allow/Deny)"| Interceptor
    
    Interceptor -->|"429 Too Many Requests"| Client
    Interceptor -->|Proceed| Controller(Upstream Service / Controller)
```

## 🧬 My Engineering Principles
1. **Performance First:** Everything I build is in C++/Java with a focus on lock-free concurrency.
2. **The Living Lab:** Every project includes a suite of benchmarks comparing at least 4 classic algorithms against my own innovations.
3. **Surgical Integration:** Native bindings for popular languages (Python, Java) to ensure zero-effort adoption for high-scale teams.
4. **Transparency:** Real-world metrics, p99 latency reports, and failure-mode analysis are built into every PRD I write.

## 🛠️ Tech Stack
- **Core:** C++20 / Atomics / Lock-free Data Structures
- **Bindings:** pybind11 (C++), JNI (Java)
- **Scale:** Target throughput of 1M+ transactions per second.
