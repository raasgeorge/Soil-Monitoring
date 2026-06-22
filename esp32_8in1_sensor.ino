/*
 * ESP32 Soil Monitoring System - 8-in-1 RS485 Sensor
 *
 * Hardware Requirements:
 * - ESP32 Dev Board
 * - 8-in-1 Soil Composite Sensor (RS485)
 * - MAX485 or TTL to RS485 Converter Module
 * - 12V Power Supply (for sensor)
 *
 * Wiring:
 * RS485 Converter to ESP32:
 * - MAX485 RO  -> ESP32 GPIO 16 (RX2)
 * - MAX485 DI  -> ESP32 GPIO 17 (TX2)
 * - MAX485 DE  -> ESP32 GPIO 4  (Direction control)
 * - MAX485 RE  -> ESP32 GPIO 4  (Direction control, tied with DE)
 * - MAX485 VCC -> ESP32 5V
 * - MAX485 GND -> ESP32 GND
 *
 * 8-in-1 Sensor to MAX485:
 * - Sensor A   -> MAX485 A
 * - Sensor B   -> MAX485 B
 * - Sensor VCC -> 12V Power Supply (check sensor specs)
 * - Sensor GND -> Common GND
 *
 * 8-in-1 Sensor Parameters:
 * - Soil Moisture (%)
 * - Soil Temperature (°C)
 * - Soil EC (Electrical Conductivity) (μS/cm)
 * - Soil pH
 * - Soil Nitrogen (mg/kg)
 * - Soil Phosphorus (mg/kg)
 * - Soil Potassium (mg/kg)
 * - Soil Salinity (mg/L)
 *
 * Created: 2026-06-22
 */

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <ModbusMaster.h>

// ============ CONFIGURATION ============
// WiFi Credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// Server Configuration
const char* serverUrl = "http://192.168.1.100:3000/api/sensor-data";  // Ganti dengan IP server Anda

// RS485 Configuration
#define RX_PIN 16           // RS485 RO pin
#define TX_PIN 17           // RS485 DI pin
#define DE_RE_PIN 4         // RS485 DE/RE control pin (direction)

// Modbus Configuration
#define MODBUS_SLAVE_ID 1   // Default sensor address (check your sensor manual)
#define MODBUS_BAUDRATE 4800  // Common: 4800 or 9600 (check your sensor manual)

// Timing Configuration
const unsigned long SEND_INTERVAL = 10000;  // Send data every 10 seconds (sensor needs time)
const unsigned long MODBUS_TIMEOUT = 1000;  // Modbus response timeout

// Modbus Register Addresses (check your specific sensor manual)
// Typical 8-in-1 sensor register map:
#define REG_MOISTURE 0x0000      // Soil moisture (%)
#define REG_TEMPERATURE 0x0001   // Soil temperature (°C * 10)
#define REG_EC 0x0002            // Electrical conductivity (μS/cm)
#define REG_PH 0x0003            // pH (pH * 10)
#define REG_NITROGEN 0x0004      // Nitrogen (mg/kg)
#define REG_PHOSPHORUS 0x0005    // Phosphorus (mg/kg)
#define REG_POTASSIUM 0x0006     // Potassium (mg/kg)
#define REG_SALINITY 0x0007      // Salinity (mg/L)

// ============ GLOBAL OBJECTS ============
WiFiClient wifiClient;
HTTPClient http;
ModbusMaster modbus;

// Data storage
struct SensorData {
  float moisture;        // Soil moisture (%)
  float temperature;     // Soil temperature (°C)
  float ec;              // Electrical conductivity (μS/cm)
  float ph;              // pH value
  float nitrogen;        // Nitrogen (mg/kg or ppm)
  float phosphorus;      // Phosphorus (mg/kg or ppm)
  float potassium;       // Potassium (mg/kg or ppm)
  float salinity;        // Salinity (mg/L or ppm)
  unsigned long timestamp;
  bool valid;            // Reading success flag
};

SensorData currentData;
unsigned long lastSendTime = 0;

// ============ RS485 CONTROL FUNCTIONS ============
void preTransmission() {
  digitalWrite(DE_RE_PIN, HIGH);  // Enable transmit mode
}

void postTransmission() {
  digitalWrite(DE_RE_PIN, LOW);   // Enable receive mode
}

