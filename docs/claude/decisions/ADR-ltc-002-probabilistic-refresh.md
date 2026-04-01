# ADR-ltc-002: Probabilistic Early Refresh

## Status
ACCEPTED

## Context
Even with jitter, nodes renewing at a fixed threshold (e.g., exactly 20%) can create micro-bursts.

## Decision
We will wrap the jittered renewal in a probabilistic trigger.
1. **The Dice Roll:** When tokens are between 10% and 30%, every request rolls a dice.
2. **The Formula:** `P(refresh) = (UpperThreshold - CurrentTokens) / Range`.
3. **Optimization:** Decision logic will use integer arithmetic where possible to minimize floating-point overhead in the hot-path.

## Consequences
- **Performance:** Adds ~5-10ns per request in the "Watch Zone."
- **Efficiency:** Drastically reduces synchronized Redis QPS by spreading renewals over a wider temporal window.
