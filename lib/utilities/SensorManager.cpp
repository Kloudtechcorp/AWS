#include <Adafruit_BME280.h>
#include <Adafruit_INA219.h>
#include <BH1750.h>
#include <Preferences.h>

#include "SensorManager.h"

#define SerialMon Serial

const uint8_t UV_PIN = 32;

const int BME_ADDR = 0x76;
const int BME_WIRE_ADDR = 0x70;
const int AS5600_ADDR = 0x36;
const int AS5600_ANGLE_REG = 0x0E;
const int SLAVE_ADDR = 0x08;

const int WIND_DIRECTION_BURST_SAMPLES = 30;
const int WIND_DIRECTION_SAMPLE_INTERVAL = 100;

const int RAIN_TIP_VALUE = 0.1099;
const float CALIBRATION_FACTOR = 2.4845;
const float RADIUS = 0.05;
const float CIRCUMFERENCE = 2 * PI * RADIUS * CALIBRATION_FACTOR;
const int PERIOD = 60;

Preferences preferences;

BH1750 lightMeter;

Adafruit_INA219 ina219;

float isCalibrated = false;
float northOffset = 0.0;
bool isDirectionReversed = false;

SensorManager::SensorManager()
{
}

#ifdef PTORIVAS_STATION
#include <Adafruit_BMP085.h>
#include <DHT.h>

const uint8_t DHT_PIN = 4;

Adafruit_BME280 bme;

Adafruit_BMP085 bmp;

DHT dht(4, DHT22);

void SensorManager::updateTemperatureHumidityPressure()
{
    // BME Connect
    SerialMon.print("BME280: ");
    if (!bme.begin(BME_ADDR))
    {
        SerialMon.println(" Failed");
    }
    else
    {
        SerialMon.println(" OK");

        _temperature1 = bme.readTemperature();
        _humidity1 = bme.readHumidity();
        _pressure1 = bme.readPressure() / 100.0F;
    }

    // BMP Connect
    SerialMon.print("BMP180: ");
    if (!bmp.begin())
    {
        SerialMon.println(" Failed");
    }
    else
    {
        SerialMon.println(" OK");
        _temperature2 = bmp.readTemperature();
        _pressure2 = bmp.readPressure() / 100.0F;
    }

    // DHT Connect
    SerialMon.print("DHT22: ");
    dht.begin();
    if (isnan(dht.readHumidity()))
    {
        SerialMon.println(" Failed");
    }
    else
    {
        SerialMon.println(" OK");
        _humidity2 = dht.readHumidity();
    }
    delay(10);
}

#else
Adafruit_BME280 bme1;
Adafruit_BME280 bme2;
Adafruit_BME280 bme3;

void selectBmeBus(uint8_t bus)
{
    Wire.beginTransmission(BME_WIRE_ADDR);
    Wire.write(1 << bus);
    Wire.endTransmission();
}

void updateTemperatureHumidityPressureInner(const char *name, Adafruit_BME280 bme, int bus, float *temp, float *hum, float *pres)
{
    // BME280 Connect
    SerialMon.printf("BME %s: \t\t", name);

    selectBmeBus(bus);

    if (!bme.begin(BME_ADDR))
    {
        SerialMon.println("Failed");
        return;
    }

    SerialMon.println("OK");
    *temp = bme.readTemperature();
    *hum = bme.readHumidity();
    *pres = bme.readPressure() / 100.0F;
}

void SensorManager::updateTemperatureHumidityPressure()
{
    updateTemperatureHumidityPressureInner("1", bme1, 2, &_temperature1, &_humidity1, &_pressure1);
    delay(10);

    updateTemperatureHumidityPressureInner("2", bme2, 3, &_temperature2, &_humidity2, &_pressure2);
    delay(10);

    updateTemperatureHumidityPressureInner("3", bme3, 4, &_temperature3, &_humidity3, &_pressure3);
}
#endif

