#ifndef SENSORS_H
#define SENSORS_H

// Serial Monitors
#define SerialMon Serial

// String Parameters
String t1Str = "";
String h1Str = "";
String p1Str = "";
String t2Str = "";
String h2Str = "";
String p2Str = "";
String t3Str = "";
String h3Str = "";
String p3Str = "";
String lightStr = "";
String uvIntensityStr = "";
String windDirStr = "";
String windSpeedStr = "";
String rainStr = "";
String gustStr = "";
String batteryStr = "";
String communication = "";

// BME280 Library and Variables
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
Adafruit_BME280 bme1;
Adafruit_BME280 bme2;
Adafruit_BME280 bme3;
float t1, h1, p1, t2, h2, p2, t3, h3, p3;

// AS5600 Variables
#include <Preferences.h>
#define AS5600_ADDRESS 0x36
#define AS5600_ANGLE_REG 0x0E
Preferences preferences;
float northOffset = 0.0;
bool isCalibrated = false;
bool reverseDirection = false;
float avgAngle = 0.0;
float degrees = 0.0;
float calibratedAngle = 0.0;
float windDirection;
// Sampling Configuration for Wind Direction
#define BURST_SAMPLES 30
#define SAMPLE_INTERVAL 100

// UV Variables
#define uvPin 32
float sensorVoltage, sensorValue;
int uvIntensity;

// BH1750 Library and Variables
#include <BH1750.h>
BH1750 lightMeter;
float lux, irradiance;

// Slave Address
#define SLAVE 0x03

// Rain Gauge
float tipValue = 0.1099, rain;
uint16_t receivedRainCount = 0;

// Wind Speed and Gust var
float windspeed, circumference, calibrationFactor = 2.4845, radius = 0.05;
int period = 60;
uint16_t receivedWindCount = 0;
float gust;
uint16_t receivedGustCount = 0;

// Battery Voltage Library and Variable
#include <Adafruit_INA219.h>
Adafruit_INA219 ina219;

void select_bus(uint8_t bus) {
  Wire.beginTransmission(0x70);
  Wire.write(1 << bus);
  Wire.endTransmission();
}

void getBME(Adafruit_BME280 bme, int bus, float *temp, float *hum, float *pres) {
  select_bus(bus);
  *temp = bme.readTemperature();
  *hum = bme.readHumidity();
  *pres = bme.readPressure() / 100.0F;
}

String getUV() {
  analogReadResolution(12);
  sensorValue = analogRead(uvPin);
  sensorVoltage = sensorValue * (3.3 / 4095.0);
  uvIntensity = sensorVoltage * 1000;
  uvIntensityStr = String(uvIntensity);
  return uvIntensityStr;
}

String getLight() {
  lux = lightMeter.readLightLevel();
  lightStr = String(lux);
  return lightStr;
}

float calculateCircularAverage(float* angles, int count) {
  if (count == 0) return 0;
  
  // Convert to unit vectors and average
  float sumX = 0, sumY = 0;
  
  for (int i = 0; i < count; i++) {
    float radians = angles[i] * PI / 180.0;
    sumX += cos(radians);
    sumY += sin(radians);
  }
  
  float avgX = sumX / count;
  float avgY = sumY / count;
  
  // Convert back to degrees
  avgAngle = atan2(avgY, avgX) * 180.0 / PI;
  
  // Normalize to 0-360 range
  while (avgAngle < 0) avgAngle += 360.0;
  while (avgAngle >= 360.0) avgAngle -= 360.0;
  
  return avgAngle;
}

void loadCalibration() {
  preferences.begin("windvane", false); // Open in read-only mode
  
  isCalibrated = preferences.getBool("calibrated", false);
  northOffset = preferences.getFloat("northOffset", 0.0);
  reverseDirection = preferences.getBool("reversed", false);
  
  preferences.end();
}

void saveCalibration() {
  preferences.begin("windvane", false);
  
  preferences.putBool("calibrated", isCalibrated);
  preferences.putFloat("northOffset", northOffset);
  preferences.putBool("reversed", reverseDirection);
  
  preferences.end();
}