// ============ SETUP ============
void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n===========================================");
  Serial.println("ESP32 Soil Monitoring - 8-in-1 RS485 Sensor");
  Serial.println("===========================================\n");

  // Configure RS485 direction control pin
  pinMode(DE_RE_PIN, OUTPUT);
  digitalWrite(DE_RE_PIN, LOW);  // Start in receive mode
  Serial.println("✓ RS485 direction control configured");

  // Initialize Serial2 for RS485 communication
  Serial2.begin(MODBUS_BAUDRATE, SERIAL_8N1, RX_PIN, TX_PIN);
  Serial.print("✓ RS485 UART initialized at ");
  Serial.print(MODBUS_BAUDRATE);
  Serial.println(" baud");

  // Initialize Modbus
  modbus.begin(MODBUS_SLAVE_ID, Serial2);
  modbus.preTransmission(preTransmission);
  modbus.postTransmission(postTransmission);
  Serial.print("✓ Modbus initialized (Slave ID: ");
  Serial.print(MODBUS_SLAVE_ID);
  Serial.println(")");

  // Connect to WiFi
  connectWiFi();

  // Test sensor connection
  Serial.println("\nTesting sensor connection...");
  readSensors();
  if (currentData.valid) {
    Serial.println("✓ Sensor communication successful!\n");
  } else {
    Serial.println("⚠ Warning: Could not read from sensor!");
    Serial.println("Check wiring and sensor configuration.\n");
  }

  Serial.println("✓ System ready!");
  Serial.println("Starting sensor readings...\n");
}

// ============ MAIN LOOP ============
void loop() {
  // Check WiFi connection
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("⚠ WiFi disconnected! Reconnecting...");
    connectWiFi();
  }

  // Read sensors and send data at interval
  if (millis() - lastSendTime >= SEND_INTERVAL) {
    readSensors();

    if (currentData.valid) {
      sendDataToServer();
      printSensorData();
    } else {
      Serial.println("✗ Sensor reading failed - skipping this cycle\n");
    }

    lastSendTime = millis();
  }

  delay(100);  // Small delay to prevent watchdog issues
}

// ============ WiFi CONNECTION ============
void connectWiFi() {
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✓ WiFi Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Signal Strength (RSSI): ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
  } else {
    Serial.println("\n✗ WiFi Connection Failed!");
    Serial.println("Please check your credentials and try again.");
  }
}

// ============ SENSOR READING (MODBUS) ============
void readSensors() {
  currentData.valid = false;
  uint8_t result;
  uint16_t data;

  Serial.println("Reading from 8-in-1 sensor...");

  // Read Soil Moisture
  result = modbus.readHoldingRegisters(REG_MOISTURE, 1);
  if (result == modbus.ku8MBSuccess) {
    data = modbus.getResponseBuffer(0);
    currentData.moisture = data / 10.0;  // Usually stored as value * 10
  } else {
    Serial.print("✗ Failed to read moisture. Error: ");
    Serial.println(result, HEX);
    return;
  }
  delay(100);  // Delay between requests

  // Read Soil Temperature
  result = modbus.readHoldingRegisters(REG_TEMPERATURE, 1);
  if (result == modbus.ku8MBSuccess) {
    data = modbus.getResponseBuffer(0);
    currentData.temperature = data / 10.0;  // Usually stored as value * 10
  } else {
    Serial.print("✗ Failed to read temperature. Error: ");
    Serial.println(result, HEX);
    return;
  }
  delay(100);

  // Read Electrical Conductivity (EC)
  result = modbus.readHoldingRegisters(REG_EC, 1);
  if (result == modbus.ku8MBSuccess) {
    data = modbus.getResponseBuffer(0);
    currentData.ec = data;  // μS/cm
  } else {
    Serial.print("✗ Failed to read EC. Error: ");
    Serial.println(result, HEX);
    return;
  }
  delay(100);

  // Read pH
  result = modbus.readHoldingRegisters(REG_PH, 1);
  if (result == modbus.ku8MBSuccess) {
    data = modbus.getResponseBuffer(0);
    currentData.ph = data / 10.0;  // Usually stored as pH * 10
  } else {
    Serial.print("✗ Failed to read pH. Error: ");
    Serial.println(result, HEX);
    return;
  }
  delay(100);

  // Read Nitrogen (N)
  result = modbus.readHoldingRegisters(REG_NITROGEN, 1);
  if (result == modbus.ku8MBSuccess) {
    data = modbus.getResponseBuffer(0);
    currentData.nitrogen = data;  // mg/kg
  } else {
    Serial.print("✗ Failed to read nitrogen. Error: ");
    Serial.println(result, HEX);
    return;
  }
  delay(100);

  // Read Phosphorus (P)
  result = modbus.readHoldingRegisters(REG_PHOSPHORUS, 1);
  if (result == modbus.ku8MBSuccess) {
    data = modbus.getResponseBuffer(0);
    currentData.phosphorus = data;  // mg/kg
  } else {
    Serial.print("✗ Failed to read phosphorus. Error: ");
    Serial.println(result, HEX);
    return;
  }
  delay(100);

  // Read Potassium (K)
  result = modbus.readHoldingRegisters(REG_POTASSIUM, 1);
  if (result == modbus.ku8MBSuccess) {
    data = modbus.getResponseBuffer(0);
    currentData.potassium = data;  // mg/kg
  } else {
    Serial.print("✗ Failed to read potassium. Error: ");
    Serial.println(result, HEX);
    return;
  }
  delay(100);

  // Read Salinity
  result = modbus.readHoldingRegisters(REG_SALINITY, 1);
  if (result == modbus.ku8MBSuccess) {
    data = modbus.getResponseBuffer(0);
    currentData.salinity = data;  // mg/L
  } else {
    Serial.print("✗ Failed to read salinity. Error: ");
    Serial.println(result, HEX);
    return;
  }

  // Record timestamp
  currentData.timestamp = millis();
  currentData.valid = true;

  Serial.println("✓ All sensor values read successfully");
}

