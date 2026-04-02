import time
import pytest
from living_limiter import RateLimiter, rate_limit, FastAPISentinelMiddleware
from fastapi import FastAPI, HTTPException
from fastapi.testclient import TestClient

def test_leased_token_bucket_basic():
    # Batch size 100
    limiter = RateLimiter(capacity=100, refill_rate=0, strategy="leased")
    
    # Check 50 requests (consumes 50 tokens)
    for _ in range(50):
        assert limiter.check() is True
        
    # We should still have 50 tokens. Consume all 50.
    assert limiter.check(requested=50) is True
    # Now we should be empty
    assert limiter.check() is False

def test_probabilistic_refresh_trigger():
    # Batch size 1000
    limiter = RateLimiter(capacity=1000, refill_rate=0, strategy="leased")
    
    # Drain until we hit the "Watch Zone" (under 300 tokens)
    for _ in range(750):
        limiter.check()
        
    # Check if renewal is needed (probabilistic)
    triggered = False
    for _ in range(100):
        if limiter._core.is_renewal_needed():
            triggered = True
            break
        limiter.check()
        
    assert triggered is True, "Probabilistic refresh should have triggered"

def test_top_up_logic():
    limiter = RateLimiter(capacity=100, refill_rate=0, strategy="leased")
    
    # Drain
    for _ in range(100):
        limiter.check()
    assert limiter.check() is False
    
    # Top up from "Redis" (simulated)
    limiter._core.top_up(100)
    
    # Should be able to check again
    assert limiter.check() is True

# --- FastAPI Integration Tests (ltc-003) ---

def test_fastapi_decorator():
    app = FastAPI()
    limiter = RateLimiter(capacity=5, refill_rate=0, strategy="token_bucket")

    @app.get("/")
    @rate_limit(limiter)
    async def root():
        return {"message": "success"}

    client = TestClient(app)

    # 5 requests should pass
    for i in range(5):
        response = client.get("/")
        assert response.status_code == 200
    
    # 6th should be limited
    response = client.get("/")
    assert response.status_code == 429

def test_fastapi_middleware():
    app = FastAPI()
    limiter = RateLimiter(capacity=2, refill_rate=0, strategy="token_bucket")
    app.add_middleware(FastAPISentinelMiddleware, limiter=limiter)

    @app.get("/test")
    async def test_route():
        return {"message": "ok"}

    client = TestClient(app)

    assert client.get("/test").status_code == 200
    assert client.get("/test").status_code == 200
    assert client.get("/test").status_code == 429
