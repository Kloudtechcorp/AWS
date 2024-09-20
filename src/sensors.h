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
uint16_t receivedRainCount;

// Wind Speed and Gust var
float windspeed;
int REV, radius = 50, period = 60;
uint16_t receivedWindCount;
float gust;
uint16_t receivedGustCount;

// Battery Voltage Library and Variable
#include <Adafruit_INA219.h>
Adafruit_INA219 ina219;