// ============ SEND DATA TO SERVER ============
void sendDataToServer() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("⚠ Cannot send data - WiFi not connected");
    return;
  }

  // Create JSON document (larger size for 8 parameters)
  StaticJsonDocument<512> doc;

  // Soil parameters
  doc["moisture"] = round(currentData.moisture * 10) / 10.0;
  doc["temperature"] = round(currentData.temperature * 10) / 10.0;
  doc["ec"] = round(currentData.ec);
  doc["ph"] = round(currentData.ph * 100) / 100.0;

  // NPK values
  doc["nitrogen"] = round(currentData.nitrogen);
  doc["phosphorus"] = round(currentData.phosphorus);
  doc["potassium"] = round(currentData.potassium);

  // Salinity
  doc["salinity"] = round(currentData.salinity);

  // Metadata
  doc["timestamp"] = currentData.timestamp;
  doc["device_id"] = "ESP32_8IN1_001";
  doc["sensor_type"] = "8-in-1-RS485";

  // Serialize JSON to string
  String jsonData;
  serializeJson(doc, jsonData);

  // Send HTTP POST request
  http.begin(wifiClient, serverUrl);
  http.addHeader("Content-Type", "application/json");

  int httpResponseCode = http.POST(jsonData);

  // Check response
  if (httpResponseCode > 0) {
    Serial.print("✓ Data sent successfully! Response code: ");
    Serial.println(httpResponseCode);

    if (httpResponseCode == 200) {
      String response = http.getString();
      Serial.print("Server response: ");
      Serial.println(response);
    }
  } else {
    Serial.print("✗ Error sending data. Error code: ");
    Serial.println(httpResponseCode);
    Serial.print("Error: ");
    Serial.println(http.errorToString(httpResponseCode));
  }

  http.end();
}

// ============ PRINT SENSOR DATA ============
void printSensorData() {
  Serial.println("\n========== Sensor Readings ==========");

  Serial.println("--- Soil Conditions ---");
  Serial.print("Moisture:       ");
  Serial.print(currentData.moisture, 1);
  Serial.println(" %");

  Serial.print("Temperature:    ");
  Serial.print(currentData.temperature, 1);
  Serial.println(" °C");

  Serial.print("EC:             ");
  Serial.print(currentData.ec, 0);
  Serial.println(" μS/cm");

  Serial.print("pH:             ");
  Serial.println(currentData.ph, 2);

  Serial.println("\n--- NPK Nutrients ---");
  Serial.print("Nitrogen (N):   ");
  Serial.print(currentData.nitrogen, 0);
  Serial.println(" mg/kg");

  Serial.print("Phosphorus (P): ");
  Serial.print(currentData.phosphorus, 0);
  Serial.println(" mg/kg");

  Serial.print("Potassium (K):  ");
  Serial.print(currentData.potassium, 0);
  Serial.println(" mg/kg");

  Serial.println("\n--- Other ---");
  Serial.print("Salinity:       ");
  Serial.print(currentData.salinity, 0);
  Serial.println(" mg/L");

  Serial.println("=====================================\n");
}
