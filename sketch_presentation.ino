#include <WiFi.h>
#include <WebServer.h>
#include <vector>

const char* ssid = "";
const char* password = "";
const char* consoleUser = "";
const char* consolePass = "";

const int pinGreenLED = 27;
const int pinButtonReset = 32; 
int buttonState = 0;

std::vector<String> taskOneClients;

WebServer server(80);

void handlePing() {
  String json = "{\"status\":\"alive\"}";
  server.send(200, "application/json", json);

  digitalWrite(pinGreenLED, HIGH);
  delay(1000);
  digitalWrite(pinGreenLED, LOW);
}


void task_one() {
  const char* solution = "matchme";
  if (!server.hasArg("param")) {
    server.send(400, "application/json", "{\"error\": \"Missing required parameter 'param'\"}");
    return;
  }

  String paramValue = server.arg("param");
  String clientName = server.hasArg("name") ? server.arg("name") : "anonymous";
  unsigned long timestamp = millis();

  if (paramValue == solution) {
    String logEntry = clientName + " at " + String(timestamp / 1000) + "s";
    taskOneClients.push_back(logEntry);

    server.send(200, "application/json", "{\"result\":true}");
  } else {
    server.send(200, "application/json", "{\"result\":false}");
  }
}



void handleWebConsole() {
  if (!server.authenticate(consoleUser, consolePass)) {
    return server.requestAuthentication();
  }

  String html = "<!DOCTYPE html><html><head>  <meta http-equiv="refresh" content="10"> <title>Admin Console</title></head><body>";
  html += "<h1>Successful Task Completions</h1><ul>";

  for (String entry : taskOneClients) {
    html += "<li>" + entry + "</li>";
  }

  html += "</ul></body></html>";
  server.send(200, "text/html", html);
}


void handleResetButton(){
  buttonState = digitalRead(pinButtonReset);
  if(buttonState ==LOW){
    delay(200);  // debounce delay
    Serial.println("Reset-Button pressed");
    ESP.restart();
  }
}

void setup() {
  Serial.begin(115200);

  Serial.println("Setup Executed");
  pinMode(pinGreenLED, OUTPUT);
  digitalWrite(pinGreenLED, HIGH);
  //for led
  pinMode(pinButtonReset, INPUT_PULLUP);


  WiFi.begin(ssid, password);
  int wifiDisconnectCounter = 0;
  while (WiFi.status() != WL_CONNECTED) {
    wifiDisconnectCounter ++;
    delay(500);
    Serial.println("Connecting...");
    Serial.print(WiFi.status());

    handleResetButton();

    if(wifiDisconnectCounter > 10){
      wifiDisconnectCounter = 0;
      Serial.println("Resetting wifi");
        WiFi.disconnect();
        WiFi.begin(ssid, password);

    }

  }

  Serial.print("Connected. IP: ");
  Serial.println(WiFi.localIP());

  server.on("/ping", handlePing);
  server.on("/task_one", task_one);
    server.on("/login", handleWebConsole);
  server.begin();
}

void loop() {
  server.handleClient();
  handleResetButton();
}

