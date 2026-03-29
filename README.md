# Living Lab 🔬
> "I benchmarked 4 rate limiting algorithms so you don't have to."

Living Lab is my engineering organization dedicated to building ultra-high-performance, low-latency infrastructure components for modern distributed systems. I specialize in C++-core engines with native Python/Java/Go bindings.

## 🏛️ The Project: `living-limiter`
A 1,000,000+ req/s rate limiting engine using C++ atomics and my hybrid lease protocols.

### 📐 The Blueprint: High-Velocity Architecture
```mermaid
graph TD
    %% Global Styling
    classDef default fill:#1a1a1a,stroke:#333,stroke-width:2px,color:#fff;
    classDef client fill:#00d4ff,stroke:#00d4ff,stroke-width:4px,color:#000,font-weight:bold;
    classDef brain fill:#39ff14,stroke:#39ff14,stroke-width:2px,color:#000,font-weight:bold;
    classDef vault fill:#7b2ff7,stroke:#7b2ff7,stroke-width:2px,color:#fff;
    classDef sentinel fill:#ff3131,stroke:#ff3131,stroke-width:2px,color:#fff;
    classDef proceed fill:#00ffa3,stroke:#00ffa3,stroke-width:2px,color:#000;

    Client((Incoming Traffic)):::client -->|HTTP| Gateway(Spring Web Filter)

    subgraph "living-limiter: The Intelligence Core"
        Gateway -->|Extract Headers| Interceptor{The Sentinel}:::sentinel
        Interceptor --> Factory{Strategy Factory}

        Factory -->|"@RateLimit(strategy=TOKEN_BUCKET)"| TB((Token Bucket)):::brain
        Factory -->|"@RateLimit(strategy=LEAKY_BUCKET)"| LB((Leaky Bucket)):::brain
        Factory -->|"@RateLimit(strategy=FIXED_WINDOW)"| FW((Fixed Window)):::brain
        Factory -->|"@RateLimit(strategy=SLIDING_WINDOW)"| SW((Sliding Window)):::brain
    end

    subgraph "Distributed Persistence"
        TB -->|"Lua Script / EVALSHA"| Redis[(Global Redis Vault)]:::vault
        LB -->|"Lua Script / EVALSHA"| Redis
        FW -->|"INCR, EXPIRE"| Redis
        SW -->|"Pipelined: ZREM, ZCARD, ZADD"| Redis
    end

    Redis -.->|"Decision (Allow/Deny)"| Interceptor

    Interceptor -.->|"429 Too Many Requests"| Deny([Access Denied]):::sentinel
    Interceptor -->|Proceed| App([Upstream Service / Controller]):::proceed
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
