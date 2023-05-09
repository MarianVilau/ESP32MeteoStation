#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include "SPIFFS.h"
#include <DHT.h>

// Replace with your network credentials
const char* ssid = "MyESP32AP"; // Numele rețelei WiFi create
const char* password = "parola123"; // Parola pentru rețeaua WiFi

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Initialize DHT sensor
#define DHTPIN 4          // DHT data pin
#define DHTTYPE DHT22     // DHT 22 (AM2302)
DHT dht(DHTPIN, DHTTYPE);

String readDHTTemperature() {
  // Read temperature as Celsius
  float t = dht.readTemperature();
  if (isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return "";
  }
  else {
    Serial.println(t);
    return String(t);
  }
}

String readDHTHumidity() {
  float h = dht.readHumidity();
  if (isnan(h)) {
    Serial.println("Failed to read from DHT sensor!");
    return "";
  }
  else {
    Serial.println(h);
    return String(h);
  }
}

void setup() {
  // Serial port for debugging purposes
  Serial.begin(115200);

  // Initialize DHT sensor
  dht.begin();

  // Connect to Wi-Fi
  WiFi.softAP(ssid, password);
  Serial.println("Access point started");
  Serial.print("SSID: ");
  Serial.println(ssid);
  Serial.print("Password: ");
  Serial.println(password);
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());

  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  File file = SPIFFS.open("/test_index.html", FILE_WRITE);

  if (!file) {
    Serial.println("There was an error opening the file for writing");
    return;
  }
  String code="<!DOCTYPE HTML><html>"
"<head>"
  "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
  "<script src=\"https://code.highcharts.com/highcharts.js\"></script>"
  "<style>"
    "body {"
      "min-width: 310px;"
    	"max-width: 800px;"
    	"height: 400px;"
      "margin: 0 auto;"
    "}"
    "h2 {"
      "font-family: Arial;"
      "font-size: 2.5rem;"
      "text-align: center;"
    "}"
  "</style>"
"</head>"
"<body>"
  "<h2>ESP32 Meteo Station</h2>"
  "<div id=\"chart-temperature\" class=\"container\"></div>"
  "<div id=\"chart-humidity\" class=\"container\"></div>"
"</body>"
"<script>"
"var chartT = new Highcharts.Chart({"
  "chart:{ renderTo : 'chart-temperature' },"
  "title: { text: 'DHT22 Temperature' },"
  "series: [{"
    "showInLegend: false,"
    "data: []"
  "}],"
  "plotOptions: {"
    "line: { animation: false,"
      "dataLabels: { enabled: true }"
    "},"
    "series: { color: '#059e8a' }"
  "},"
  "xAxis: { type: 'datetime',"
    "dateTimeLabelFormats: { second: '%H:%M:%S' }"
  "},"
  "yAxis: {"
    "title: { text: 'Temperature (Celsius)' }"
  "},"
  "credits: { enabled: false }"
"});"
"setInterval(function ( ) {"
  "var xhttp = new XMLHttpRequest();"
  "xhttp.onreadystatechange = function() {"
    "if (this.readyState == 4 && this.status == 200) {"
      "var x = (new Date()).getTime(),"
          "y = parseFloat(this.responseText);"
      "if(chartT.series[0].data.length > 40) {"
        "chartT.series[0].addPoint([x, y], true, true, true);"
      "} else {"
        "chartT.series[0].addPoint([x, y], true, false, true);"
      "}"
    "}"
  "};"
  "xhttp.open(\"GET\", \"/temperature\", true);"
  "xhttp.send();"
"}, 30000 ) ;"

"var chartH = new Highcharts.Chart({"
  "chart:{ renderTo:'chart-humidity' },"
  "title: { text: 'DHT22 Humidity' },"
  "series: [{"
    "showInLegend: false,"
    "data: []"
  "}],"
  "plotOptions: {"
    "line: { animation: false,"
      "dataLabels: { enabled: true }"
    "}"
  "},"
  "xAxis: {"
    "type: 'datetime',"
    "dateTimeLabelFormats: { second: '%H:%M:%S' }"
  "},"
  "yAxis: {"
    "title: { text: 'Humidity (%)' }"
  "},"
  "credits: { enabled: false }"
"});"
"setInterval(function ( ) {"
  "var xhttp = new XMLHttpRequest();"
  "xhttp.onreadystatechange = function() {"
    "if (this.readyState == 4 && this.status == 200) {"
      "var x = (new Date()).getTime(),"
          "y = parseFloat(this.responseText);"
      "if(chartH.series[0].data.length > 40) {"
        "chartH.series[0].addPoint([x, y], true, true, true);"
      "} else {"
        "chartH.series[0].addPoint([x, y], true, false, true);"
      "}"
    "}"
  "};"
  "xhttp.open(\"GET\", \"/humidity\", true);"
  "xhttp.send();"
"}, 30000);"
"</script>"
"</html>";
  file.print(code);
  file.close();

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/test_index.html");
  });
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/plain", readDHTTemperature().c_str());
  });
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/plain", readDHTHumidity().c_str());
  });

  // Start server
  server.begin();
}

void loop() {
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/plain", readDHTTemperature().c_str());
  });
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/plain", readDHTHumidity().c_str());
  });
}
