# Working Note: FastAPI Sentinel Interceptor (ltc-003)

## Problem Statement
The high-performance C++ core is useless if it is hard to use. We need a Pythonic integration that allows FastAPI developers to protect their routes with a single line of code. This middleware must pay the "nanosecond tax" efficiently, bridging the Python `async` event loop to the synchronous atomic checks in the C++ Sentinel core.

## Acceptance Criteria
- **GIVEN** a FastAPI application.
- **WHEN** a route is decorated with `@rate_limit(key="...", strategy="...")`.
- **THEN** it must call the C++ `living_limiter::Sentinel::check()` engine before reaching the route logic.
- **IF** the check returns `False` (Limited):
  - **THEN** it must return an HTTP 429 "Too Many Requests" response.
  - **AND** include standard `X-RateLimit` headers.
- **IF** the check returns `True` (Allowed):
  - **THEN** it must proceed to the route handler with zero detectable latency.
- **AND** the middleware must support global "Fail Open" configuration if the C++ core or Redis sync path hangs.

## Implementation Details (Python)
- Decorator: `@rate_limit` using `functools.wraps`.
- Middleware: `FastAPISentinelMiddleware(BaseHTTPMiddleware)`.
- Bridge: Direct call to the `living_limiter_core` binary module.

## Out of Scope
- Support for Django or Flask (LTC focus is on high-QPS FastAPI).
- Authentication logic (the middleware assumes the `key` is provided).

## Open Questions
1. Should the middleware use **Dependencies** (`Depends`) or **Middleware**? (Recommendation: Middleware for global protection, Decorators for route-specific tuning).
2. How do we handle `async` C++ calls if the lease renewal is blocking? (Recommendation: Lease renewal must be background-threaded in C++ to avoid blocking the Python GIL).

## Risk Flags
- **GIL Contention:** Frequent calls from Python to C++ could cause minor GIL overhead if not handled via zero-copy.
- **Error Propagation:** C++ exceptions must be caught and translated to Python HTTP 500s or handled as "Fail Open."
