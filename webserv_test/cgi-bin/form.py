#!/usr/bin/env python3
import cgi
import html
import os
import sys

# Add the project directories to the path if needed
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

print("Content-Type: text/html\r\n\r\n")

# Get the request method
request_method = os.environ.get("REQUEST_METHOD", "GET")

form = cgi.FieldStorage()
name = html.escape(form.getvalue("name", ""))
email = html.escape(form.getvalue("email", ""))
message = html.escape(form.getvalue("message", ""))

print("""<!DOCTYPE html>
<html>
<head>
    <title>Form Processing Example</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 20px;
            background-color: #f5f5f5;
            line-height: 1.6;
        }
        .container {
            max-width: 800px;
            margin: 0 auto;
            background-color: white;
            padding: 20px;
            border-radius: 8px;
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
        }
        h1 {
            color: #333;
            border-bottom: 2px solid #eee;
            padding-bottom: 10px;
        }
        form {
            background-color: #f9f9f9;
            padding: 20px;
            border-radius: 5px;
            margin-top: 20px;
        }
        label {
            display: block;
            margin-bottom: 5px;
            font-weight: bold;
        }
        input, textarea {
            margin-bottom: 15px;
            padding: 10px;
            width: 100%;
            border: 1px solid #ddd;
            border-radius: 4px;
            box-sizing: border-box;
        }
        button {
            padding: 12px 20px;
            background-color: #4CAF50;
            color: white;
            border: none;
            border-radius: 4px;
            cursor: pointer;
            font-size: 16px;
        }
        button:hover {
            background-color: #45a049;
        }
        .result {
            margin-top: 20px;
            padding: 15px;
            background-color: #e8f5e9;
            border-radius: 5px;
            border-left: 5px solid #4CAF50;
        }
        .back-link {
            display: inline-block;
            margin-top: 20px;
            color: #2980b9;
            text-decoration: none;
        }
        .back-link:hover {
            text-decoration: underline;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>Form Processing Example</h1>
""")

# Only show the "Form Data Received" section if this is a POST request
if request_method == "POST" and (name or email or message):
    print(f"""
    <div class="result">
        <h2>Form Data Received:</h2>
        <p><strong>Name:</strong> {name}</p>
        <p><strong>Email:</strong> {email}</p>
        <p><strong>Message:</strong> {message}</p>
    </div>
    """)

print(f"""
        <form method="post" action="/cgi-bin/form.py">
            <label for="name">Name:</label>
            <input type="text" id="name" name="name" value="{name}" required>
            
            <label for="email">Email:</label>
            <input type="email" id="email" name="email" value="{email}" required>
            
            <label for="message">Message:</label>
            <textarea id="message" name="message" rows="4" required>{message}</textarea>
            
            <button type="submit">Submit Form</button>
        </form>
        
        <a href="/" class="back-link">Back to Home</a>
    </div>
</body>
</html>
""")