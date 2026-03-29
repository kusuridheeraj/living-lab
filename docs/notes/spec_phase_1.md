# Living Lab: living-limiter - Phase 1: High-Performance C++ Core
**Architecture:** C++ (pybind11) + Python (Asyncio)
**Package Name:** `living-limiter`

## 1.1 Why C++ for the Core?
To handle **1 million sales/sec**, we need a language with zero overhead and direct access to CPU hardware instructions (Atomics).
- **Python (Async):** ~2k-5k req/s before hitting CPU limits.
- **C++ (std::atomic):** ~250k-500k req/s per core.

We will use **pybind11** to wrap the C++ core. This means the user runs `pip install living-limiter` and gets a pre-compiled binary that Python calls directly.

## 1.2 Directory Structure
```text
living-lab/rate-limiter-toolkit/
├── src/                # C++ source code
│   ├── core.cpp        # Atomics, Token Bucket logic
│   ├── bindings.cpp    # pybind11 definitions
│   └── redis_client.h  # Shared Redis logic
├── python/             # Python package
│   └── living_limiter/   # Asyncio wrappers, Decorators
├── docs/notes/         # Your requested structure
├── bugs.md
├── check.md
└── GEMINI.md           # Local overrides
```

## 1.3 The "Hybrid Vault" Algorithm (100% Accuracy)
We will use **Leased Token Buckets** to achieve 100% accuracy without killing Redis.

**The "Strict" Strategy (For Payments):**
1.  **Request comes in:** Check local C++ atomic counter.
2.  **Tokens available:** Allow request (0ms latency).
3.  **Tokens < 10%:** C++ background thread fires `async` request to Redis.
4.  **Redis Update:** `INCRBY limit -batch_size`.
5.  **Failure Case:** If Redis is down, this endpoint **Fails Closed**. Better to lose a few sales than to oversell inventory by 10,000 units.

**The "High-Velocity" Strategy (For Browsing):**
-   Uses **Bloom Filters** to ignore first-time visitors (90% of traffic).
-   Uses **Count-Min Sketch** to estimate frequency of "Heavy Hitters".
-   **Fails Open:** If the system is overloaded, keep serving. Customer experience > perfect limiting.

## 1.4 Quickstart Strategy (uv/pip)
We will use **cibuildwheel** in GitHub Actions to build Python Wheels for all OSs.
User command: `pip install living-limiter`
Usage:
```python
from living_limiter import RateLimiter

limiter = RateLimiter(redis_url="redis://localhost", mode="strict")
allowed = await limiter.check(user_id="kusuri")
```
