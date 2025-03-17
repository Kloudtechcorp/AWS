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
int magnetStatus, lowbyte, rawAngle, correctedAngle;
word highbyte;
float degAngle, startAngle;
RTC_DATA_ATTR float rtcStartAngle;
RTC_DATA_ATTR int rtcCorrectAngle;

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

void ReadRawAngle() {
  Wire.beginTransmission(0x36);
  Wire.write(0x0D);
  Wire.endTransmission();
  Wire.requestFrom(0x36, 1);
  lowbyte = Wire.read();

  Wire.beginTransmission(0x36);
  Wire.write(0x0C);
  Wire.endTransmission();
  Wire.requestFrom(0x36, 1);
  highbyte = Wire.read();

  highbyte = highbyte << 8;
  rawAngle = highbyte | lowbyte;
  degAngle = rawAngle * 0.087890625;
}

void correctAngle() {
  correctedAngle = 360 - degAngle + startAngle;
  if (correctedAngle > 360) { correctedAngle -= 360; }
  rtcCorrectAngle = correctedAngle;
  if (correctedAngle == 360) { correctedAngle = 0; }
}

String getDirection() {
  ReadRawAngle();
  correctAngle();
  windDirStr = String(correctedAngle);
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
  startAngle - rtcStartAngle;
  ReadRawAngle();
  if (rtcStartAngle == 0) { rtcStartAngle = degAngle; }
  startAngle = rtcStartAngle;
  correctedAngle = rtcCorrectAngle;
  Wire.beginTransmission(0x36);
  if (!Wire.endTransmission() == 0) { SerialMon.println(" Failed"); }
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