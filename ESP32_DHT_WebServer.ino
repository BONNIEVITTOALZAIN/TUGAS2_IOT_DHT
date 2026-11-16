#include <Arduino.h>
#include <WiFi.h>          
#include "DHT.h"


const char* ssid = "xioami_note_9";
const char* password = "qwerty1234";   

// Web Server di port 80
WiFiServer server(80);


#define DHTPIN 27          
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

float temperature = NAN;
float humidity = NAN;

unsigned long previousMillis = 0;
const long interval = 2000;

void setup() {
  Serial.begin(115200);
  delay(1000);

  dht.begin();

  // Membuat Access Point
  Serial.println("Setting Access Point...");
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);       // default 192.168.4.1

  server.begin();
  Serial.println("Web Server Started!");
}

void loop() {

  // Membaca sensor setiap 2 detik
  if (millis() - previousMillis >= interval) {
    previousMillis = millis();

    temperature = dht.readTemperature();
    humidity = dht.readHumidity();

    if (!isnan(temperature) && !isnan(humidity)) {
      Serial.print("Suhu: ");
      Serial.print(temperature);
      Serial.println(" C");

      Serial.print("Kelembapan: ");
      Serial.print(humidity);
      Serial.println(" %");
    } else {
      Serial.println("Sensor DHT Error!");
    }
  }

  WiFiClient client = server.available();

  if (client) {
    Serial.println("New Client");

    String currentLine = "";
    unsigned long timeout = millis();

    while (client.connected() && millis() - timeout < 2000) {

      if (client.available()) {
        char c = client.read();

        if (c == '\n') {

          if (currentLine.length() == 0) {

            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            client.println("<!DOCTYPE html><html>");
            client.println("<head>");
            client.println("<meta charset='UTF-8'>");
            client.println("<meta name='viewport' content='width=device-width, initial-scale=1'>");
            client.println("<meta http-equiv='refresh' content='3'>");

            client.println("<style>");
            client.println("body{font-family:Arial;text-align:center;background:#eaf2f8;padding:20px;}");
            client.println("h1{color:#154360;}");
            client.println(".card{background:white;padding:25px;margin:20px auto;max-width:350px;border-radius:15px;box-shadow:0 3px 10px #aaa;}");
            client.println(".value{font-size:40px;color:#2874a6;font-weight:bold;}");
            client.println(".label{font-size:18px;color:#555;}");
            client.println("</style>");

            client.println("</head><body>");

            client.println("<h1>MONITORING DHT11</h1>");
            client.println("<p>ESP32 LOCAL WEB SERVER</p>");

            if (!isnan(temperature) && !isnan(humidity)) {

              client.println("<div class='card'>");
              client.println("<div class='label'>Suhu Udara</div>");
              client.print("<div class='value'>");
              client.print(temperature, 1);
              client.println(" &deg;C</div>");
              client.println("</div>");

              client.println("<div class='card'>");
              client.println("<div class='label'>Kelembapan Udara</div>");
              client.print("<div class='value'>");
              client.print(humidity, 1);
              client.println(" %</div>");
              client.println("</div>");

              client.println("<p style='color:green;'>Sensor Terhubung</p>");

            } else {
              client.println("<h3 style='color:red;'>Sensor Tidak Terbaca</h3>");
            }

            client.println("<p>Update setiap 3 detik</p>");
            client.println("</body></html>");
            client.println();

            break;

          } else {
            currentLine = "";
          }

        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }

    client.stop();
    Serial.println("Client disconnected");
  }
}
