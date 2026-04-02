import time
import pytest
import threading
from living_limiter import RateLimiter

def test_sliding_window_log_integrated():
    # Limit 10 requests per 100ms
    limiter = RateLimiter(capacity=10, window_ms=100, strategy="sliding_window")
    
    # 10 should pass
    for _ in range(10):
        assert limiter.check() is True
    
    # 11th should fail
    assert limiter.check() is False
    
    # Wait for window to slide
    time.sleep(0.15)
    assert limiter.check() is True

def test_leaky_bucket_integrated():
    # Capacity 5, leak 10/sec (1 every 100ms)
    limiter = RateLimiter(capacity=5, refill_rate=10, strategy="leaky_bucket")
    
    for _ in range(5):
        assert limiter.check() is True
    assert limiter.check() is False
    
    time.sleep(0.2) # Leak 2 tokens
    assert limiter.check() is True
    assert limiter.check() is True
    assert limiter.check() is False

def test_atomic_refill_no_loss():
    # Capacity 1000, refill 1000/sec
    limiter = RateLimiter(capacity=1000, refill_rate=1000, strategy="token_bucket")
    
    # Drain completely
    for _ in range(1000):
        limiter.check()
    
    def worker():
        for _ in range(100):
            limiter.check()

    # Multiple threads hitting refill logic
    threads = [threading.Thread(target=worker) for _ in range(10)]
    for t in threads: t.start()
    for t in threads: t.join()
    
    # After some time, tokens should be replenished correctly
    time.sleep(0.1)
    assert limiter.get_remaining() > 0
