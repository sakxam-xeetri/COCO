#include <WiFi.h>
#include <WebServer.h>
#include <esp_wifi.h>

// -----------------------------------------------------------------------------
// ESP32-CAM Web Controller for the COCO spider robot
//
// Use this sketch on an ESP32-CAM board even if the camera module is damaged.
// It does not use any camera functions. It only provides a Wi-Fi access point
// and a control web page, then forwards commands to the robot controller over
// UART.
//
// Suggested ESP32-CAM wiring for AI Thinker boards:
//   GPIO14 -> Robot RX
//   GPIO13 <- Robot TX (optional, only if you want replies)
//   GND    -> Robot GND
//
// Commands:
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

// ESP32-CAM UART pins.
// Adjust these if your board or wiring uses different free GPIOs.
static const int ROBOT_RX_PIN = 13;
static const int ROBOT_TX_PIN = 14;
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
  <title>COCO CAM Controller</title>
  <style>
    :root {
      --bg1: #08111f;
      --bg2: #15213c;
      --panel: rgba(9, 15, 28, 0.86);
      --panel-strong: rgba(15, 23, 42, 0.94);
      --accent: #4ade80;
      --text: #eff6ff;
      --muted: #9fb0c9;
      --border: rgba(148, 163, 184, 0.18);
    }
    * { box-sizing: border-box; }
    html, body, button {
      -webkit-touch-callout: none;
      -webkit-user-select: none;
      -ms-user-select: none;
      user-select: none;
    }
    html, body { min-height: 100%; }
    body {
      margin: 0;
      min-height: 100vh;
      font-family: "Segoe UI", Tahoma, Geneva, Verdana, sans-serif;
      color: var(--text);
      background:
        radial-gradient(circle at top, rgba(56, 189, 248, 0.16), transparent 30%),
        radial-gradient(circle at 15% 80%, rgba(74, 222, 128, 0.16), transparent 25%),
        radial-gradient(circle at 85% 15%, rgba(251, 113, 133, 0.13), transparent 22%),
        linear-gradient(160deg, var(--bg1), var(--bg2));
      display: grid;
      place-items: center;
      padding: 16px;
      touch-action: manipulation;
    }
    .card {
      width: min(620px, 100%);
      background: var(--panel);
      border: 1px solid var(--border);
      border-radius: 26px;
      padding: 20px;
      box-shadow: 0 30px 90px rgba(0, 0, 0, 0.42);
      backdrop-filter: blur(14px);
      overflow: hidden;
      position: relative;
    }
    .card::before {
      content: '';
      position: absolute;
      inset: 0;
      background: linear-gradient(135deg, rgba(255,255,255,0.08), transparent 30%, transparent 70%, rgba(255,255,255,0.04));
      pointer-events: none;
    }
    .hero {
      position: relative;
      z-index: 1;
      display: flex;
      justify-content: space-between;
      gap: 16px;
      align-items: flex-start;
      margin-bottom: 18px;
    }
    h1 {
      margin: 0 0 8px;
      font-size: clamp(1.8rem, 4vw, 2.45rem);
      letter-spacing: 0.01em;
      line-height: 1.05;
    }
    .subtitle {
      margin: 0;
      color: var(--muted);
      line-height: 1.5;
      max-width: 32rem;
    }
    .badge {
      display: inline-flex;
      align-items: center;
      gap: 8px;
      padding: 10px 14px;
      border-radius: 999px;
      background: rgba(255,255,255,0.06);
      border: 1px solid var(--border);
      color: var(--text);
      font-size: 0.9rem;
      white-space: nowrap;
      flex: 0 0 auto;
    }
    .status {
      display: flex;
      justify-content: space-between;
      gap: 12px;
      align-items: center;
      margin-bottom: 18px;
      padding: 14px 16px;
      border-radius: 18px;
      background: var(--panel-strong);
      border: 1px solid var(--border);
      font-size: 0.95rem;
      position: relative;
      z-index: 1;
    }
    .status-label { color: var(--muted); }
    .grid {
      position: relative;
      z-index: 1;
      display: grid;
      grid-template-columns: repeat(3, 1fr);
      gap: 12px;
    }
    button {
      border: 0;
      border-radius: 18px;
      padding: 16px 12px;
      font-size: 1rem;
      font-weight: 800;
      letter-spacing: 0.01em;
      color: #07101d;
      background: linear-gradient(180deg, #e2e8f0, #cbd5e1);
      cursor: pointer;
      transition: transform 0.14s ease, filter 0.14s ease, box-shadow 0.14s ease;
      min-height: 64px;
      box-shadow: 0 10px 24px rgba(0, 0, 0, 0.18);
      -webkit-tap-highlight-color: transparent;
      touch-action: none;
    }
    button:hover { transform: translateY(-1px); filter: brightness(1.02); }
    button:active { transform: scale(0.97); filter: brightness(0.95); }
    .forward {
      background: linear-gradient(180deg, #bbf7d0, #22c55e);
      font-size: 1.08rem;
      min-height: 88px;
    }
    .back { background: linear-gradient(180deg, #fecdd3, #fb7185); }
    .left, .right { background: linear-gradient(180deg, #bae6fd, #38bdf8); }
    .gesture { background: linear-gradient(180deg, #fde68a, #f59e0b); }
    .basic-pos { background: linear-gradient(180deg, #ddd6fe, #8b5cf6); color: #fff; }
    .spider-pos { background: linear-gradient(180deg, #fbcfe8, #ec4899); color: #fff; }
    .led-on { background: linear-gradient(180deg, #bbf7d0, #4ade80); }
    .led-off { background: linear-gradient(180deg, #e2e8f0, #94a3b8); }
    .led-blink { background: linear-gradient(180deg, #c4b5fd, #8b5cf6); color: #fff; }
    .wide { grid-column: span 3; }
    .nav-left, .nav-right { min-height: 72px; }
    .nav-center {
      min-height: 92px;
      display: grid;
      place-items: center;
      box-shadow: 0 16px 30px rgba(34, 197, 94, 0.22);
    }
    .pill {
      display: inline-flex;
      align-items: center;
      gap: 6px;
      padding: 6px 10px;
      border-radius: 999px;
      background: rgba(255,255,255,0.08);
      color: var(--muted);
      font-size: 0.8rem;
      margin-bottom: 8px;
    }
    .hint {
      margin-top: 18px;
      font-size: 0.9rem;
      color: var(--muted);
      position: relative;
      z-index: 1;
      display: flex;
      justify-content: space-between;
      gap: 12px;
      flex-wrap: wrap;
    }
    .pulse {
      color: var(--accent);
      font-weight: 700;
    }
    .footer-note {
      color: var(--muted);
    }
    @media (max-width: 560px) {
      .card { padding: 16px; border-radius: 22px; }
      .hero { flex-direction: column; }
      .badge { align-self: flex-start; }
      .grid { gap: 10px; }
      button { min-height: 60px; padding: 14px 10px; }
      .forward { min-height: 82px; }
      .hint { flex-direction: column; }
    }
  </style>
</head>
<body>
  <div class="card">
    <div class="hero">
      <div>
        <div class="pill">ESP32-CAM Controller</div>
        <h1>Spider Robot Control</h1>
        <p class="subtitle">This board does not use the camera. Hold a direction to move, release to stop, and use the pose buttons for basic stance control.</p>
      </div>
      <div class="badge">Wi-Fi AP: <span class="pulse">SpiderRobot</span></div>
    </div>
    <div class="status">
      <div><span class="status-label">Last command</span> <span class="pulse" id="cmd">none</span></div>
      <div id="msg">Ready</div>
    </div>
    <div class="grid">
      <div></div>
      <button class="forward hold nav-center" data-cmd="F">Forward<br><span class="pill">Hold to move</span></button>
      <div></div>
      <button class="left hold nav-left" data-cmd="L">Left</button>
      <button class="back hold" data-cmd="B">Back</button>
      <button class="right hold nav-right" data-cmd="R">Right</button>
      <button class="basic-pos wide" onclick="sendCmd('P')">Basic Position</button>
      <button class="spider-pos wide" onclick="sendCmd('Q')">Spider Position</button>
      <button class="gesture wide" onclick="sendCmd('U')">Hand Shake</button>
      <button class="gesture wide" onclick="sendCmd('W')">Hand Wave</button>
      <button class="gesture wide" onclick="sendCmd('V')">Body Dance</button>
      <button class="led-on wide" onclick="sendCmd('O')">LED On</button>
      <button class="led-off wide" onclick="sendCmd('X')">LED Off</button>
      <button class="led-blink wide" onclick="sendCmd('K')">LED Blink</button>
    </div>
    <div class="hint">
      <div>Wi-Fi password: <span class="pulse">12345678</span></div>
      <div class="footer-note">Release any direction button to send stop.</div>
    </div>
  </div>
  <script>
    let activeMotionCmd = '';

    async function sendCmd(cmd, fireAndForget = false) {
      const msg = document.getElementById('msg');
      const last = document.getElementById('cmd');
      try {
        if (!fireAndForget) {
          msg.textContent = 'Sending ' + cmd + '...';
        }

        const res = await fetch('/cmd?go=' + encodeURIComponent(cmd), {
          cache: 'no-store'
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
    }

    function stopHold(silent) {
      if (!activeMotionCmd) {
        return;
      }

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
        e.stopPropagation();
        btn.setPointerCapture(e.pointerId);
        startHold(cmd);
      });
      btn.addEventListener('pointerup', () => stopHold(false));
      btn.addEventListener('pointerleave', () => stopHold(false));
      btn.addEventListener('pointercancel', () => stopHold(false));
      btn.addEventListener('lostpointercapture', () => stopHold(false));
      btn.addEventListener('contextmenu', (e) => e.preventDefault());
    }

    document.addEventListener('contextmenu', (e) => e.preventDefault());
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
  Serial1.write(command);
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
  Serial1.begin(ROBOT_BAUD, SERIAL_8N1, ROBOT_RX_PIN, ROBOT_TX_PIN);

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
  Serial.println("ESP32-CAM Spider Robot Web Controller started");
  Serial.print("Reset reason: ");
  Serial.println((int)esp_reset_reason());
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());
}

void loop() {
  server.handleClient();
  delay(2);
}