void SensorManager::updateLux()
{
    SerialMon.print("BH1750: \t");

    if (!lightMeter.begin())
    {
        SerialMon.println(" Failed");
        return;
    }

    SerialMon.println(" OK");

    _lux = lightMeter.readLightLevel();
}

void SensorManager::updateUvIntensity()
{
    const int DEBOUNCE_THRESHOLD = 10;

    SerialMon.print("UV: \t\t");

    analogReadResolution(12);
    int sensorValue = analogRead(UV_PIN);

    if (abs(sensorValue - _uvPrevSensorValue) > DEBOUNCE_THRESHOLD)
    {
        _uvPrevSensorValue = sensorValue;
        delay(50);
        sensorValue = analogRead(UV_PIN);
    }

    if (abs(sensorValue - _uvPrevSensorValue) <= DEBOUNCE_THRESHOLD)
    {
        _uvPrevSensorValue = sensorValue;
        SerialMon.println("OK");

        float sensorVoltage = sensorValue * (3.3 / 4095.0);
        _uvIntensity = sensorVoltage * 1000;
    }
    else
    {
        SerialMon.println("Failed");
        return;
    }
}

float calculateCircularAverage(float *angles, int count)
{
    // Convert to unit vectors and average
    float sumX = 0, sumY = 0;

    for (int i = 0; i < count; i++)
    {
        float radians = angles[i] * PI / 180.0;
        sumX += cos(radians);
        sumY += sin(radians);
    }

    float avgX = sumX / count;
    float avgY = sumY / count;

    // Convert back to degrees
    float avgAngle = atan2(avgY, avgX) * 180.0 / PI;

    // Normalize to 0-360 range
    while (avgAngle < 0)
        avgAngle += 360.0;
    while (avgAngle >= 360.0)
        avgAngle -= 360.0;

    return avgAngle;
}

void loadCalibration()
{
    preferences.begin("windvane", false); // Open in read-only mode

    isCalibrated = preferences.getBool("calibrated", false);
    northOffset = preferences.getFloat("northOffset", 0.0);
    isDirectionReversed = preferences.getBool("reversed", false);

    preferences.end();
}

void saveCalibration()
{
    preferences.begin("windvane", false);

    preferences.putBool("calibrated", isCalibrated);
    preferences.putFloat("northOffset", northOffset);
    preferences.putBool("reversed", isDirectionReversed);

    preferences.end();
}

float readAngle()
{
    Wire.beginTransmission(AS5600_ADDR);
    Wire.write(AS5600_ANGLE_REG);
    byte error = Wire.endTransmission();

    if (error != 0)
    {
        return -1;
    }

    Wire.requestFrom(AS5600_ADDR, 2);

    if (Wire.available() >= 2)
    {
        byte highByte = Wire.read();
        byte lowByte = Wire.read();

        uint16_t rawValue = ((uint16_t)highByte << 8) | lowByte;
        float degrees = (rawValue * 360.0) / 4096.0;

        return degrees;
    }

    return -1;
}

float applyWindvaneCalibration(float rawAngle)
{
    float calibratedAngle = rawAngle;

    // Apply direction reversal if needed
    if (isDirectionReversed)
    {
        calibratedAngle = 360.0 - calibratedAngle;
    }

    // Apply north offset
    calibratedAngle = calibratedAngle - northOffset;

    // Normalize to 0-360 range
    while (calibratedAngle < 0)
        calibratedAngle += 360.0;
    while (calibratedAngle >= 360.0)
        calibratedAngle -= 360.0;

    return calibratedAngle;
}

void calibrateNorth()
{
    Serial.println("Point windvane to NORTH and press Enter...");

    // Clear serial buffer
    while (Serial.available())
        Serial.read();
    while (!Serial.available())
    {
        delay(100);
    }
    while (Serial.available())
        Serial.read();

    // Take multiple readings for accuracy
    float totalAngle = 0;
    int validReadings = 0;

    Serial.println("Calibrating...");
    for (int i = 0; i < 20; i++)
    {
        float angle = readAngle();
        if (angle >= 0)
        {
            totalAngle += angle;
            validReadings++;
        }
        delay(50);
    }

    if (validReadings > 10)
    {
        northOffset = totalAngle / validReadings;
        isCalibrated = true;

        // Save to preferences
        saveCalibration();

        Serial.println("North calibrated and saved!");
        Serial.print("North offset: ");
        Serial.print(northOffset, 2);
        Serial.println("°");
    }
    else
    {
        Serial.println("Calibration failed - try again");
    }
}

