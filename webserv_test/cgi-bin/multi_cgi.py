#!/usr/bin/env python3
import subprocess
import os
import time
import sys

# Add the project directories to the path if needed
PROJECT_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.append(PROJECT_ROOT)

print("Content-Type: text/html\r\n\r\n")

print("""<!DOCTYPE html>
<html>
<head>
    <title>Multiple CGI Test</title>
    <style>
        body { 
            font-family: Arial, sans-serif; 
            margin: 20px; 
            background-color: #f5f5f5;
            line-height: 1.6;
        }
        .container {
            max-width: 900px;
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
        .cgi-result { 
            border: 1px solid #ddd; 
            padding: 15px; 
            margin: 15px 0; 
            border-radius: 5px; 
        }
        .cgi-header { 
            background-color: #f5f5f5; 
            padding: 10px; 
            margin-bottom: 10px; 
            border-radius: 4px;
            font-weight: bold;
        }
        .success { color: #4CAF50; }
        .error { color: #f44336; }
        .execution-time {
            font-size: 0.9em;
            color: #777;
        }
        iframe {
            width: 100%;
            border: none;
            min-height: 100px;
            background-color: #f9f9f9;
            border-radius: 4px;
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
        <h1>Multiple CGI Test</h1>
        <p>This script demonstrates calling multiple CGI scripts from a single script.</p>
""")

# Array of CGI scripts to call - use relative paths from this script
cgi_scripts = [
    "./test.py",
    "./env.py",
    "./test.php",
    "./test.pl",
    "./test.sh"
]

for script in cgi_scripts:
    script_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), script)
    
    print(f"<div class='cgi-result'>")
    print(f"<div class='cgi-header'>Executing: {script}</div>")
    
    try:
        start_time = time.time()
        
        # Prepare environment
        env = os.environ.copy()
        env["QUERY_STRING"] = "param1=value1&param2=value2"
        
        # Execute the script
        process = subprocess.Popen([script_path], env=env, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        stdout, stderr = process.communicate(timeout=10)
        
        elapsed_time = time.time() - start_time
        
        # Handle output
        try:
            output = stdout.decode('utf-8')
            
            # Extract body content (after headers)
            headers_end = output.find('\r\n\r\n')
            if headers_end != -1:
                body = output[headers_end + 4:]
            else:
                body = output
            
            # Display execution status
            status_class = "success" if process.returncode == 0 else "error"
            status_text = "Success" if process.returncode == 0 else "Failed"
            print(f"<p><strong>Status:</strong> <span class='{status_class}'>{status_text}</span></p>")
            print(f"<p class='execution-time'><strong>Execution Time:</strong> {elapsed_time:.4f} seconds</p>")
            
            # Display errors if any
            if stderr:
                error_text = stderr.decode('utf-8')
                print(f"<p><strong>Errors:</strong> <pre>{error_text}</pre></p>")
            
            # Create a unique iframe ID for each script
            iframe_id = f"iframe_{os.path.basename(script).replace('.', '_')}"
            
            # Display the output in an iframe
            print(f"<iframe id='{iframe_id}' srcdoc='{body.replace('\"', '&quot;')}'></iframe>")
            
            # Add script to adjust iframe height
            print(f"""
            <script>
                document.getElementById('{iframe_id}').onload = function() {{
                    try {{
                        this.style.height = (this.contentWindow.document.body.scrollHeight + 20) + 'px';
                    }} catch(e) {{
                        console.error("Unable to adjust iframe height:", e);
                    }}
                }};
            </script>
            """)
            
        except Exception as e:
            print(f"<p><strong>Error processing output:</strong> {str(e)}</p>")
    
    except subprocess.TimeoutExpired:
        print(f"<p><strong>Error:</strong> <span class='error'>Script execution timed out after 10 seconds</span></p>")
    
    except Exception as e:
        print(f"<p><strong>Error executing {script}:</strong> <span class='error'>{str(e)}</span></p>")
    
    print("</div>")

print("""
        <a href="/" class="back-link">Back to Home</a>
    </div>
</body>
</html>
""")