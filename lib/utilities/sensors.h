#ifndef SENSORS_H
#define SENSORS_H

// Serial Monitors
#define SerialMon Serial

// String Parameters
String t1Str = "";
String h1Str = "";
String t2Str = "";
String h2Str = "";
String communication = "";

// BME280 Library and Variables
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
Adafruit_BME280 bme;

// SHT31 Library and Variables
#include "SHT31.h"
#define SHT31_ADDRESS   0x44
SHT31 sht;
uint32_t start;
uint32_t stop;

void collectBME() {
  SerialMon.print("BME : ");
  if (!bme.begin(0x76)) { SerialMon.println(" Failed"); }
  else {
    SerialMon.println(" OK");
    t1Str = String(bme.readTemperature());
    h1Str = String(bme.readHumidity());
  }
  delay(10);
}

void collectSHT() {
  SerialMon.print("SHT : ");
  if (!sht.begin()) { SerialMon.println(" Failed"); }
  else {
    SerialMon.println(" OK");
    start = micros();
    sht.read();         //  default = true/fast       slow = false
    stop = micros();
    t2Str = String(sht.getTemperature());
    h2Str = String(sht.getHumidity());
  }
  delay(10);
}
#endif