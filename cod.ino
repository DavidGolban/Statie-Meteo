#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "Adafruit_CCS811.h"

Adafruit_BME280 bme; 
Adafruit_CCS811 ccs;

const char* ssid = "CeaMaiPuternicaStatieMeteo"; 
const char* password = "12345678";   
const float mmHg = 133.322387415f;

ESP8266WebServer server(80);

void handleRoot() {
  float temp = bme.readTemperature();
  float hum = bme.readHumidity();
  float pres = bme.readPressure() / mmHg;
  int co2 = ccs.geteCO2();
  int tvoc = ccs.getTVOC();

  String html = "<html><head><meta name='viewport' content='width=device-width, initial-scale=1' charset='UTF-8'>";
  html += "<meta http-equiv='refresh' content='5'>"; //refresh meta-tag
  html += "<style>body{font-family:sans-serif; text-align:center; background:#2c3e50; color:white;}";
  html += ".box{background:#34495e; padding:20px; border-radius:15px; display:inline-block; margin-top:30px; border:2px solid #3498db; box-shadow: 0 10px 20px rgba(0,0,0,0.5);}</style>";
  html += "</head><body><div class='box'><h1>📊 Date colectate</h1>";
  html += "<h2>🌡️ Temp: " + String(temp, 2) + "°C</h2>";
  html += "<h2>💧 Hum: " + String(hum, 2) + "%</h2>";
  html += "<h2>🎈 Pres: " + String(pres, 1) + " mmHg</h2>";
  html += "<hr style='border:1px solid #3498db'>";
  html += "<h2 style='color:#2ecc71'>☁️ CO2: " + String(co2) + " ppm</h2>";
  html += "<h2 style='color:#f1c40f'>✨ TVOC: " + String(tvoc) + " ppb</h2>";
  html += "</div><p>Pagina se actualizează singură o dată la 5 secunde.</p></body></html>";
  
  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(115200);
  Wire.begin(D2, D1);
  pinMode(LED_BUILTIN, OUTPUT);

  if (!bme.begin(0x76, &Wire)) Serial.println("Eroare: BME280!");
  if (!ccs.begin()) Serial.println("Eroare: CCS811!");
  delay(1000);

  // Pornire Access Point
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  
  Serial.println("\n--- Server Pornit (Mod Hotspot) ---");
  Serial.print("1. Conectează telefonul la: "); Serial.println(ssid);
  Serial.print("2. Scrie în browser: "); Serial.println(myIP);

  server.on("/", handleRoot);
  server.begin();
}

void loop() {
  server.handleClient();

  if (ccs.available()) {
    float temp = bme.readTemperature();
    float hum = bme.readHumidity();
    float pres = bme.readPressure() / mmHg;
    
    if(!ccs.readData()){
      ccs.setEnvironmentalData(hum, temp);
      int co2 = ccs.geteCO2();
      int tvoc = ccs.getTVOC();
      Serial.printf("🌡️ Temp: %.2f°C | 💧 Hum: %.2f%% | 🎈 Pres: %.1f mmHg\n", temp, hum, pres);
      Serial.printf("☁️ CO2: %d ppm | ✨ TVOC: %d ppb\n", co2, tvoc);
      
      digitalWrite(LED_BUILTIN, (co2 > 1000) ? LOW : HIGH);
      Serial.println("------------------------------------------------");
    }
  }
  delay(1000);
}