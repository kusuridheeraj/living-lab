import functools
import asyncio
import os
import sys
from typing import Callable, Optional

# --- DLL Load Fix for Windows (MinGW/MSYS2) ---
if sys.platform == "win32":
    mingw_path = "C:\\msys64\\mingw64\\bin"
    if os.path.exists(mingw_path):
        os.add_dll_directory(mingw_path)

from .living_limiter_core import TokenBucket, LeasedTokenBucket, SlidingWindowLog, LeakyBucket

class RateLimiter:
    def __init__(self, capacity: int, refill_rate: float, strategy: str = "token_bucket"):
        if strategy == "token_bucket":
            self._core = TokenBucket(capacity, refill_rate)
        elif strategy == "leased":
            self._core = LeasedTokenBucket(capacity) # capacity is batch_size here
        else:
            raise ValueError(f"Unknown strategy: {strategy}")
        self.strategy = strategy

    def check(self, requested: int = 1) -> bool:
        """Check if the request is allowed."""
        return self._core.check(requested)

    def get_remaining(self) -> int:
        """Get remaining tokens (if supported)."""
        if hasattr(self._core, 'get_tokens'):
            return self._core.get_tokens()
        return -1

# --- FastAPI Integration (ltc-003) ---

def rate_limit(limiter: RateLimiter, key: str = "default", fail_open: bool = True):
    """
    FastAPI Decorator for route-specific rate limiting.
    """
    def decorator(func: Callable):
        @functools.wraps(func)
        async def wrapper(*args, **kwargs):
            from fastapi import HTTPException
            allowed = False
            try:
                # Synchronous atomic check in C++ core (nanoseconds)
                allowed = limiter.check()
            except Exception as e:
                if not fail_open:
                    raise e
                allowed = True # Fail open

            if not allowed:
                raise HTTPException(status_code=429, detail="Too Many Requests")
            
            return await func(*args, **kwargs)
        return wrapper
    return decorator

class FastAPISentinelMiddleware:
    """
    FastAPI Middleware for global rate limiting protection.
    """
    def __init__(self, app, limiter: RateLimiter, fail_open: bool = True):
        self.app = app
        self.limiter = limiter
        self.fail_open = fail_open

    async def __call__(self, scope, receive, send):
        if scope["type"] != "http":
            await self.app(scope, receive, send)
            return

        try:
            allowed = self.limiter.check()
            if not allowed:
                await self._send_429(send)
                return
        except Exception:
            if not self.fail_open:
                await self._send_500(send)
                return

        await self.app(scope, receive, send)

    async def _send_429(self, send):
        await send({
            "type": "http.response.start",
            "status": 429,
            "headers": [(b"content-type", b"application/json")],
        })
        await send({
            "type": "http.response.body",
            "body": b'{"detail": "Too Many Requests"}',
        })

    async def _send_500(self, send):
        await send({
            "type": "http.response.start",
            "status": 500,
            "headers": [(b"content-type", b"application/json")],
        })
        await send({
            "type": "http.response.body",
            "body": b'{"detail": "Internal Sentinel Error"}',
        })
