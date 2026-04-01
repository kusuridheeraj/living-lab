# Working Note: Jittered Lease Renewal Core (ltc-001)

## Problem Statement
Standard centralized rate limiting patterns fail at 1M+ RPS due to "Thundering Herds." When thousands of microservice instances expire their token leases at the same time, they simultaneously blast the backing store (Redis), causing cascading latency spikes or outages. We need a mechanism that randomizes lease renewal intervals to smooth out the global QPS profile.

## Acceptance Criteria
- **GIVEN** a `LeasedTokenBucket` initialized with a batch size and a jitter percentage.
- **WHEN** a request is made via `check()`.
- **THEN** it must perform a lock-free decrement of the local atomic counter.
- **WHEN** the local counter drops below the `renewal_threshold` (default 20% of batch size).
- **THEN** it must trigger an internal signal for background lease renewal.
- **WHEN** calculating the next renewal point.
- **THEN** it must apply a randomized jitter (e.g., +/- 10% of the refill interval) to ensure distributed staggering.
- **AND** the request path (`check()`) must never block on network I/O or mutexes.

## Implementation Details (C++)
- Class: `living_limiter::LeasedTokenBucket`
- Hot-path state: `std::atomic<long long> local_tokens`
- Sync state: `std::atomic<bool> renewal_in_progress`
- Jitter Mechanism: `std::mt19937` with `std::uniform_int_distribution` initialized per-instance.

## Out of Scope
- Actual Redis network integration (this spec focuses on the core logic and renewal signaling).
- Python middleware bindings (handled in `ltc-003`).

## Open Questions
1. Should the jitter be calculated based on **token count** or **time interval**? (Recommendation: Time interval staggered against the last refresh).
2. Do we require a hard "Fail Closed" if the renewal fails multiple times?

## Risk Flags
- **Memory Order:** Improper use of `std::memory_order_relaxed` vs `std::memory_order_seq_cst` could lead to subtle race conditions in high-concurrency environments.
- **RNG Performance:** Using a global lock for random number generation will kill performance at 1M RPS; must use thread-local or per-instance generators.
