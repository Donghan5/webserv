#!/usr/bin/env python3
import os
import time
import http.cookies
import random
import string
import sys

# Function to generate a random session ID
def generate_session_id(length=32):
    return ''.join(random.choice(string.ascii_letters + string.digits) for _ in range(length))

# Initialize cookie
cookie = http.cookies.SimpleCookie()
http_cookie = os.environ.get('HTTP_COOKIE', '')

# Load existing cookies
if http_cookie:
    cookie.load(http_cookie)

# Check for session cookie
session_id = None
visit_count = 1
if 'session_id' in cookie:
    session_id = cookie['session_id'].value
    # Simulate session validation with visit count
    if 'visit_count' in cookie:
        try:
            visit_count = int(cookie['visit_count'].value) + 1
        except ValueError:
            visit_count = 1
else:
    # Create new session
    session_id = generate_session_id()

# Set/update cookies
cookie['session_id'] = session_id
cookie['session_id']['path'] = '/'
cookie['session_id']['max-age'] = 3600  # 1 hour
cookie['session_id']['HttpOnly'] = True  # Add HttpOnly flag for security
cookie['session_id']['SameSite'] = 'Lax'

cookie['visit_count'] = str(visit_count)
cookie['visit_count']['path'] = '/'
cookie['visit_count']['max-age'] = 3600  # 1 hour
cookie['visit_count']['SameSite'] = 'Lax'

# Output headers with explicit \r\n line endings
print("Content-Type: text/html\r")
# Print each cookie on its own line with proper format
for morsel in cookie.values():
    print(f"Set-Cookie: {morsel.OutputString()}\r")
print("\r")  # Empty line to separate headers from body

# HTML content follows
print(f"""<!DOCTYPE html>
<html>
<head>
    <title>Session Test</title>
    <style>
        body {{ 
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            margin: 0;
            padding: 20px;
            background-color: #f5f5f5;
            line-height: 1.6;
        }}
        .container {{
            max-width: 800px;
            margin: 0 auto;
            background-color: white;
            padding: 20px;
            border-radius: 8px;
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
        }}
        h1 {{ 
            color: #333;
            border-bottom: 2px solid #eee;
            padding-bottom: 10px;
        }}
        .session-info {{ 
            background-color: #f0f8ff; 
            padding: 20px; 
            border-radius: 5px; 
            margin: 20px 0;
            border-left: 5px solid #2980b9;
        }}
        .counter {{ 
            font-size: 24px; 
            font-weight: bold; 
            margin: 20px 0; 
            color: #2980b9;
        }}
        .note {{
            font-style: italic;
            color: #666;
            margin-top: 15px;
        }}
        .tech-details {{
            background-color: #f9f9f9;
            padding: 15px;
            border-radius: 4px;
            margin-top: 20px;
            font-family: monospace;
            font-size: 14px;
        }}
        .actions {{
            margin-top: 20px;
        }}
        .btn {{
            display: inline-block;
            padding: 10px 15px;
            background-color: #3498db;
            color: white;
            text-decoration: none;
            border-radius: 4px;
            margin-right: 10px;
        }}
        .btn:hover {{
            background-color: #2980b9;
        }}
    </style>
</head>
<body>
    <div class="container">
        <h1>Session/Cookie Test</h1>
        <p>This example demonstrates how to manage user sessions with cookies.</p>
        
        <div class="session-info">
            <h2>Your Session Information</h2>
            <p><strong>Session ID:</strong> {session_id}</p>
            <p><strong>Created/Updated:</strong> {time.strftime('%Y-%m-%d %H:%M:%S')}</p>
            
            <div class="counter">
                You have visited this page {visit_count} time{'s' if visit_count > 1 else ''}
            </div>
            
            <p class="note">Refresh the page to see the counter increase. The session will expire after 1 hour.</p>
            
            <div class="tech-details">
                <p><strong>Cookie Details:</strong></p>
                <pre>{cookie.output().replace('<', '&lt;').replace('>', '&gt;')}</pre>
                <p><strong>Environment:</strong></p>
                <pre>HTTP_COOKIE: {http_cookie}</pre>
            </div>
        </div>
        
        <div class="actions">
            <a href="/cgi-bin/session.py" class="btn">Refresh Page</a>
            <a href="/" class="btn">Back to Home</a>
        </div>
    </div>
</body>
</html>""")