float performWindMeasurement()
{
    // Check for serial commands
    if (Serial.available())
    {
        char cmd = Serial.read();
        if (cmd == 'c' || cmd == 'C')
        {
            calibrateNorth();
        }
        else if (cmd == 'r' || cmd == 'R')
        {
            isDirectionReversed = !isDirectionReversed;
            saveCalibration();
            Serial.print("Direction reversed: ");
            Serial.println(isDirectionReversed ? "YES" : "NO");
        }
        else if (cmd == 't' || cmd == 'T')
        {
            // Test measurement now
            performWindMeasurement();
        }
    }

    float readings[WIND_DIRECTION_BURST_SAMPLES];
    int validReadings = 0;

    Serial.print("Collecting data");

    // Take burst of readings over 3 seconds
    for (int i = 0; i < WIND_DIRECTION_BURST_SAMPLES; i++)
    {
        float rawAngle = readAngle();

        if (rawAngle >= 0)
        {
            // Apply calibration immediately
            readings[validReadings] = applyWindvaneCalibration(rawAngle);
            validReadings++;
        }

        // Show progress
        if (i % 10 == 0)
            Serial.print(".");

        delay(WIND_DIRECTION_SAMPLE_INTERVAL);
    }

    Serial.println();

    if (validReadings < 15)
    { // Need at least half the samples
        Serial.println("ERROR: Insufficient valid readings");
        return NAN;
    }

    // Calculate statistics
    float windDirection = calculateCircularAverage(readings, validReadings);
    Serial.println();

    return windDirection;
}

void SensorManager::updateWindDirection()
{
    SerialMon.print("AS5600: \t");

    loadCalibration();

    Wire.beginTransmission(AS5600_ADDR);
    Wire.write(AS5600_ANGLE_REG);
    uint8_t error = Wire.endTransmission();

    if (error != 0)
    {
        SerialMon.println("Failed");
        return;
    }

    SerialMon.println("OK");

    _windDirection = performWindMeasurement();
}

void SensorManager::updateSlave()
{
    SerialMon.print("Slave: \t\t");

    Wire.beginTransmission(SLAVE_ADDR);
    if (Wire.endTransmission() != 0)
    {
        SerialMon.println("Failed");
        return;
    }

    SerialMon.println("OK");

    Wire.requestFrom(SLAVE_ADDR, 6);

    // Rain
    if (Wire.available() >= 4)
    {
        byte msb = Wire.read();
        byte lsb = Wire.read();
        uint16_t receivedRainCount = (msb << 8) | lsb;
        _rain = receivedRainCount * RAIN_TIP_VALUE;
    }

    // Wind speed
    if (Wire.available() >= 2)
    {
        byte msb = Wire.read();
        byte lsb = Wire.read();
        uint16_t receivedWindSpeedCount = (msb << 8) | lsb;
        _windSpeed = ((CIRCUMFERENCE * receivedWindSpeedCount * 3.6) / PERIOD);
    }

    // Gust
    if (Wire.available() >= 2)
    {
        byte msb = Wire.read();
        byte lsb = Wire.read();
        uint16_t receivedGustCount = (msb << 8) | lsb;
        _gust = ((CIRCUMFERENCE * receivedGustCount * 3.6) / 3);
    }
}

void SensorManager::updateBatteryVoltage()
{
    SerialMon.print("INA219: \t");

    if (!ina219.begin())
    {
        SerialMon.println("Failed");
        return;
    }

    SerialMon.println("OK");

    _batteryVoltage = ina219.getBusVoltage_V();
}
