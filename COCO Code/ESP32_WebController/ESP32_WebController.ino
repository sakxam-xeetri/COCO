#include <WiFi.h>
#include <WebServer.h>

// Replace with your network credentials or create an access point
const char* ssid = "ESP32_Robot_Controller";
const char* password = "password123";

WebServer server(80);

const char htmlPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Robot Controller</title>
  <style>
    body { font-family: Arial, sans-serif; text-align: center; margin-top: 50px; background-color: #222; color: #fff;}
    .btn { background-color: #4CAF50; border: none; color: white; padding: 20px 40px; text-align: center; font-size: 24px; margin: 10px; cursor: pointer; border-radius: 10px; user-select: none; }
    .btn:active { background-color: #3e8e41; }
    .row { display: flex; justify-content: center; }
    .controls { display: inline-block; margin-top: 20px; }
  </style>
</head>
<body>
  <h1>Robot Controller</h1>
  <div class="controls">
    <div class="row">
      <button class="btn" onmousedown="sendCmd('F')" onmouseup="sendCmd('S')" ontouchstart="sendCmd('F')" ontouchend="sendCmd('S')">Forward</button>
    </div>
    <div class="row">
      <button class="btn" onmousedown="sendCmd('L')" onmouseup="sendCmd('S')" ontouchstart="sendCmd('L')" ontouchend="sendCmd('S')">Left</button>
      <button class="btn" onmousedown="sendCmd('S')" onmouseup="sendCmd('S')" ontouchstart="sendCmd('S')" ontouchend="sendCmd('S')">Stop</button>
      <button class="btn" onmousedown="sendCmd('R')" onmouseup="sendCmd('S')" ontouchstart="sendCmd('R')" ontouchend="sendCmd('S')">Right</button>
    </div>
    <div class="row">
      <button class="btn" onmousedown="sendCmd('B')" onmouseup="sendCmd('S')" ontouchstart="sendCmd('B')" ontouchend="sendCmd('S')">Back</button>
    </div>
    <br>
    <div class="row">
      <button class="btn" onclick="sendCmd('U')">Stand</button>
      <button class="btn" onclick="sendCmd('X')">Sit</button>
      <button class="btn" onclick="sendCmd('D')">Dance</button>
    </div>
  </div>
  <script>
    function sendCmd(cmd) {
      fetch('/command?c=' + cmd);
    }
  </script>
</body>
</html>
)rawliteral";

void handleRoot() {
  server.send(200, "text/html", htmlPage);
}

void handleCommand() {
  if (server.hasArg("c")) {
    String cmd = server.arg("c");
    if (cmd.length() > 0) {
      char c = cmd.charAt(0);
      Serial.print(c); // Send the command to Arduino Nano over TX
    }
  }
  server.send(200, "text/plain", "OK");
}

void setup() {
  // Use 115200 for faster communication with Nano
  Serial.begin(115200);

  // Setup ESP32 as an Access Point
  WiFi.softAP(ssid, password);
  
  server.on("/", handleRoot);
  server.on("/command", handleCommand);
  server.begin();
}

void loop() {
  server.handleClient();
}
