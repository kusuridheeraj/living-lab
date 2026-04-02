import time
import pytest
from living_limiter import RateLimiter

def test_fixed_window():
    print("\nTesting Fixed Window...")
    limiter = RateLimiter(capacity=5, window_ms=100, strategy="fixed_window")
    for i in range(5):
        assert limiter.check() is True, f"Request {i+1} failed"
    assert limiter.check() is False, "6th request should fail"
    time.sleep(0.11)
    assert limiter.check() is True, "Should reset after window"

def test_probabilistic_shield():
    print("\nTesting Probabilistic Shield...")
    # Small shield size to force collisions if needed, limit 2
    limiter = RateLimiter(capacity=2, strategy="probabilistic_shield", shield_size=100)
    
    assert limiter.check(key="user1") is True
    assert limiter.check(key="user1") is True
    assert limiter.check(key="user1") is False, "user1 should be shielded"
    
    assert limiter.check(key="user2") is True, "user2 should be allowed"

def test_all_6_strategies_exist():
    strategies = [
        "token_bucket", "leased", "sliding_window", 
        "leaky_bucket", "fixed_window", "probabilistic_shield"
    ]
    for s in strategies:
        print(f"Verifying existence of {s}...")
        limiter = RateLimiter(capacity=10, strategy=s)
        assert limiter.strategy == s

def test_leased_bucket_depletion():
    print("\nDeep-testing Leased Bucket Depletion...")
    # Batch size 10
    limiter = RateLimiter(capacity=10, strategy="leased")
    
    # Check current tokens
    print(f"Initial tokens: {limiter.get_remaining()}")
    
    # Consume 5
    for i in range(5):
        allowed = limiter.check()
        print(f"Req {i+1}: allowed={allowed}, remaining={limiter.get_remaining()}")
        assert allowed is True
        
    # Consume 5 more
    allowed = limiter.check(requested=5)
    print(f"Req 6-10 (bulk): allowed={allowed}, remaining={limiter.get_remaining()}")
    assert allowed is True
    
    # 11th should fail
    allowed = limiter.check()
    print(f"Req 11: allowed={allowed}, remaining={limiter.get_remaining()}")
    assert allowed is False
