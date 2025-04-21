#!/usr/bin/python3

import cgi, os, sys
import cgitb
import time
import mimetypes

cgitb.enable(display=0, logdir='./www/cgi-bin/tmp')

debug_info = "\n--- CGI env ---\n"
for key, value in os.environ.items():
	debug_info += f"{key}: {value}\n"

with open('./www/cgi-bin/tmp/debug.log', 'w') as f:
	f.write(debug_info)

def get_file_size_str(size_bytes):
    """Convert file size in bytes to human-readable format."""
    if size_bytes < 1024:
        return f"{size_bytes} bytes"
    elif size_bytes < 1024 * 1024:
        return f"{size_bytes/1024:.2f} KB"
    elif size_bytes < 1024 * 1024 * 1024:
        return f"{size_bytes/(1024*1024):.2f} MB"
    else:
        return f"{size_bytes/(1024*1024*1024):.2f} GB"


html_start = """<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>File Upload Result</title>
    <style>
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            line-height: 1.6;
            color: #333;
            max-width: 800px;
            margin: 0 auto;
            padding: 20px;
            background-color: #f9f9f9;
        }
        .result-card {
            background-color: white;
            border-radius: 8px;
            box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
            padding: 20px;
            margin-top: 20px;
        }
        .success {
            border-left: 5px solid #4CAF50;
        }
        .error {
            border-left: 5px solid #f44336;
        }
        h1 {
            color: #2c3e50;
            margin-top: 0;
        }
        .file-info {
            background-color: #f5f5f5;
            border-radius: 4px;
            padding: 15px;
            margin: 15px 0;
        }
        .file-preview {
            margin: 20px 0;
            text-align: center;
        }
        .file-preview img {
            max-width: 100%;
            max-height: 300px;
            border-radius: 4px;
            box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1);
        }
        .btn {
            display: inline-block;
            background-color: #3498db;
            color: white;
            padding: 10px 15px;
            text-decoration: none;
            border-radius: 4px;
            font-weight: 500;
            margin-top: 15px;
            transition: background-color 0.3s;
        }
        .btn:hover {
            background-color: #2980b9;
        }
        table {
            width: 100%;
            border-collapse: collapse;
            margin: 15px 0;
        }
        th, td {
            padding: 10px;
            text-align: left;
            border-bottom: 1px solid #ddd;
        }
        th {
            background-color: #f2f2f2;
        }
    </style>
</head>
<body>
"""

html_end = """
    <a href="/" class="btn">Back to Home</a>
</body>
</html>
"""

try:
    upload_dir = os.path.join('www', 'cgi-bin', 'tmp')
    if not os.path.exists(upload_dir):
        os.makedirs(upload_dir)

    form = cgi.FieldStorage()

    if 'filename' not in form:
        print(html_start)
        print('<div class="result-card error">')
        print('<h1>⚠️ Upload Error</h1>')
        print('<p>No file was provided. Please select a file to upload.</p>')
        print('</div>')
        print(html_end)
    else:
        fileitem = form['filename']

        if fileitem.filename:
            filename = os.path.basename(fileitem.filename)
            file_content = fileitem.file.read()
            file_size = len(file_content)
            file_type = mimetypes.guess_type(filename)[0] or "Unknown"

            filepath = os.path.join(upload_dir, filename)
            with open(filepath, 'wb') as f:
                f.write(file_content)

            print(html_start)
            print('<div class="result-card success">')
            print(f'<h1>✅ File Upload Successful</h1>')
            print(f'<p>Your file has been uploaded successfully.</p>')

            print('<div class="file-info">')
            print('<table>')
            print(f'<tr><th>File Name</th><td>{filename}</td></tr>')
            print(f'<tr><th>File Size</th><td>{get_file_size_str(file_size)}</td></tr>')
            print(f'<tr><th>File Type</th><td>{file_type}</td></tr>')
            print(f'<tr><th>Upload Time</th><td>{time.strftime("%Y-%m-%d %H:%M:%S")}</td></tr>')
            print(f'<tr><th>Saved Path</th><td>{filepath}</td></tr>')
            print('</table>')
            print('</div>')

            if file_type and file_type.startswith('image/'):
                print('<div class="file-preview">')
                print('<h3>File Preview</h3>')
                print(f'<img src="/cgi-bin/tmp/{filename}" alt="{filename}">')
                print('</div>')

            print('</div>')
            print(html_end)
        else:
            print(html_start)
            print('<div class="result-card error">')
            print('<h1>⚠️ Upload Error</h1>')
            print('<p>The file has an invalid filename. Please try again with a valid file.</p>')
            print('</div>')
            print(html_end)

except Exception as e:
    print(html_start)
    print('<div class="result-card error">')
    print('<h1>❌ Upload Failed</h1>')
    print(f'<p>An error occurred during the upload process: {str(e)}</p>')
    print('<p>Please try again or contact the administrator if the problem persists.</p>')
    print('</div>')
    print(html_end)
