#include <WiFi.h>
#include <WebServer.h>

// Replace with your network credentials
const char* ssid = "SpiderBot_Wifi";
const char* password = "spiderbot_pwd";

WebServer server(80);

// Hardware Serial2 for Arduino communication
#define RXD2 16
#define TXD2 17

const char htmlPage[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
    <title>SpiderBot Control</title>
    <style>
        :root {
            --primary: #00d2ff;
            --bg: #111;
            --panel: #222;
            --text: #fff;
            --btn-bg: #333;
            --btn-active: #00d2ff;
        }
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background-color: var(--bg);
            color: var(--text);
            margin: 0;
            padding: 0;
            display: flex;
            flex-direction: column;
            align-items: center;
            height: 100vh;
            overflow: hidden;
            touch-action: manipulation;
        }
        .header {
            width: 100%;
            padding: 15px 0;
            text-align: center;
            background: linear-gradient(90deg, #1a1a1a, #0a0a0a);
            border-bottom: 2px solid var(--primary);
            box-shadow: 0 0 15px rgba(0, 210, 255, 0.3);
            margin-bottom: 20px;
        }
        .header h1 {
            margin: 0;
            font-size: 24px;
            letter-spacing: 2px;
            text-transform: uppercase;
        }
        .status-panel {
            display: flex;
            justify-content: space-around;
            width: 90%;
            max-width: 400px;
            background: var(--panel);
            padding: 10px;
            border-radius: 10px;
            margin-bottom: 20px;
            box-shadow: inset 0 0 10px #000;
        }
        .status-item {
            font-size: 14px;
            display: flex;
            flex-direction: column;
            align-items: center;
        }
        .status-val {
            color: var(--primary);
            font-weight: bold;
            margin-top: 5px;
        }
        .grid-container {
            display: grid;
            grid-template-columns: repeat(3, 1fr);
            gap: 15px;
            margin-bottom: 30px;
        }
        .btn {
            background-color: var(--btn-bg);
            border: 2px solid #555;
            color: white;
            border-radius: 12px;
            padding: 20px;
            font-size: 24px;
            cursor: pointer;
            transition: all 0.2s ease;
            user-select: none;
            -webkit-user-select: none;
            display: flex;
            justify-content: center;
            align-items: center;
            text-transform: uppercase;
            font-weight: bold;
        }
        .btn:active, .btn.active {
            background-color: var(--btn-active);
            border-color: var(--primary);
            color: black;
            box-shadow: 0 0 20px var(--primary);
            transform: scale(0.95);
        }
        .btn-up { grid-column: 2; grid-row: 1; border-top-left-radius: 20px; border-top-right-radius: 20px; }
        .btn-down { grid-column: 2; grid-row: 3; border-bottom-left-radius: 20px; border-bottom-right-radius: 20px; }
        .btn-left { grid-column: 1; grid-row: 2; border-top-left-radius: 20px; border-bottom-left-radius: 20px; }
        .btn-right { grid-column: 3; grid-row: 2; border-top-right-radius: 20px; border-bottom-right-radius: 20px; }
        .btn-stop { grid-column: 2; grid-row: 2; background-color: #522; border-color: #f55; }
        .btn-stop:active, .btn-stop.active { background-color: #f55; box-shadow: 0 0 20px #f55; }
        
        .action-btns {
            display: flex;
            gap: 15px;
            flex-wrap: wrap;
            justify-content: center;
            width: 90%;
            max-width: 400px;
        }
        .action-btn {
            flex: 1;
            min-width: 80px;
            padding: 12px;
            background: #2a2a2a;
            border: 1px solid #444;
            border-radius: 8px;
            color: #ccc;
            font-size: 14px;
            text-transform: uppercase;
            font-weight: bold;
        }
        .action-btn:active {
            background: var(--primary);
            color: #000;
        }
    </style>
</head>
<body>
    <div class="header">
        <h1>SpiderBot OS v2.0</h1>
    </div>
    
    <div class="status-panel">
        <div class="status-item">
            <span>LINK</span>
            <span class="status-val">ACTIVE</span>
        </div>
        <div class="status-item">
            <span>MODE</span>
            <span class="status-val" id="mode-val">IDLE</span>
        </div>
        <div class="status-item">
            <span>BATT</span>
            <span class="status-val">98%</span>
        </div>
    </div>

    <div class="grid-container">
        <div class="btn btn-up" onmousedown="sendCommand('F')" onmouseup="sendCommand('S')" ontouchstart="sendCommand('F'); event.preventDefault()" ontouchend="sendCommand('S'); event.preventDefault()">▲</div>
        <div class="btn btn-left" onmousedown="sendCommand('L')" onmouseup="sendCommand('S')" ontouchstart="sendCommand('L'); event.preventDefault()" ontouchend="sendCommand('S'); event.preventDefault()">◀</div>
        <div class="btn btn-stop" onclick="sendCommand('S')" ontouchstart="sendCommand('S'); event.preventDefault()">■</div>
        <div class="btn btn-right" onmousedown="sendCommand('R')" onmouseup="sendCommand('S')" ontouchstart="sendCommand('R'); event.preventDefault()" ontouchend="sendCommand('S'); event.preventDefault()">▶</div>
        <div class="btn btn-down" onmousedown="sendCommand('B')" onmouseup="sendCommand('S')" ontouchstart="sendCommand('B'); event.preventDefault()" ontouchend="sendCommand('S'); event.preventDefault()">▼</div>
    </div>

    <div class="action-btns">
        <button class="action-btn" onclick="sendCommand('U')">STAND</button>
        <button class="action-btn" onclick="sendCommand('X')">SIT</button>
        <button class="action-btn" onclick="sendCommand('D')">DANCE</button>
    </div>

    <script>
        function sendCommand(cmd) {
            fetch('/cmd?c=' + cmd);
            const modes = {'F':'FORWARD', 'B':'BACKWARD', 'L':'LEFT', 'R':'RIGHT', 'S':'IDLE', 'U':'STAND', 'X':'SIT', 'D':'DANCE'};
            if(modes[cmd]) document.getElementById('mode-val').innerText = modes[cmd];
        }

        // Keyboard support
        const keyMap = {'ArrowUp':'F', 'ArrowDown':'B', 'ArrowLeft':'L', 'ArrowRight':'R', 'Space':'S'};
        let activeKey = null;

        window.addEventListener('keydown', (e) => {
            const code = e.code === 'Space' ? 'Space' : e.key;
            if (keyMap[code] && activeKey !== code) {
                activeKey = code;
                sendCommand(keyMap[code]);
                // Visual feedback could be added here
            }
        });

        window.addEventListener('keyup', (e) => {
            const code = e.code === 'Space' ? 'Space' : e.key;
            if (keyMap[code] && activeKey === code) {
                activeKey = null;
                // Only send stop if it's a movement key
                if (code !== 'Space') {
                    sendCommand('S');
                }
            }
        });
    </script>
</body>
</html>
)=====";

void handleRoot() {
  server.send(200, "text/html", htmlPage);
}

void handleCommand() {
  if (server.hasArg("c")) {
    String cmd = server.arg("c");
    Serial2.print(cmd); // Send to Arduino Nano via UART
    Serial.println("Sent: " + cmd);
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Bad Request");
  }
}

void setup() {
  Serial.begin(115200); // For debugging
  
  // Set up Serial2 for Arduino Nano (RX=16, TX=17)
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);

  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(ssid, password);
  
  Serial.println("AP IP address: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.on("/cmd", handleCommand);
  server.begin();
  
  Serial.println("Web server started");
}

void loop() {
  server.handleClient();
}
