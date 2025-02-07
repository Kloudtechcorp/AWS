#include <Arduino.h>
#include <math.h>
#include <Wire.h>

#define DEMO_STATION       // Choose a location/station
#define USE_GSM         // Choose between GSM or WiFi (Uncomment to use GSM Mode)
// Network Credentials
#ifdef USE_GSM
  #define SMART_APN       
#else
  #define WIFI_1
#endif
#include <deviceConfig.h>

void setup()
{
  SerialMon.begin(115200); delay(10);
  SerialMon.println("\nStation Name: " + stationName);
  #ifdef USE_GSM
    GSMinit();
  #else
    connectWiFi();
    timeClient.begin();
    timeClient.update();
  #endif

  SerialMon.println("\n=================================== Sensors Status ===================================");
  Wire.begin(21, 22);
  getTime();
  SerialMon.println("Date and Time: " + dateTime);
  collectTHP();
  collectLight();
  collectUV();
  collectDirection();
  collectSlave();
  collectBatteryV();
}

void loop()
{
  startSDCard();

  #ifdef USE_GSM
    connectAPN();
    connectServer();
    if (connectedAPN && connectedServer) {
      SerialMon.println("\n=================================== Print results ===================================");
      printResults();
      sendHTTPPostRequest();
      SerialMon.println("\n========================================Closing Client========================================");
      client.stop();
      SerialMon.println(F("Server disconnected"));
      modem.gprsDisconnect();
      SerialMon.println(F("GPRS disconnected"));
    }
  #else
    SerialMon.println("\n=================================== Print results ===================================");
    printResults();
    sendDataToServer();
    SerialMon.println("\n========================================Closing Client========================================");
    client.stop();
    SerialMon.println(F("Server disconnected"));
  #endif
  
  SerialMon.print("\nSleeping...\n");
  int sleeptimer = 60000 - (millis() % 60000);
  esp_sleep_enable_timer_wakeup(sleeptimer * 1000);
  esp_deep_sleep_start();
}