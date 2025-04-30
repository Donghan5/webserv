#!/usr/bin/perl
use strict;
use warnings;
use POSIX qw(strftime);
use FindBin qw($Bin);
use File::Basename;

my $timestamp = strftime("%Y-%m-%d %H:%M:%S", localtime);
my $script_path = $ENV{'SCRIPT_FILENAME'} || 'Not available';
my $project_root = dirname(dirname($Bin));
my $request_method = $ENV{'REQUEST_METHOD'} || 'Not available';

print "Content-Type: text/html\r\n\r\n";
print <<HTML;
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Perl CGI Test</title>
    <style>
        body {
            font-family: 'Arial', sans-serif;
            background-color: #f9f9f9;
            margin: 0;
            padding: 20px;
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
        }
        .card {
            background-color: white;
            border-radius: 8px;
            box-shadow: 0 4px 8px rgba(0,0,0,0.1);
            padding: 30px;
            max-width: 500px;
            width: 100%;
            text-align: center;
            border-top: 5px solid #004065;
        }
        h1 {
            color: #004065;
            margin-top: 0;
        }
        .icon {
            font-size: 48px;
            margin-bottom: 20px;
        }
        .divider {
            height: 1px;
            background-color: #eee;
            margin: 20px 0;
        }
        .timestamp {
            color: #999;
            font-size: 14px;
            margin-top: 20px;
        }
        .info {
            background-color: #e8f5e9;
            padding: 10px;
            border-radius: 4px;
            margin-top: 15px;
            text-align: left;
            font-size: 0.9em;
        }
    </style>
</head>
<body>
    <div class="card">
        <div class="icon">🐪</div>
        <h1>Perl CGI Test</h1>
        <div class="divider"></div>
        <p>Hello from Perl CGI!</p>
        <p class="timestamp">Response generated: $timestamp</p>
        
        <div class="info">
            <p><strong>Script Path:</strong> $script_path</p>
            <p><strong>Project Root:</strong> $project_root</p>
            <p><strong>Request Method:</strong> $request_method</p>
        </div>
    </div>
</body>
</html>
HTML