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

    Client((Incoming Traffic)):::client -->|Raw Payload| Gateway(Spring Web Filter)
    
    subgraph "living-limiter: The Intelligence Core"
        Gateway -->|Extract Headers| Interceptor{The Sentinel}:::sentinel
        Interceptor --> Factory{Strategy Factory}
        
        Factory -->|Leased| TB((Token Bucket)):::brain
        Factory -->|Shaped| LB((Leaky Bucket)):::brain
        Factory -->|Precise| SWL((Sliding Window)):::brain
        Factory -->|Atomic| FW((Fixed Window)):::brain
    end

    subgraph "Distributed Persistence"
        TB <-->|Batch Sync| Redis[(Global Redis Vault)]:::vault
        LB <-->|Sync| Redis
        SWL <-->|Log Sync| Redis
        FW <-->|Atomic Sync| Redis
    end

    Redis -.->|Decision| Interceptor
    
    Interceptor -.->|BLOCKED: 429| Deny([Access Denied]):::sentinel
    Interceptor -->|ALLOWED| App([Upstream Service]):::proceed

    %% Links styling
    linkStyle default stroke:#555,stroke-width:2px;
    linkStyle 0,1,2,3,4,5,6 stroke:#00d4ff,stroke-width:2px;
    linkStyle 13 stroke:#00ffa3,stroke-width:4px;
    linkStyle 12 stroke:#ff3131,stroke-width:4px;
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
