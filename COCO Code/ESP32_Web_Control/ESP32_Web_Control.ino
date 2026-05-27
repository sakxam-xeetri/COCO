#include <WiFi.h>
#include <WebServer.h>
#include <esp_wifi.h>

// -----------------------------------------------------------------------------
// ESP32 Wi-Fi controller for the Robot Lk spider robot
//
// This sketch does not drive the servos directly. It replaces the Bluetooth
// module with an ESP32 web page and forwards the same serial commands used by
// the original robot sketch:
//   F = forward
//   B = back
//   L = left
//   R = right
//   S = stop
//   P = basic position
//   Q = spider position
//   U = hand shake
//   W = hand wave
//   V = body dance
//   O = LED on
//   X = LED off
//   K = LED blink
// -----------------------------------------------------------------------------

const char* apSsid = "SpiderRobot";
const char* apPassword = "12345678";

// ESP32 UART2 pins. Change these if your wiring uses different pins.
static const int ESP32_RX_PIN = 16;
static const int ESP32_TX_PIN = 17;
static const uint32_t ROBOT_BAUD = 9600;

IPAddress apIP(192, 168, 4, 1);
IPAddress apGateway(192, 168, 4, 1);
IPAddress apSubnet(255, 255, 255, 0);

WebServer server(80);

