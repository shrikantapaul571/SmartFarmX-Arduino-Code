// ==== ESP8266 → LoRa (transparent UART out) ====
// Uses your pins and structure; adds a DATA: prefix for easy parsing.

#include <SoftwareSerial.h>
#include <DHT.h>

#define DHTPIN 2      // GPIO2 (D4 on many NodeMCU boards)
#define DHTTYPE DHT11

// NOTE: GPIO13 = D7 (RX), GPIO15 = D8 (TX).
// Keep 9600 for SoftwareSerial reliability on ESP8266.
SoftwareSerial lora(13, 15);  // RX, TX for LoRa

DHT dht(DHTPIN, DHTTYPE);
unsigned long count = 1;  // Message counter

void setup() {
  Serial.begin(115200);
  lora.begin(9600);
  dht.begin();
  delay(2000);
  Serial.println("📤 ESP8266 LoRa Sender with DHT11 started.");
}

void loop() {
  float temperature = dht.readTemperature(); // °C
  float humidity    = dht.readHumidity();    // %

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("❌ Failed to read from DHT11 sensor!");
    delay(2000);
    return;
  }

  // Example: DATA:12,Temp=24.5C,Hum=60.0%
  String payload = "DATA:" + String(count++) +
                   ",Temp=" + String(temperature, 1) + "C" +
                   ",Hum="  + String(humidity, 1) + "%";

  lora.println(payload);
  Serial.println("✅ Sent via LoRa: " + payload);

  delay(5000);
}
