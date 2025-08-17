// ==== ESP32 → Blynk (LoRa UART in) ====
// Prints raw received line AND parsed fields (count/temp/hum) to Serial

// ---- Blynk setup (your values kept) ----
#define BLYNK_TEMPLATE_ID "TMPL6Qo8aZF3f"
#define BLYNK_TEMPLATE_NAME "SmartFarmX"
#define BLYNK_AUTH_TOKEN "lYA4_W3KiA0g36qBxD3e-ICvulTRZNh_"

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

// ---- LoRa UART pins ----
#define RXD2 16  // LoRa TX → ESP32 RX
#define TXD2 17  // ESP32 TX → LoRa RX

// ---- WiFi (your values kept) ----
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "67";
char pass[] = "horimiya2.4";

BlynkTimer timer;
bool printedHeader = false;

static void printHeaderOnce() {
  if (!printedHeader) {
    Serial.println("\n--- LoRa Parsed Data ---");
    Serial.println("COUNT\tTEMP_C\tHUM_%");
    printedHeader = true;
  }
}

void checkLoRa() {
  while (Serial2.available()) {
    String line = Serial2.readStringUntil('\n');
    line.trim();
    if (!line.length()) continue;

    // Always show the raw line we got over LoRa
    Serial.println("LoRa Raw: " + line);

    // Only parse packets that start with our tag
    // Expected: DATA:<count>,Temp=24.5C,Hum=60.0%
    if (!line.startsWith("DATA:")) {
      // ignore boot/status lines like "Power on"
      continue;
    }

    // ---------- Parse COUNT ----------
    unsigned long count = 0;
    {
      int tagEnd = line.indexOf(':');        // after "DATA"
      int comma  = line.indexOf(',', tagEnd + 1);
      if (tagEnd > 0 && comma > tagEnd) {
        String countStr = line.substring(tagEnd + 1, comma);
        countStr.trim();
        count = (unsigned long) countStr.toInt();
      }
    }

    // ---------- Parse Temp & Hum ----------
    auto extractAfter = [](const String& s, const String& key) -> float {
      int i = s.indexOf(key);
      if (i < 0) return NAN;
      i += key.length();
      // Skip separators
      while (i < (int)s.length() && (s[i] == ' '  s[i] == ':'  s[i] == '=')) i++;
      int j = i;
      while (j < (int)s.length() && (isDigit(s[j])  s[j] == '.'  s[j] == '-')) j++;
      String num = s.substring(i, j);
      return num.toFloat();
    };

    float temp = extractAfter(line, "Temp");
    float hum  = extractAfter(line, "Hum");

    if (isnan(temp) || isnan(hum)) {
      Serial.println("ℹ️ DATA line but couldn't parse numbers, skipping.\n");
      continue;
    }

    // ---------- Print parsed data nicely ----------
    printHeaderOnce();
    Serial.printf("%lu\t%.1f\t%.1f\n", count, temp, hum);
    Serial.printf("Parsed → 🌡 %.1f °C   💧 %.1f %%\n\n", temp, hum);

    // ---------- Send to Blynk ----------
    Blynk.virtualWrite(V0, temp);
    Blynk.virtualWrite(V1, hum);
  }
}

void setup() {
  Serial.begin(115200);

  // LoRa UART
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  Serial2.setTimeout(100); // make readStringUntil('\n') snappier

  // Connect WiFi
  WiFi.begin(ssid, pass);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.println("IP address: " + WiFi.localIP().toString());

  // Connect Blynk
  Blynk.config(auth);
  Blynk.connect();

  Serial.println("ESP32 LoRa Receiver with Blynk ready.");

  // Poll LoRa regularly
  timer.setInterval(2000L, checkLoRa);
}

void loop() {
  Blynk.run();
  timer.run();
}