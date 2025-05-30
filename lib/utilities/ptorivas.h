#ifndef PTORIVAS_H
#define PTORIVAS_H

// ESP32 Serial Monitor
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
Adafruit_BME280 bme;
float t1, h1, p1;

// DHT22 Library and Variables
#include <DHT.h>
#define DHTPIN 04
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
float h2;

// BMP180 Library and Variables
#include <Adafruit_BMP085.h>
Adafruit_BMP085 bmp;
float t2, p2;

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
float lux;

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

void getBME() {
    t1 = bme.readTemperature();
    h1 = bme.readHumidity();
    p1 = bme.readPressure() / 100.0F;
    t1Str = String(t1);
    h1Str = String(h1);
    p1Str = String(p1);
}

String getDHT() {
    h2 = dht.readHumidity();
    h2Str = String(h2);
    return h2Str;
}

void getBMP() {
    t2 = bmp.readTemperature();
    p2 = bmp.readPressure() / 100.0F;
    t2Str = String(t2);
    p2Str = String(p2);
}

String getLight() {
  lux = lightMeter.readLightLevel();
  lightStr = String(lux);
  return lightStr;
}

String getUV() {
  analogReadResolution(12);
  sensorValue = analogRead(uvPin);
  sensorVoltage = sensorValue * (3.3 / 4095);
  uvIntensity = sensorVoltage * 1000;
  uvIntensityStr = String(uvIntensity);
  return uvIntensityStr;
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
    // BME Connect
    SerialMon.print("BME280: ");
    if (!bme.begin(0x76)) { SerialMon.println(" Failed"); }
    else {
        SerialMon.println(" OK");
        getBME();
    }
    // BMP Connect
    SerialMon.print("BMP180: ");
    if (!bmp.begin()) { SerialMon.println(" Failed"); }
    else {
        SerialMon.println(" OK");
        getBMP();
    }
    // DHT Connect
    SerialMon.print("DHT22: ");
    dht.begin();
    if (isnan(dht.readHumidity())) { SerialMon.println(" Failed"); }
    else {
        SerialMon.println(" OK");
        getDHT();
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

  if (abs(uvStatus- uvPrevStatus) <= debounceThreshold) { 
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