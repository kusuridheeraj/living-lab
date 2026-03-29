import time
import asyncio
from living_limiter import RateLimiter

async def run_benchmark(limiter, name, duration=5, target_rps=20000):
    print(f"--- Benchmarking: {name} ---")
    start_time = time.perf_counter()
    requests = 0
    allowed = 0
    
    # Target total requests over duration
    total_requests = target_rps * duration
    
    for _ in range(total_requests):
        if limiter.check():
            allowed += 1
        requests += 1
        
    end_time = time.perf_counter()
    elapsed = end_time - start_time
    rps = requests / elapsed
    
    print(f"Total Requests: {requests}")
    print(f"Allowed: {allowed}")
    print(f"Elapsed Time: {elapsed:.2f}s")
    print(f"RPS: {rps:.2f}")
    print(f"---------------------------\n")

async def main():
    # Token Bucket Benchmark
    # Capacity 1M, refill 100k/s
    limiter = RateLimiter(capacity=1000000, refill_rate=100000.0)
    await run_benchmark(limiter, "Token Bucket")

if __name__ == "__main__":
    asyncio.run(main())
