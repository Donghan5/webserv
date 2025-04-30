#!/usr/bin/env python3
import requests
import time
import threading
import argparse
import sys
import random

class RequestThread(threading.Thread):
    def __init__(self, url, thread_id, requests_per_thread, delay=0):
        threading.Thread.__init__(self)
        self.url = url
        self.thread_id = thread_id
        self.requests_per_thread = requests_per_thread
        self.delay = delay
        self.success_count = 0
        self.failure_count = 0
        
    def run(self):
        paths = [
            "/",
            "/about.html",
            "/css/style.css",
            "/js/script.js",
            "/cgi-bin/test.py",
            "/cgi-bin/env.py",
            "/nonexistent"
        ]
        
        for i in range(self.requests_per_thread):
            try:
                path = random.choice(paths)
                start_time = time.time()
                r = requests.get(f"{self.url}{path}")
                elapsed = time.time() - start_time
                
                if r.status_code < 400:
                    self.success_count += 1
                else:
                    self.failure_count += 1
                    
                if self.delay > 0:
                    time.sleep(self.delay)
                    
            except Exception:
                self.failure_count += 1

def main():
    parser = argparse.ArgumentParser(description="Stress test a web server")
    parser.add_argument("--url", default="http://localhost:8080", help="Base URL of the server to test")
    parser.add_argument("--threads", type=int, default=10, help="Number of threads")
    parser.add_argument("--requests", type=int, default=100, help="Requests per thread")
    parser.add_argument("--delay", type=float, default=0.01, help="Delay between requests per thread (seconds)")
    args = parser.parse_args()
    
    try:
        print(f"Starting stress test against {args.url}")
        print(f"Using {args.threads} threads with {args.requests} requests per thread")
        print(f"Total requests that will be sent: {args.threads * args.requests}")
        
        # Check if server is running
        try:
            requests.get(args.url, timeout=2)
        except requests.exceptions.ConnectionError:
            print(f"ERROR: Could not connect to {args.url}")
            sys.exit(1)
        
        threads = []
        start_time = time.time()
        
        # Create and start threads
        for i in range(args.threads):
            thread = RequestThread(args.url, i, args.requests, args.delay)
            threads.append(thread)
            thread.start()
        
        # Wait for all threads to complete
        for thread in threads:
            thread.join()
        
        total_time = time.time() - start_time
        total_requests = args.threads * args.requests
        success_count = sum(t.success_count for t in threads)
        failure_count = sum(t.failure_count for t in threads)
        
        print("\nStress Test Results:")
        print(f"Total time: {total_time:.2f} seconds")
        print(f"Requests per second: {total_requests / total_time:.2f}")
        print(f"Successful requests: {success_count} ({success_count / total_requests * 100:.2f}%)")
        print(f"Failed requests: {failure_count} ({failure_count / total_requests * 100:.2f}%)")
        
    except KeyboardInterrupt:
        print("\nStress test interrupted!")
        sys.exit(0)
    except Exception as e:
        print(f"ERROR: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()