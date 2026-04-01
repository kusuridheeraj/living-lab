# ADR-ltc-001: Jittered Lease Renewal Mechanism

## Status
ACCEPTED

## Context
Standard rate limiting causes synchronized load spikes (thundering herds) on the central store. To hit 1M+ RPS, nodes must renew leases at staggered intervals.

## Decision
We will implement a jittered renewal strategy in the `LeasedTokenBucket`.
1. **Random Number Generation:** Use `thread_local std::mt19937` to avoid global lock contention.
2. **Jitter Calculation:** Jitter will be +/- 10% of the lease duration, applied at the point of the next renewal calculation.
3. **Atomic Hand-off:** Use `std::atomic<bool> renewal_in_progress` with `std::memory_order_acquire/release` to ensure only one thread triggers the background renewal task.

## Consequences
- **Positive:** Smooths global QPS load on Redis.
- **Negative:** Slight variability in local token availability; bounded inaccuracy in global limit enforcement (acceptable per PROJECT_CHARTER).
- **Security:** Use `std::random_device` for seeding to prevent predictable jitter patterns.
