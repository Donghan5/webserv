#!/usr/bin/python3

import cgi, os, sys
import cgitb
import time
import mimetypes

cgitb.enable(display=0, logdir='./www/cgi-bin/tmp')

debug_info = "\n--- CGI env ---\n"
for key, value in os.environ.items():
	debug_info += f"{key}: {value}\n"

# 디버그용 로그 디렉토리 설정
debug_dir = './www/cgi-bin/tmp'
if not os.path.exists(debug_dir):
    os.makedirs(debug_dir)

debug_info += "\n--- FORM DATA ---\n"
form = cgi.FieldStorage()
debug_info += f"Form keys: {list(form.keys())}\n"
debug_info += f"Form contains 'filename': {'filename' in form}\n"
if 'filename' in form:
    debug_info += f"Filename value type: {type(form['filename'])}\n"
    debug_info += f"Filename has filename attr: {hasattr(form['filename'], 'filename')}\n"
    if hasattr(form['filename'], 'filename'):
        debug_info += f"Filename.filename: {form['filename'].filename}\n"

# Add to your CGI script for more detailed debugging
debug_info += "\n--- POST DATA CHECK ---\n"
debug_info += f"CONTENT_LENGTH: {os.environ.get('CONTENT_LENGTH', 'Not set')}\n"
debug_info += f"CONTENT_TYPE: {os.environ.get('CONTENT_TYPE', 'Not set')}\n"

try:
    # Try to read start of raw POST data
    raw_data = sys.stdin.buffer.read(100)
    debug_info += f"stdin data first 100 bytes length: {len(raw_data)}\n"
    debug_info += f"stdin data first 100 bytes (hex): {raw_data.hex()[:200]}\n"

    # Check boundary
    content_type = os.environ.get('CONTENT_TYPE', '')
    if 'boundary=' in content_type:
        boundary = content_type.split('boundary=')[1].strip()
        debug_info += f"Detected boundary: {boundary}\n"
    else:
        debug_info += "No boundary detected in Content-Type\n"
except Exception as e:
    debug_info += f"Error reading stdin: {str(e)}\n"

with open(os.path.join(debug_dir, 'debug.log'), 'w') as f:
    f.write(debug_info)

with open(os.path.join(debug_dir, 'debug.log'), 'w') as f:
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

