from .living_limiter_core import TokenBucket

class RateLimiter:
    def __init__(self, capacity: int, refill_rate: float):
        self._core = TokenBucket(capacity, refill_rate)

    def check(self, requested: int = 1) -> bool:
        """Check if the request is allowed."""
        return self._core.check(requested)

    def get_remaining(self) -> int:
        """Get the number of tokens currently in the bucket."""
        return self._core.get_tokens()
