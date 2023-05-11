#include <Wire.h>
#include <DHT.h>
#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>
#include <WiFiMulti.h>

WiFiMulti wifiMulti;

#define DEVICE "ESP32"
#define DHTTYPE DHT22
uint8_t DHTPin = 4;
DHT dht(DHTPin, DHTTYPE);

#define WIFI_SSID "ESP32-host"
#define WIFI_PASSWORD "parola123"

#define INFLUXDB_URL "https://eu-central-1-1.aws.cloud2.influxdata.com"
#define INFLUXDB_TOKEN "vGv2bhc_YDlzB0SMeTpqT6dq2SQ977w5ZXUdiv_1QLZUz7wlMCQjSVcHPI7NQr9A7lH1PhOAQfpCuEmj1bYq5w=="
#define INFLUXDB_ORG "2da19e0fa64d9e56"
#define INFLUXDB_BUCKET "ESP32"
#define TZ_INFO "UTC3"

InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);
Point sensor("measurements");

void setup() 
{
  Serial.begin(115200);
  pinMode(DHTPin, INPUT);
  dht.begin();

  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to WiFi: ");
  while (wifiMulti.run() != WL_CONNECTED) 
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println();

  sensor.addTag("device", DEVICE);
  sensor.addTag("SSID", WiFi.SSID());

  timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");

  if (client.validateConnection()) 
  {
    Serial.print("Connected to InfluxDB: ");
    Serial.println(client.getServerUrl());
  } 
  else 
  {
    Serial.print("InfluxDB connection failed: ");
    Serial.println(client.getLastErrorMessage());
  }
}

void loop() 
{
  sensor.clearFields();

  sensor.addField("Temperature", dht.readTemperature());
  sensor.addField("Humidity", dht.readHumidity());

  Serial.print("Writing: ");
  Serial.println(client.pointToLineProtocol(sensor));

  if (wifiMulti.run() != WL_CONNECTED) 
  {
    Serial.println("Wifi connection lost");
  }
  
  if (!client.writePoint(sensor)) 
  {
    Serial.print("InfluxDB write failed: ");
    Serial.println(client.getLastErrorMessage());
  }
  Serial.println("");
  Serial.println("Delay 0.5 min");
  delay(30000);
}
