# ADR-ltc-003: FastAPI Sentinel Interceptor

## Status
ACCEPTED

## Context
High-performance C++ must be accessible via FastAPI without introducing massive overhead or blocking the event loop.

## Decision
1. **Integration Pattern:** Use a FastAPI `BaseHTTPMiddleware` for global protection and a custom Decorator for fine-grained control.
2. **Async Bridge:** The C++ `check()` call remains synchronous (nanoseconds). Background renewals in C++ MUST use a separate thread pool or detached threads to avoid blocking the Python GIL.
3. **Error Handling:** If the C++ core throws or Redis is unreachable, the interceptor defaults to the `fail_strategy` (OPEN for reads, CLOSED for writes).

## Consequences
- **Readability:** Clean `@rate_limit` syntax for developers.
- **Observability:** Injects `X-RateLimit` headers automatically.
- **Risk:** Minor overhead of Python-to-C++ transition (~100-200ns).
