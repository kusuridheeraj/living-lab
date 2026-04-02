import os
import sys

# --- DLL Load Fix for Windows (MinGW/MSYS2) ---
if sys.platform == "win32":
    mingw_path = "C:\\msys64\\mingw64\\bin"
    if os.path.exists(mingw_path):
        os.add_dll_directory(mingw_path)

import functools
import asyncio
from typing import Callable, Optional, Union, Dict, Any
from .living_limiter_core import (
    TokenBucket, LeasedTokenBucket, SlidingWindowLog, 
    LeakyBucket, FixedWindow, ProbabilisticShield
)

class RateLimiter:
    def __init__(
        self, 
        capacity: int, 
        refill_rate: float = 0, 
        window_ms: int = 1000, 
        strategy: str = "token_bucket",
        jitter_factor: float = 0.1,
        shield_size: int = 10000
    ):
        self.strategy = strategy
        if strategy == "token_bucket":
            self._core = TokenBucket(capacity, refill_rate)
        elif strategy == "leased":
            self._core = LeasedTokenBucket(capacity, jitter_factor)
        elif strategy == "sliding_window":
            self._core = SlidingWindowLog(capacity, window_ms)
        elif strategy == "leaky_bucket":
            self._core = LeakyBucket(capacity, refill_rate)
        elif strategy == "fixed_window":
            self._core = FixedWindow(capacity, window_ms)
        elif strategy == "probabilistic_shield":
            self._core = ProbabilisticShield(shield_size, capacity)
        else:
            raise ValueError(f"Unknown strategy: {strategy}")

    def check(self, key: str = "default", requested: int = 1) -> bool:
        """Check if the request is allowed."""
        if self.strategy == "probabilistic_shield":
            return self._core.check(key)
        
        if self.strategy in ["sliding_window", "leaky_bucket", "fixed_window"]:
            return self._core.check()
            
        # TokenBucket and LeasedTokenBucket support requested
        # We must use positional or keyword correctly. 
        # C++ bindings expect requested as first arg if no key is present.
        return self._core.check(requested)

    def get_remaining(self) -> int:
        """Get remaining tokens or current level."""
        if hasattr(self._core, 'get_tokens'):
            return self._core.get_tokens()
        if hasattr(self._core, 'get_level'):
            return self._core.get_level()
        return -1

# --- FastAPI Integration ---

def rate_limit(limiter: RateLimiter, key_func: Optional[Callable] = None, fail_open: bool = True):
    def decorator(func: Callable):
        @functools.wraps(func)
        async def wrapper(*args, **kwargs):
            from fastapi import HTTPException, Request
            allowed = False
            key = "global"
            if key_func:
                request = next((arg for arg in args if isinstance(arg, Request)), None)
                if request: key = key_func(request)
            try:
                allowed = limiter.check(key=key)
            except Exception as e:
                if not fail_open: raise e
                allowed = True
            if not allowed:
                raise HTTPException(status_code=429, detail="Too Many Requests")
            return await func(*args, **kwargs)
        return wrapper
    return decorator

class FastAPISentinelMiddleware:
    def __init__(self, app, limiter: RateLimiter, key_func: Optional[Callable] = None, fail_open: bool = True):
        self.app = app
        self.limiter = limiter
        self.fail_open = fail_open
        self.key_func = key_func or (lambda scope: "global")

    async def __call__(self, scope, receive, send):
        if scope["type"] != "http":
            await self.app(scope, receive, send)
            return
        try:
            key = self.key_func(scope)
            allowed = self.limiter.check(key=key)
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
            "type": "http.response.start", "status": 429,
            "headers": [(b"content-type", b"application/json")],
        })
        await send({"type": "http.response.body", "body": b'{"detail": "Too Many Requests"}'})

    async def _send_500(self, send):
        await send({
            "type": "http.response.start", "status": 500,
            "headers": [(b"content-type", b"application/json")],
        })
        await send({"type": "http.response.body", "body": b'{"detail": "Internal Sentinel Error"}'})