float readAngle() {
  Wire.beginTransmission(AS5600_ADDRESS);
  Wire.write(AS5600_ANGLE_REG);
  byte error = Wire.endTransmission();

  if (error != 0) {
      return -1;
  }

  Wire.requestFrom(AS5600_ADDRESS, 2);

  if (Wire.available() >= 2) {
  byte highByte = Wire.read();
  byte lowByte = Wire.read();

  uint16_t rawValue = ((uint16_t)highByte << 8) | lowByte;
  degrees = (rawValue * 360.0) / 4096.0;

  return degrees;
  }

  return -1;
}

float applyWindvaneCalibration(float rawAngle) {
  calibratedAngle = rawAngle;

  // Apply direction reversal if needed
  if (reverseDirection) {
  calibratedAngle = 360.0 - calibratedAngle;
  }

  // Apply north offset
  calibratedAngle = calibratedAngle - northOffset;

  // Normalize to 0-360 range
  while (calibratedAngle < 0) calibratedAngle += 360.0;
  while (calibratedAngle >= 360.0) calibratedAngle -= 360.0;

  return calibratedAngle;
}

void calibrateNorth() {
  Serial.println("Point windvane to NORTH and press Enter...");
  
  // Clear serial buffer
  while (Serial.available()) Serial.read();
  while (!Serial.available()) {
    delay(100);
  }
  while (Serial.available()) Serial.read();
  
  // Take multiple readings for accuracy
  float totalAngle = 0;
  int validReadings = 0;
  
  Serial.println("Calibrating...");
  for (int i = 0; i < 20; i++) {
    float angle = readAngle();
    if (angle >= 0) {
      totalAngle += angle;
      validReadings++;
    }
    delay(50);
  }
  
  if (validReadings > 10) {
    northOffset = totalAngle / validReadings;
    isCalibrated = true;
    
    // Save to preferences
    saveCalibration();
    
    Serial.println("North calibrated and saved!");
    Serial.print("North offset: ");
    Serial.print(northOffset, 2);
    Serial.println("°");
  } else {
    Serial.println("Calibration failed - try again");
  }
}

float performWindMeasurement() {
  // Check for serial commands
  if (Serial.available()) {
    char cmd = Serial.read();
    if (cmd == 'c' || cmd == 'C') {
      calibrateNorth();
    } else if (cmd == 'r' || cmd == 'R') {
      reverseDirection = !reverseDirection;
      saveCalibration();
      Serial.print("Direction reversed: ");
      Serial.println(reverseDirection ? "YES" : "NO");
    } else if (cmd == 't' || cmd == 'T') {
      // Test measurement now
      performWindMeasurement();
    }
  }

  float readings[BURST_SAMPLES];
  int validReadings = 0;
  
  Serial.print("Collecting data");
  
  // Take burst of readings over 3 seconds
  for (int i = 0; i < BURST_SAMPLES; i++) {
    float rawAngle = readAngle();
    
    if (rawAngle >= 0) {
      // Apply calibration immediately
      readings[validReadings] = applyWindvaneCalibration(rawAngle);
      validReadings++;
    }
    
    // Show progress
    if (i % 10 == 0) Serial.print(".");
    
    delay(SAMPLE_INTERVAL);
  }
  
  Serial.println();
  
  if (validReadings < 15) { // Need at least half the samples
    Serial.println("ERROR: Insufficient valid readings");
    return NAN;
  }
  
  // Calculate statistics
  windDirection = calculateCircularAverage(readings, validReadings);
  Serial.println();

  return windDirection;
}

String getDirection() {
  performWindMeasurement();
  windDirStr = String(windDirection);
  return windDirStr;
}

