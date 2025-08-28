#define LIMAY_STATION // Choose a location/station
#define USE_GSM            // Choose between GSM or WiFi (Uncomment to use GSM Mode)

#include <Arduino.h>
#include <Wire.h>
#include "esp_task_wdt.h"

#ifdef USE_GSM
#define SMART_APN
#else
#define WIFI_1
#endif

#include "SdCard.h"
#include "SensorManager.h"
#include "deviceConfig.h"

#ifdef USE_GSM
#include "GsmClient.h"
GsmClient commClient(deviceApn, deviceSerial.c_str());
#else
#include "WifiClient.h"
WifiClient commClient(ssid, password, deviceSerial.c_str());
#endif

#define SerialMon Serial

void setup()
{
    SerialMon.begin(115200);
    delay(10);
    
    // Configure watchdog timer (30 second timeout for sensor operations)
    esp_task_wdt_init(30, true);
    esp_task_wdt_add(NULL); // Add current task to watchdog
    SerialMon.println("Watchdog timer configured (30s timeout)");

    Wire.begin(21, 22);
    Wire.setTimeOut(20);
    Wire.setClock(100000);
    
    SerialMon.println("\n=================================================");
    SerialMon.println("Station Name: " + stationName);

    commClient.connect();
    
    // Reset watchdog after setup
    esp_task_wdt_reset();
}

void loop()
{
    commClient.updateDateTime();
    DateTime dateTime = commClient.getDateTime();
    String dateTimeStr = dateTime.toString();
    SerialMon.print("Date and Time: " + dateTimeStr);

    SensorManager sensorManager;
    sensorManager.updateAll();
    
    // Feed watchdog after sensor readings
    esp_task_wdt_reset();

    String t1Str = String(sensorManager.getTemperature1());
    String t2Str = String(sensorManager.getTemperature2());
    String t3Str = String(sensorManager.getTemperature3());
    String h1Str = String(sensorManager.getHumidity1());
    String h2Str = String(sensorManager.getHumidity2());
    String h3Str = String(sensorManager.getHumidity3());
    String p1Str = String(sensorManager.getPressure1());
    String p2Str = String(sensorManager.getPressure2());
    String p3Str = String(sensorManager.getPressure3());
    String lightStr = String(sensorManager.getLux());
    String uvIntensityStr = String(sensorManager.getUvIntensity());
    String windDirStr = String(sensorManager.getWindDirection());
    String windSpeedStr = String(sensorManager.getWindSpeed());
    String rainStr = String(sensorManager.getRain());
    String gustStr = String(sensorManager.getGust());
    String batteryStr = String(sensorManager.getBatteryVoltage());
    String communication = commClient.getCommunicationStatus();

    char sdCardData[256];
    sprintf(sdCardData, ", %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s",
          t1Str.c_str(), h1Str.c_str(), p1Str.c_str(), t2Str.c_str(), h2Str.c_str(), p2Str.c_str(),
          t3Str.c_str(), h3Str.c_str(), p3Str.c_str(), windDirStr.c_str(), lightStr.c_str(),
          uvIntensityStr.c_str(), rainStr.c_str(), windSpeedStr.c_str(), communication.c_str());

    SdCard sdCard;
    sdCard.logData(dateTime.toFileName(), "Date, Temperature 1, Humidity 1, Pressure 1, Temperature 2, Humidity 2, Pressure 2, Temperature 3, Humidity 3, Pressure 3, Wind Direction, Light Intensity, UV Intensity, Precipitation, Wind Speed, Communication", sdCardData);

    if (commClient.isConnected())
    {
        SerialMon.println("=================================================");
        SerialMon.println("Sending Data to Server...");
        SerialMon.println("T1 = " + t1Str);
        SerialMon.println("T2 = " + t2Str);
        SerialMon.println("T3 = " + t3Str);
        SerialMon.println("H1 = " + h1Str);
        SerialMon.println("H2 = " + h2Str);
        SerialMon.println("H3 = " + h3Str);
        SerialMon.println("P1 = " + p1Str);
        SerialMon.println("P2 = " + p2Str);
        SerialMon.println("P3 = " + p3Str);
        SerialMon.println("Light Intensity = " + lightStr);
        SerialMon.println("UV Intensity = " + uvIntensityStr);
        SerialMon.println("Wind Direction = " + windDirStr);
        SerialMon.println("Wind Speed = " + windSpeedStr);
        SerialMon.println("Rain = " + rainStr);
        SerialMon.println("Gust = " + gustStr);
        SerialMon.println("Battery Voltage = " + batteryStr);
        SerialMon.println("Time: " + dateTimeStr);
        SerialMon.println("Communication Status: " + communication);

        String postData = "{\"recordedAt\":\"" + dateTimeStr + "\", \"light\":\"" + lightStr + "\", \"uvIntensity\":\"" + uvIntensityStr + "\", \"windDirection\":\"" + windDirStr + "\", \"windSpeed\":\"" + windSpeedStr + "\", \"precipitation\":\"" + rainStr + "\", \"gust\":\"" + gustStr + "\", \"T1\":\"" + t1Str + "\", \"T2\":\"" + t2Str + "\", \"T3\":\"" + t3Str + "\", \"H1\":\"" + h1Str + "\", \"H2\":\"" + h2Str + "\", \"H3\":\"" + h3Str + "\", \"P1\":\"" + p1Str + "\", \"P2\":\"" + p2Str + "\", \"P3\":\"" + p3Str + "\", \"batteryVoltage\":\"" + batteryStr + "\"}";
        commClient.sendData(postData);
        commClient.disconnect();
        
        // Feed watchdog after communication
        esp_task_wdt_reset();
    }

    SerialMon.println("Sleeping...");
    SerialMon.println("=================================================\n");
    
    // Remove task from watchdog before deep sleep
    esp_task_wdt_delete(NULL);
    
    int sleeptimer = 60000 - (millis() % 60000);
    esp_sleep_enable_timer_wakeup(sleeptimer * 1000);
    esp_deep_sleep_start();
}
