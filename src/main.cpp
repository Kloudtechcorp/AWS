#include <Arduino.h> 
#include <math.h>
#include <Wire.h>

#define DEMO_MARIA_STATION      // Choose a location/station
// #define USE_GSM         // Choose between GSM or WiFi (Uncomment to use GSM Mode)
// Network Credentials
#ifdef USE_GSM
  #define SMART_APN       
#else
  #define WIFI_1
#endif
#include <deviceConfig.h>

void setup()
{
  SerialMon.begin(115200); Wire.begin(21, 22); delay(10);
  SerialMon.println("\nStation Name: " + stationName);
  #ifdef USE_GSM
    GSMinit();
    connectAPN();
    connectServer();
  #else
    connectWiFi();
    timeClient.begin();
    timeClient.update();
  #endif
}

void loop()
{
  SerialMon.println("\n=================================== Sensors Status ===================================");
  getTime();
  SerialMon.println("Date and Time: " + dateTime);
  collectTHP();
  collectLight();
  collectUV();
  collectDirection();
  collectSlave();
  collectBatteryV();
  startSDCard();

  #ifdef USE_GSM
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
    if (connectedWifi) {
      SerialMon.println("\n=================================== Print results ===================================");
      printResults();
      sendDataToServer();
      SerialMon.println("\n========================================Closing Client========================================");
      client.stop();
      SerialMon.println(F("Server disconnected"));
    }
  #endif
  
  SerialMon.print("\nSleeping...\n");
  int sleeptimer = 60000 - (millis() % 60000);
  esp_sleep_enable_timer_wakeup(sleeptimer * 1000);
  esp_deep_sleep_start();
}

// Void loop() with conditional sleep
// void loop()
// {
//   SerialMon.println("\n=================================== Sensors Status ===================================");
//   getTime();
//   SerialMon.println("Date and Time: " + dateTime);
//   collectTHP();
//   collectLight();
//   collectUV();
//   collectDirection();
//   collectSlave();
//   collectBatteryV();
//   startSDCard();

//   #ifdef USE_GSM
//     if (!modem.gprsConnect(deviceApn) && !base_client.connect(server, port)) {
//       connectedAPN = false;
//       connectedServer = false;
//       SerialMon.print("\nSleeping...\n");
//       int sleeptimer = 60000 - (millis() % 60000);
//       esp_sleep_enable_timer_wakeup(sleeptimer * 1000);
//       esp_deep_sleep_start();
//     }
//     if (connectedAPN && connectedServer) {
//       SerialMon.println("\n=================================== Print results ===================================");
//       printResults();
//       sendHTTPPostRequest();
//       SerialMon.println("\n========================================Closing Client========================================");
//       client.stop();
//       SerialMon.println(F("Server disconnected"));
//       modem.gprsDisconnect();
//       SerialMon.println(F("GPRS disconnected"));
//       int delayTimer = 60000 - (millis() % 60000);
//       delay(delayTimer);
//     }
//   #else
//     if (WiFi.status() != WL_CONNECTED) {
//       connectedWifi = false;
//       SerialMon.print("\nSleeping...\n");
//       int sleeptimer = 60000 - (millis() % 60000);
//       esp_sleep_enable_timer_wakeup(sleeptimer * 1000);
//       esp_deep_sleep_start();
//     }
//     else {
//       connectedWifi = true;
//       communication = "Success";
//     }
//     if (connectedWifi) {
//       SerialMon.println("\n=================================== Print results ===================================");
//       printResults();
//       sendDataToServer();
//       SerialMon.println("\n========================================Closing Client========================================");
//       client.stop();
//       SerialMon.println(F("Server disconnected"));
//       int delayTimer = 60000 - (millis() % 60000);
//       delay(delayTimer);
//     }
//   #endif
  
// }