#!/usr/bin/env python3
import sys

# Force a 500 error for testing
print("Status: 500 Internal Server Error")
print("Content-Type: text/html")
print()
print("""
<!DOCTYPE html>
<html>
<head>
    <title>500 Test Error</title>
    <link rel="stylesheet" href="/css/style.css">
</head>
<body>
    <div class="error-container">
        <h1>500</h1>
        <h2>Intentional Error</h2>
        <p>This is a test of the 500 error page.</p>
        <div class="error-details">
            <pre>Test error message
Python version: {}</pre>
        </div>
        <a href="/" class="btn">Return Home</a>
    </div>
</body>
</html>
""".format(sys.version))