void getSlave() {
  Wire.requestFrom(SLAVE, 6);

  // Rain
  if (Wire.available() >= 4) {
    byte msb = Wire.read();
    byte lsb = Wire.read();
    receivedRainCount = (msb << 8) | lsb;
  }
  rain = receivedRainCount * tipValue;

  // Wind speed
  if (Wire.available() >= 2) {
    byte msb = Wire.read();
    byte lsb = Wire.read();
    receivedWindCount = (msb << 8) | lsb;
  }
  circumference = 2 * PI * radius * calibrationFactor;
  windspeed = ((circumference * receivedWindCount * 3.6) / period);

  // Gust
  if (Wire.available() >= 2) {
    byte msb = Wire.read();
    byte lsb = Wire.read();
    receivedGustCount = (msb << 8) | lsb;
  }
  gust = ((circumference * receivedGustCount * 3.6) / 3);
}

String getBatteryVoltage() {
  batteryStr = String(ina219.getBusVoltage_V());
  return batteryStr;
}

void collectTHP() {
  // BME 1 Connect
  SerialMon.print("BME 1: ");
  select_bus(2);
  if (!bme1.begin(0x76)) { SerialMon.println(" Failed"); }
  else {
    SerialMon.println(" OK");
    getBME(bme1, 2, &t1, &h1, &p1);
    t1Str = String(t1);
    h1Str = String(h1);
    p1Str = String(p1);
  }
  delay(10);
  // BME 2 Connect
  SerialMon.print("BME 2: ");
  select_bus(3);
  if (!bme2.begin(0x76)) { SerialMon.println(" Failed"); }
  else {
    SerialMon.println(" OK");
    getBME(bme2, 3, &t2, &h2, &p2);
    t2Str = String(t2);
    h2Str = String(h2);
    p2Str = String(p2);
  }
  delay(10);
  // BME 3 Connect
  SerialMon.print("BME 3: ");
  select_bus(4);
  if (!bme3.begin(0x76)) { SerialMon.println(" Failed"); }
  else {
    SerialMon.println(" OK");
    getBME(bme3, 4, &t3, &h3, &p3);
    t3Str = String(t3);
    h3Str = String(h3);
    p3Str = String(p3);
  }
  delay(10);
}

void collectLight() {
  SerialMon.print("BH1750: ");
  if (!lightMeter.begin()) { SerialMon.println(" Failed"); }
  else {
    SerialMon.println(" OK");
    getLight();
  }
  delay(10);
}

void collectUV() {
  // UV Connect
  const int debounceThreshold = 10;
  static int uvPrevStatus = 0;
  SerialMon.print("UV: ");
  int uvStatus = analogRead(uvPin);
  if (abs(uvStatus - uvPrevStatus) > debounceThreshold) {
    uvPrevStatus = uvStatus;
    delay(50);
    uvStatus = analogRead(uvPin);
  }

  if (abs(uvStatus - uvPrevStatus) <= debounceThreshold) { 
    uvPrevStatus = uvStatus;
    SerialMon.println(" OK");
    getUV();
  }
  else { SerialMon.println(" Failed"); }
  delay(10);
}

void collectDirection() {
  SerialMon.print("AS5600: ");
  loadCalibration();
  Wire.beginTransmission(AS5600_ADDRESS);
  Wire.write(AS5600_ANGLE_REG);
  byte error = Wire.endTransmission();
  if (error != 0) { SerialMon.println(" Failed"); }
  else {
    SerialMon.println(" OK");
    getDirection();
  }
}

void collectSlave() {
  SerialMon.print("Slave: ");
  Wire.beginTransmission(SLAVE);
  if (!Wire.endTransmission() == 0) { SerialMon.println(" Failed"); }
  else {
    SerialMon.println(" OK");
    getSlave();
    windSpeedStr = String(windspeed);
    rainStr = String(rain);
    gustStr = String(gust);
  }
  delay(10);
}

void collectBatteryV() {
  SerialMon.print("INA219: ");
  if (!ina219.begin()) { SerialMon.println(" Failed"); }
  else {
    SerialMon.println(" OK");
    getBatteryVoltage();
  }
  delay(10);
}

#endif