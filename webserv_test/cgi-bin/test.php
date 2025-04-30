#!/usr/bin/php
<?php
echo "Content-Type: text/html\r\n\r\n";

// Get script paths
$script_path = $_SERVER['SCRIPT_FILENAME'] ?? 'Not available';
$project_root = dirname(dirname($script_path));
?>
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>PHP CGI Test</title>
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
            border-top: 5px solid #777BB3;
        }
        h1 {
            color: #777BB3;
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
        <div class="icon">🐘</div>
        <h1>PHP CGI Test</h1>
        <div class="divider"></div>
        <p>Hello from PHP CGI!</p>
        <p class="timestamp">Response generated: <?php echo date('Y-m-d H:i:s'); ?></p>
        
        <div class="info">
            <p><strong>Script Path:</strong> <?php echo htmlspecialchars($script_path); ?></p>
            <p><strong>Project Root:</strong> <?php echo htmlspecialchars($project_root); ?></p>
            <p><strong>Request Method:</strong> <?php echo htmlspecialchars($_SERVER['REQUEST_METHOD'] ?? 'Not available'); ?></p>
        </div>
    </div>
</body>
</html>