const char indexHtml[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Spider Robot Control</title>
  <style>
    :root {
      --bg1: #0b1020;
      --bg2: #111a33;
      --panel: rgba(15, 23, 42, 0.88);
      --accent: #22c55e;
      --accent2: #38bdf8;
      --text: #e5eefb;
      --muted: #94a3b8;
      --danger: #fb7185;
    }
    * { box-sizing: border-box; }
    body {
      margin: 0;
      min-height: 100vh;
      font-family: Arial, Helvetica, sans-serif;
      color: var(--text);
      background:
        radial-gradient(circle at top, rgba(56, 189, 248, 0.18), transparent 34%),
        radial-gradient(circle at bottom right, rgba(34, 197, 94, 0.18), transparent 30%),
        linear-gradient(160deg, var(--bg1), var(--bg2));
      display: grid;
      place-items: center;
      padding: 18px;
    }
    .card {
      width: min(480px, 100%);
      background: var(--panel);
      border: 1px solid rgba(148, 163, 184, 0.22);
      border-radius: 22px;
      padding: 20px;
      box-shadow: 0 24px 80px rgba(0, 0, 0, 0.35);
      backdrop-filter: blur(12px);
    }
    h1 {
      margin: 0 0 8px;
      font-size: 1.7rem;
      letter-spacing: 0.02em;
    }
    p {
      margin: 0 0 18px;
      color: var(--muted);
      line-height: 1.5;
    }
    .status {
      display: flex;
      justify-content: space-between;
      gap: 12px;
      align-items: center;
      margin-bottom: 16px;
      padding: 12px 14px;
      border-radius: 14px;
      background: rgba(255, 255, 255, 0.04);
      border: 1px solid rgba(148, 163, 184, 0.14);
      font-size: 0.95rem;
    }
    .grid {
      display: grid;
      grid-template-columns: repeat(3, 1fr);
      gap: 12px;
    }
    button {
      border: 0;
      border-radius: 16px;
      padding: 18px 12px;
      font-size: 1rem;
      font-weight: 700;
      color: #08111f;
      background: linear-gradient(180deg, #e2e8f0, #cbd5e1);
      cursor: pointer;
      transition: transform 0.12s ease, filter 0.12s ease, opacity 0.12s ease;
      min-height: 62px;
    }
    button:active { transform: scale(0.97); filter: brightness(0.96); }
    .forward { background: linear-gradient(180deg, #86efac, #22c55e); }
    .back { background: linear-gradient(180deg, #fda4af, #fb7185); }
    .left, .right { background: linear-gradient(180deg, #bae6fd, #38bdf8); }
    .gesture { background: linear-gradient(180deg, #fde68a, #f59e0b); }
    .basic-pos { background: linear-gradient(180deg, #c4b5fd, #8b5cf6); }
    .spider-pos { background: linear-gradient(180deg, #f9a8d4, #ec4899); }
    .led-on { background: linear-gradient(180deg, #bbf7d0, #4ade80); }
    .led-off { background: linear-gradient(180deg, #e2e8f0, #94a3b8); }
    .led-blink { background: linear-gradient(180deg, #c4b5fd, #8b5cf6); }
    .wide { grid-column: span 3; }
    .hint {
      margin-top: 16px;
      font-size: 0.9rem;
      color: var(--muted);
    }
    .pulse {
      color: var(--accent);
      font-weight: 700;
    }
  </style>
</head>
<body>
  <div class="card">
    <h1>Spider Robot Control</h1>
    <p>Press and hold direction buttons to move. Release to stop.</p>
    <div class="status">
      <div>Last command: <span class="pulse" id="cmd">none</span></div>
      <div id="msg">Ready</div>
    </div>
    <div class="grid">
      <div></div>
      <button class="forward hold" data-cmd="F">Forward</button>
      <div></div>
      <button class="left hold" data-cmd="L">Left</button>
      <button class="back hold" data-cmd="B">Back</button>
      <button class="right hold" data-cmd="R">Right</button>
      <button class="gesture wide" onclick="sendCmd('U')">Hand Shake</button>
      <button class="gesture wide" onclick="sendCmd('W')">Hand Wave</button>
      <button class="gesture wide" onclick="sendCmd('V')">Body Dance</button>
      <button class="basic-pos wide" onclick="sendCmd('P')">Basic Position</button>
      <button class="spider-pos wide" onclick="sendCmd('Q')">Spider Position</button>
      <button class="led-on wide" onclick="sendCmd('O')">LED On</button>
      <button class="led-off wide" onclick="sendCmd('X')">LED Off</button>
      <button class="led-blink wide" onclick="sendCmd('K')">LED Blink</button>
    </div>
    <div class="hint">Wi-Fi network: <span class="pulse">SpiderRobot</span> | Password: <span class="pulse">12345678</span></div>
  </div>
  <script>
    const HOLD_SEND_INTERVAL_MS = 120;
    let activeMotionCmd = '';
    let holdTimer = null;

    async function sendCmd(cmd, fireAndForget = false) {
      const msg = document.getElementById('msg');
      const last = document.getElementById('cmd');
      try {
        if (!fireAndForget) {
          msg.textContent = 'Sending ' + cmd + '...';
        }

        const res = await fetch('/cmd?go=' + encodeURIComponent(cmd), {
          cache: 'no-store',
          keepalive: true
        });

        const text = await res.text();
        last.textContent = cmd;
        if (!fireAndForget) {
          msg.textContent = text;
        }
      } catch (e) {
        if (!fireAndForget) {
          msg.textContent = 'Connection error';
        }
      }
    }

    function startHold(cmd) {
      if (activeMotionCmd === cmd) {
        return;
      }

      stopHold(true);
      activeMotionCmd = cmd;
      document.getElementById('msg').textContent = 'Holding ' + cmd;
      document.getElementById('cmd').textContent = cmd;

      sendCmd(cmd, true);
      holdTimer = setInterval(() => {
        if (activeMotionCmd === cmd) {
          sendCmd(cmd, true);
        }
      }, HOLD_SEND_INTERVAL_MS);
    }

    function stopHold(silent) {
      if (!activeMotionCmd) {
        return;
      }

      clearInterval(holdTimer);
      holdTimer = null;
      activeMotionCmd = '';
      sendCmd('S', true);

      if (!silent) {
        document.getElementById('cmd').textContent = 'S';
        document.getElementById('msg').textContent = 'Stopped';
      }
    }

    for (const btn of document.querySelectorAll('button.hold')) {
      const cmd = btn.dataset.cmd;
      btn.addEventListener('pointerdown', (e) => {
        e.preventDefault();
        btn.setPointerCapture(e.pointerId);
        startHold(cmd);
      });
      btn.addEventListener('pointerup', () => stopHold(false));
      btn.addEventListener('pointerleave', () => stopHold(false));
      btn.addEventListener('pointercancel', () => stopHold(false));
      btn.addEventListener('lostpointercapture', () => stopHold(false));
      btn.addEventListener('contextmenu', (e) => e.preventDefault());
      btn.style.touchAction = 'none';
    }

    window.addEventListener('blur', () => stopHold(true));
    window.addEventListener('pagehide', () => stopHold(true));
    document.addEventListener('visibilitychange', () => {
      if (document.visibilityState !== 'visible') {
        stopHold(true);
      }
    });
  </script>
</body>
</html>
)rawliteral";

void sendRobotCommand(const char command) {
  Serial2.write(command);
}

void handleRoot() {
  server.send_P(200, "text/html", indexHtml);
}

void handleCommand() {
  if (!server.hasArg("go") || server.arg("go").length() == 0) {
    server.send(400, "text/plain", "Missing command");
    return;
  }

  const char command = server.arg("go")[0];
  switch (command) {
    case 'F':
    case 'B':
    case 'L':
    case 'R':
    case 'S':
    case 'P':
    case 'Q':
    case 'U':
    case 'W':
    case 'V':
    case 'O':
    case 'X':
    case 'K':
      sendRobotCommand(command);
      server.send(200, "text/plain", String("Sent ") + command);
      break;
    default:
      server.send(400, "text/plain", "Invalid command");
      break;
  }
}

void handleNotFound() {
  server.send(404, "text/plain", "Not found");
}

void setup() {
  Serial.begin(115200);
  Serial2.begin(ROBOT_BAUD, SERIAL_8N1, ESP32_RX_PIN, ESP32_TX_PIN);

  WiFi.mode(WIFI_AP);
  WiFi.setSleep(false);
  esp_wifi_set_ps(WIFI_PS_NONE);
  WiFi.softAPConfig(apIP, apGateway, apSubnet);
  WiFi.softAP(apSsid, apPassword);

  server.on("/", handleRoot);
  server.on("/cmd", handleCommand);
  server.onNotFound(handleNotFound);
  server.begin();

  Serial.println();
  Serial.println("ESP32 Spider Robot Web Controller started");
  Serial.print("Reset reason: ");
  Serial.println((int)esp_reset_reason());
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());
}

void loop() {
  server.handleClient();
  delay(2);
}