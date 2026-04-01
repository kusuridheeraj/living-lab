# Working Note: Probabilistic Early Refresh Logic (ltc-002)

## Problem Statement
Fixed-threshold renewals (e.g., "Renew at 20%") still create micro-bursts of traffic. While jitter helps, we can further smooth the load by using probability. Instead of all nodes renewing exactly at 20%, nodes should start rolling a probability dice once they hit a "Watch Zone." This ensures that the global renewal rate is a smooth curve rather than a jagged step function.

## Acceptance Criteria
- **GIVEN** a `LeasedTokenBucket` in the "Watch Zone" (e.g., tokens between 10% and 30% of batch size).
- **WHEN** a request is made.
- **THEN** the system must calculate a renewal probability `P` based on the remaining token percentage.
- **AND** roll a randomized value to decide if an "Early Refresh" should be triggered.
- **THEN** if the roll succeeds, trigger the background renewal.
- **AND** ensure that only one renewal is triggered at a time per node (idempotency).

## Implementation Details (C++)
- Probability Formula: `P = (Threshold - Current) / Threshold` (linear increase as tokens drop).
- Integration: This logic wraps the Jittered Renewal signaling path.
- State: Atomic check on `renewal_in_progress` to avoid redundant concurrent rolls.

## Out of Scope
- Metric tracking for "Roll Success Rate" (handled in `metrics.md` via the Profiler).

## Open Questions
1. Should the probability curve be **Linear** or **Exponential**? (Recommendation: Linear for predictability, Exponential for extreme thundering herd protection).

## Risk Flags
- **CPU Overhead:** Floating-point division and RNG rolls on every request could add nanoseconds to the hot path. Optimization: Only roll every `N` requests or use integer-based bit-shifting for the "dice."
