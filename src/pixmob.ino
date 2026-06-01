#include <pixmob_cement.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Arduino_JSON.h>

// ============================================================
//  WiFi instellingen — pas aan naar jouw netwerk
// ============================================================
const char* WIFI_SSID = "jouw_wifi_naam";
const char* WIFI_PASS = "jouw_wifi_wachtwoord";

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
Pixmob batch;

int Params[] = {85, 0, 0, 2, 2, 2, 0, 0};
// Params[0] = Red      (0-255)
// Params[1] = Green    (0-255)
// Params[2] = Blue     (0-255)
// Params[3] = Attack   (0-7)
// Params[4] = Hold     (0-7)
// Params[5] = Release  (0-7)
// Params[6] = Random   (0-7)
// Params[7] = Group    (0-31, 0 = all)

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<title>XIAO PIXMOB</title>
<meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
<style>
* { text-align: center; }
.slider { appearance: none; width: 80%; height: 20px; outline: none; border-radius: 10px; accent-color: black; }
</style>
</head>
<body>
<h1>XIAO PIXMOB</h1>
<div>
<script>
var websocket;
function onOpen(event) { websocket.send("getValues"); }
function onClose(event) { setTimeout(initWebSocket, 2000); }
function onMessage(event) {
  Object.entries(JSON.parse(event.data)).forEach(([key, value]) => {
    document.getElementById(key).previousElementSibling.previousElementSibling.innerHTML = value;
    document.getElementById(key).value = value;
  })
}
function initWebSocket() {
  websocket = new WebSocket(`ws://${window.location.hostname}/ws`);
  websocket.onopen = onOpen;
  websocket.onclose = onClose;
  websocket.onmessage = onMessage;
}
window.addEventListener('load', () => initWebSocket());
function updateSliderPWM(element) { websocket.send(element.id + "|" + document.getElementById(element.id).value.toString()); }
SlidersArray = [["Red","255","red"],["Green","255","green"],["Blue","255","blue"],
                ["Attack","7","gray"],["Hold","7","gray"],["Release","7","gray"],
                ["Random","7","gray"],["Group","31","gray"]];
SlidersArray.forEach(function(element, index) {
  document.writeln('<div><p>', element[0], ': <output></output><br>');
  document.writeln('<input type="range" onchange="updateSliderPWM(this)" id="Slider', index+1,
    '" min="0" max="', element[1], '" step="1" value="0" class="slider" style="background:', element[2],
    ';" oninput="this.previousElementSibling.previousElementSibling.value = this.value">');
  document.writeln('</p></div>');
});
</script>
</div>
</body>
</html>
)rawliteral";

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    JSONVar SliderValues;
    data[len] = 0;
    String message = (char *)data;
    for (int i = 0; i < 8; i++) {
      if (message.indexOf(String(i + 1) + "|") >= 0)
        Params[i] = message.substring(message.indexOf("|") + 1).toInt();
      SliderValues["Slider" + String(i + 1)] = String(Params[i]);
    }
    ws.textAll(JSON.stringify(SliderValues));
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT: break;
    case WS_EVT_DISCONNECT: break;
    case WS_EVT_DATA: handleWebSocketMessage(arg, data, len); break;
    default: break;
  }
}

void setup() {
  Serial.begin(115200);

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  batch.begin(D0);

  ws.onEvent(onEvent);
  server.addHandler(&ws);
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", index_html);
  });
  server.begin();
}

void loop() {
  batch.setFXtiming(Params[3], Params[4], Params[5], Params[6]);
  batch.sendColor(Params[0], Params[1], Params[2], Params[7]);
  delay(10);
  ws.cleanupClients();
}