def log_progress(message, filename="upload_progress.log"):
    """Log progress information with timestamp."""
    progress_log_path = os.path.join(debug_dir, filename)
    with open(progress_log_path, 'a') as f:
        timestamp = time.strftime("%Y-%m-%d %H:%M:%S")
        f.write(f"[{timestamp}] {message}\n")


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
        .debug-info {
            margin-top: 20px;
            border: 1px solid #ddd;
            border-radius: 4px;
            padding: 10px;
        }
        .debug-info h3 {
            margin-top: 0;
            color: #666;
            font-size: 16px;
        }
        details summary {
            cursor: pointer;
            color: #3498db;
            font-weight: 500;
            padding: 5px 0;
        }
        details summary:hover {
            color: #2980b9;
        }
        pre {
            white-space: pre-wrap;
            word-break: break-all;
            font-family: monospace;
            font-size: 12px;
            line-height: 1.4;
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
    upload_dir = './www/cgi-bin/tmp'
    if not os.path.exists(upload_dir):
        os.makedirs(upload_dir)
        log_progress(f"Created upload directory: {upload_dir}")
    else:
        log_progress(f"Upload directory exists: {upload_dir}")

    # 디버그: 업로드 디렉토리 상태 확인
    upload_dir_debug = {
        "directory": upload_dir,
        "exists": os.path.exists(upload_dir),
        "is_writable": os.access(upload_dir, os.W_OK) if os.path.exists(upload_dir) else False,
        "parent_exists": os.path.exists(os.path.dirname(upload_dir)),
        "parent_is_writable": os.access(os.path.dirname(upload_dir), os.W_OK) if os.path.exists(os.path.dirname(upload_dir)) else False
    }

    with open(os.path.join(debug_dir, 'upload_dir_debug.log'), 'w') as f:
        for key, value in upload_dir_debug.items():
            f.write(f"{key}: {value}\n")

    log_progress("Starting to process form data")
    form = cgi.FieldStorage()

    if 'filename' not in form:
        log_progress("Error: No file was provided in the form")
        print(html_start)
        print('<div class="result-card error">')
        print('<h1>⚠️ Upload Error</h1>')
        print('<p>No file was provided. Please select a file to upload.</p>')
        print('</div>')
        print(html_end)
    else:
        log_progress("Form contains filename field")
        fileitem = form['filename']

        if fileitem.filename:
            log_progress(f"Starting to process file: {fileitem.filename}")
            filename = os.path.basename(fileitem.filename)
            log_progress(f"Parsed basename: {filename}")

            # 파일 읽기에 진행률 로깅 추가
            log_progress(f"Starting to read file content")
            file_content = b''
            chunk_size = 8192  # 8KB 청크 크기
            total_size = 0

            # 청크로 파일 읽기
            try:
                while True:
                    chunk = fileitem.file.read(chunk_size)
                    if not chunk:
                        break
                    file_content += chunk
                    total_size += len(chunk)
                    log_progress(f"Read chunk: {len(chunk)} bytes, Total so far: {total_size} bytes")
            except Exception as e:
                log_progress(f"Error during file reading: {str(e)}")
                raise

            file_size = len(file_content)
            log_progress(f"Completed file reading. Total size: {file_size} bytes")
            file_type = mimetypes.guess_type(filename)[0] or "Unknown"
            log_progress(f"File type detected: {file_type}")

            filepath = os.path.join(upload_dir, filename)

            # 디버그: 파일 업로드 시도 정보
            upload_attempt_info = {
                "time": time.strftime("%Y-%m-%d %H:%M:%S"),
                "filename": filename,
                "filepath": filepath,
                "file_size": file_size,
                "file_type": file_type,
                "upload_dir_exists": os.path.exists(upload_dir),
                "upload_dir_writable": os.access(upload_dir, os.W_OK) if os.path.exists(upload_dir) else False
            }

            with open(os.path.join(debug_dir, 'upload_attempt.log'), 'a') as f:
                f.write("\n--- New Upload Attempt ---\n")
                for key, value in upload_attempt_info.items():
                    f.write(f"{key}: {value}\n")

            # 파일 저장 - 청크별로 저장하고 진행 상황 기록
            log_progress(f"Starting to write file to: {filepath}")
            try:
                with open(filepath, 'wb') as f:
                    chunk_size = 8192  # 8KB 청크
                    total_written = 0

                    # 진행 상황 기록을 위해 청크로 나누어 저장
                    for i in range(0, len(file_content), chunk_size):
                        chunk = file_content[i:i+chunk_size]
                        f.write(chunk)
                        total_written += len(chunk)
                        percent = int((total_written / file_size) * 100) if file_size > 0 else 100
                        log_progress(f"Writing progress: {percent}% ({total_written}/{file_size} bytes)")

                log_progress(f"File successfully written to disk: {filepath}")
            except Exception as e:
                log_progress(f"Error during file writing: {str(e)}")
                raise

            # 디버그: 파일 저장 성공 확인
            upload_success_info = {
                "time": time.strftime("%Y-%m-%d %H:%M:%S"),
                "file_saved": os.path.exists(filepath),
                "actual_size": os.path.getsize(filepath) if os.path.exists(filepath) else 0,
                "expected_size": file_size,
                "size_match": (os.path.getsize(filepath) == file_size) if os.path.exists(filepath) else False
            }

            with open(os.path.join(debug_dir, 'upload_success.log'), 'a') as f:
                f.write("\n--- Upload Status ---\n")
                for key, value in upload_success_info.items():
                    f.write(f"{key}: {value}\n")

            print(html_start)
            print('<div class="result-card success">')
            print(f'<h1>✅ File Upload Successful</h1>')
            print(f'<p>Your file has been uploaded successfully.</p>')

            # 디버그 로그 정보 표시
            print('<div class="debug-info">')
            print('<h3>Debug Information</h3>')
            print('<details>')
            print('<summary>Click to view upload progress logs</summary>')
            print('<pre style="background-color: #f5f5f5; padding: 10px; border-radius: 4px; max-height: 200px; overflow-y: auto;">')

            # 진행 로그 표시
            log_path = os.path.join(debug_dir, 'upload_progress.log')
            if os.path.exists(log_path):
                with open(log_path, 'r') as f:
                    log_content = f.read()
                print(log_content.replace('<', '&lt;').replace('>', '&gt;'))
            else:
                print("No progress log available.")

            print('</pre>')
            print('</details>')
            print('</div>')

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
                # 이미지 미리보기 경로도 디버그 정보에 저장
                img_path = f"/cgi-bin/tmp/{filename}"
                with open(os.path.join(debug_dir, 'image_preview.log'), 'a') as f:
                    f.write(f"\n--- Image Preview Info ---\n")
                    f.write(f"Actual file path: {filepath}\n")
                    f.write(f"Preview path: {img_path}\n")
                    f.write(f"Time: {time.strftime('%Y-%m-%d %H:%M:%S')}\n")

                print(f'<img src="/cgi-bin/tmp/{filename}" alt="{filename}">')
                print('</div>')

            print('</div>')
            print(html_end)
        else:
            log_progress("Error: Invalid filename")
            print(html_start)
            print('<div class="result-card error">')
            print('<h1>⚠️ Upload Error</h1>')
            print('<p>The file has an invalid filename. Please try again with a valid file.</p>')
            print('</div>')
            print(html_end)

except Exception as e:
    # 디버그: 오류 정보 저장
    log_progress(f"ERROR: {type(e).__name__}: {str(e)}")
    error_info = {
        "time": time.strftime("%Y-%m-%d %H:%M:%S"),
        "error_type": type(e).__name__,
        "error_message": str(e),
        "upload_dir": upload_dir if 'upload_dir' in locals() else "Not defined",
        "upload_dir_exists": os.path.exists(upload_dir) if 'upload_dir' in locals() else "Unknown",
        "traceback": cgitb.text(sys.exc_info())
    }

    with open(os.path.join(debug_dir, 'error.log'), 'a') as f:
        f.write("\n--- New Error ---\n")
        for key, value in error_info.items():
            if key != "traceback":  # 트레이스백은 마지막에 별도로 저장
                f.write(f"{key}: {value}\n")
        f.write("\n--- Traceback ---\n")
        f.write(error_info["traceback"])

    print(html_start)
    print('<div class="result-card error">')
    print('<h1>❌ Upload Failed</h1>')
    print(f'<p>An error occurred during the upload process: {str(e)}</p>')
    print('<p>Please try again or contact the administrator if the problem persists.</p>')
    print(f'<p>Error details have been logged to: {os.path.join(debug_dir, "error.log")}</p>')

    # 디버그 로그 정보 표시
    print('<div class="debug-info">')
    print('<h3>Debug Progress Logs</h3>')
    print('<details>')
    print('<summary>Click to view upload progress logs</summary>')
    print('<pre style="background-color: #f5f5f5; padding: 10px; border-radius: 4px; max-height: 200px; overflow-y: auto;">')

    # 진행 로그 표시
    log_path = os.path.join(debug_dir, 'upload_progress.log')
    if os.path.exists(log_path):
        with open(log_path, 'r') as f:
            log_content = f.read()
        print(log_content.replace('<', '&lt;').replace('>', '&gt;'))
    else:
        print("No progress log available.")

    print('</pre>')
    print('</details>')
    print('</div>')

    print('</div>')
    print(html_end)
