#include <Arduino.h>
#include <math.h>
#include <Wire.h>
#include <axe.h>

void setup()
{
  SerialMon.begin(115200); delay(10);
  SerialMon.println("Station Name: " + stationName);
  SerialMon.println("Code version: " + versionCode);
  GSMinit();

  SerialMon.println("\n=================================== Sensors Status ===================================");
  Wire.begin(21, 22);

  collectTHP();
  collectLight();
  collectUV();
  collectDirection();
  collectSlave();
  collectBatteryV();
}

void loop()
{
  connectAPN();
  connectServer();
  startSDCard();

  SerialMon.println("\n=================================== Print results ===================================");
  printResults();

  if (connectedAPN && connectedServer) {
    sendHTTPPostRequest();
    SerialMon.println("\n========================================Closing Client========================================");
    client.stop();
    SerialMon.println(F("Server disconnected"));
    modem.gprsDisconnect();
    SerialMon.println(F("GPRS disconnected"));
  }
  SerialMon.print("\nSleeping...\n");
  int sleeptimer = 60000 - (millis() % 60000);
  esp_sleep_enable_timer_wakeup(sleeptimer * 1000);
  esp_deep_sleep_start();
}