#!/bin/bash

# HTTP 헤더 출력
echo -e "Content-Type: text/html\r\n"

# 빈 줄(헤더와 본문 구분)
echo "<html><body>"
echo "<h1>Bash CGI Test</h1>"
echo "<p>Hello from Bash CGI!</p>"
echo "</body></html>"
