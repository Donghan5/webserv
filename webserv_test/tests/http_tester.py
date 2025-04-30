#!/usr/bin/env python3
import requests
import time
import argparse
import sys

def test_get(base_url):
    print("Testing GET requests...")
    
    # Test basic GET request
    r = requests.get(f"{base_url}/")
    print(f"GET / : {r.status_code}")
    
    # Test 404
    r = requests.get(f"{base_url}/nonexistent")
    print(f"GET /nonexistent : {r.status_code}")
    
    # Test static file
    r = requests.get(f"{base_url}/css/style.css")
    print(f"GET /css/style.css : {r.status_code}")
    
    # Test directory listing if enabled
    r = requests.get(f"{base_url}/uploads/")
    print(f"GET /uploads/ : {r.status_code}")
    
    # Test CGI script
    r = requests.get(f"{base_url}/cgi-bin/test.py")
    print(f"GET /cgi-bin/test.py : {r.status_code}")
    
    print("GET tests completed\n")

def test_post(base_url):
    print("Testing POST requests...")
    
    # Test form submission
    data = {
        "name": "Test User",
        "email": "test@example.com",
        "message": "This is a test message"
    }
    r = requests.post(f"{base_url}/cgi-bin/form.py", data=data)
    print(f"POST /cgi-bin/form.py : {r.status_code}")
    
    # Test file upload
    files = {'filename': ('test.txt', 'This is test content for the file')}
    r = requests.post(f"{base_url}/cgi-bin/upload.py", files=files)
    print(f"POST /cgi-bin/upload.py (file upload) : {r.status_code}")
    
    print("POST tests completed\n")

def test_delete(base_url):
    print("Testing DELETE requests...")
    
    # Create a test file first
    files = {'filename': ('delete_test.txt', 'This file will be deleted')}
    r = requests.post(f"{base_url}/cgi-bin/upload.py", files=files)
    
    # Then try to delete it
    r = requests.delete(f"{base_url}/uploads/delete_test.txt")
    print(f"DELETE /uploads/delete_test.txt : {r.status_code}")
    
    print("DELETE tests completed\n")

def test_redirects(base_url):
    print("Testing redirects...")
    
    # Test permanent redirect
    r = requests.get(f"{base_url}/old", allow_redirects=False)
    print(f"GET /old (redirect) : {r.status_code} -> {r.headers.get('Location', 'No redirect')}")
    
    # Test temporary redirect
    r = requests.get(f"{base_url}/temp", allow_redirects=False)
    print(f"GET /temp (redirect) : {r.status_code} -> {r.headers.get('Location', 'No redirect')}")
    
    print("Redirect tests completed\n")

def main():
    parser = argparse.ArgumentParser(description="Test HTTP functionality of a web server")
    parser.add_argument("--url", default="http://localhost:8080", help="Base URL of the server to test")
    args = parser.parse_args()
    
    try:
        print(f"Starting tests against {args.url}\n")
        
        test_get(args.url)
        test_post(args.url)
        test_delete(args.url)
        test_redirects(args.url)
        
        print("All tests completed successfully!")
        
    except requests.exceptions.ConnectionError:
        print(f"ERROR: Could not connect to {args.url}")
        sys.exit(1)
    except Exception as e:
        print(f"ERROR: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()