#include <WiFi.h>
#include <WebServer.h>

// Replace with your Network Credentials
const char* ssid = "Spider_Robot_WiFi";
const char* password = "spiderpassword"; // or set to "" for open network

// Set web server port number to 80
WebServer server(80);

// HTML & CSS & JS for Mobile UI
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>Spider Robot Controller</title>
  <meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1, user-scalable=no">
  <style>
    body { font-family: Arial; text-align: center; background-color: #2c3e50; color: white; margin:0; padding:10px; user-select:none; }
    h2 { margin-bottom: 20px; }
    .grid { display: grid; grid-template-columns: repeat(3, 1fr); gap: 10px; max-width: 300px; margin: 0 auto 30px auto; }
    .btn { background-color: #3498db; color: white; border: none; padding: 20px; font-size: 24px; border-radius: 10px; box-shadow: 0 4px #2980b9; user-select: none; -webkit-tap-highlight-color: transparent; }
    .btn:active { background-color: #1abc9c; box-shadow: 0 2px #16a085; transform: translateY(2px); }
    .action-btn { background-color: #e67e22; box-shadow: 0 4px #d35400; margin: 5px; width: 80px; }
    .action-btn:active { background-color: #d35400; box-shadow: 0 2px #a04000; }
    .slider-container { margin: 20px auto; max-width: 300px; }
    input[type=range] { width: 100%; }
  </style>
</head>
<body>
  <h2>Spider Control</h2>
  
  <div class="grid">
    <div></div>
    <button class="btn" id="btn-F">&#8593;</button>
    <div></div>
    <button class="btn" id="btn-L">&#8592;</button>
    <button class="btn" style="background:#e74c3c; box-shadow: 0 4px #c0392b;" onclick="sendCommand('S')">&#9632;</button>
    <button class="btn" id="btn-R">&#8594;</button>
    <div></div>
    <button class="btn" id="btn-B">&#8595;</button>
    <div></div>
  </div>

  <div>
    <button class="btn action-btn" onclick="sendCommand('W')">Wave</button>
    <button class="btn action-btn" onclick="sendCommand('U')">Shake</button>
    <button class="btn action-btn" onclick="sendCommand('V')">Dance</button>
    <button class="btn action-btn" onclick="sendCommand('X')">Sit</button>
  </div>

  <div class="slider-container">
    <p>Speed</p>
    <input type="range" min="1" max="9" value="4" id="speedSlider" onchange="sendSpeed()">
  </div>

  <script>
    var interval;
    
    function sendCommand(cmd) {
      fetch('/cmd?c=' + cmd);
    }
    
    function sendSpeed() {
      var val = document.getElementById("speedSlider").value;
      sendCommand(val); // '1'-'9'
    }

    function startHold(cmd) {
      sendCommand(cmd);
      interval = setInterval(function(){ sendCommand(cmd); }, 600); 
      // 600ms corresponds roughly to the time 1 step cycle takes on Nano
    }

    function stopHold() {
      clearInterval(interval);
      sendCommand('S'); // Stop movement when released
    }

    function addHoldEvents(id, cmd) {
      var el = document.getElementById(id);
      el.addEventListener('mousedown', function(e){ e.preventDefault(); startHold(cmd); });
      el.addEventListener('mouseup', function(e){ e.preventDefault(); stopHold(); });
      el.addEventListener('mouseleave', function(e){ e.preventDefault(); stopHold(); });
      
      el.addEventListener('touchstart', function(e){ e.preventDefault(); startHold(cmd); });
      el.addEventListener('touchend', function(e){ e.preventDefault(); stopHold(); });
    }

    addHoldEvents('btn-F', 'F');
    addHoldEvents('btn-B', 'B');
    addHoldEvents('btn-L', 'L');
    addHoldEvents('btn-R', 'R');
  </script>
</body>
</html>
)rawliteral";

void setup() {
  Serial.begin(9600); // Debugging
  Serial2.begin(9600, SERIAL_8N1, 16, 17); // Communication to Nano (RX = GPIO16, TX = GPIO17)
  
  // Create Access Point
  WiFi.softAP(ssid, password);
  
  server.on("/", []() {
    server.send(200, "text/html", index_html);
  });

  server.on("/cmd", []() {
    if (server.hasArg("c")) {
      String cmd = server.arg("c");
      if(cmd.length() > 0) {
        Serial2.write(cmd[0]); // Send via UART to Arduino Nano
        Serial.print("Sent URL command: ");
        Serial.println(cmd[0]);
      }
    }
    server.send(200, "text/plain", "OK");
  });

  server.begin();
  Serial.println("HTTP Web server started");
}

void loop() {
  server.handleClient();
}