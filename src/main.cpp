#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_MPU6050 mpu;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

int logCount = 0;
unsigned long lastSmoothLog = 0;
const unsigned long SMOOTH_LOG_INTERVAL_MS = 2000; // Log a smooth sample every 2s

void showIdleScreen() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("ROAD BUMP TRACKER");
  display.drawLine(0, 12, 128, 12, SSD1306_WHITE);

  display.setTextSize(2);
  display.setCursor(0, 22);
  display.println("SMOOTH");

  display.setTextSize(1);
  display.setCursor(0, 50);
  display.print("Total Logs: ");
  display.println(logCount);
  display.display();
}

void printDbLog(float impact, const char* severity) {
  logCount++;
  Serial.print("[MOCK DB LOG]: {\"log_id\": ");
  Serial.print(logCount);
  Serial.print(", \"impact\": ");
  Serial.print(impact, 2);
  Serial.print(", \"severity\": \"");
  Serial.print(severity);
  Serial.println("\"}");
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Wire.begin(21, 22);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("[ERROR] OLED not found at 0x3C! Check wiring.");
    while (1) {
      delay(10);
    }
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 20);
  display.println("Starting system...");
  display.display();

  if (!mpu.begin()) {
    Serial.println("[ERROR] MPU6050 not found! Check wiring.");
    display.clearDisplay();
    display.setCursor(0, 20);
    display.println("MPU6050 ERROR!");
    display.display();
    while (1) {
      delay(10);
    }
  }

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  Serial.println("[OK] MPU6050 & OLED Ready! Logging SMOOTH and BUMP data.");
  showIdleScreen();
}

void loop() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  float z_accel = a.acceleration.z;
  float impact = abs(z_accel - 9.8);
  unsigned long now = millis();

  if (impact > 8.5) {
    const char* severity = (impact > 16.0) ? "HIGH" : "MEDIUM";

    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 4);
    display.println("! BUMP !");

    display.setTextSize(1);
    display.setCursor(0, 28);
    display.print("Severity: ");
    display.println(severity);

    display.setCursor(0, 40);
    display.print("Impact  : ");
    display.print(impact, 1);
    display.println(" m/s2");

    display.setCursor(0, 52);
    display.print("Logged #");
    display.println(logCount + 1);
    display.display();

    printDbLog(impact, severity);
    lastSmoothLog = now; // Reset smooth timer so we don't log smooth right after a bump

    delay(1500);
    showIdleScreen();
  } else if (now - lastSmoothLog >= SMOOTH_LOG_INTERVAL_MS) {
    lastSmoothLog = now;
    printDbLog(impact, "SMOOTH");
  }

  delay(50);
}