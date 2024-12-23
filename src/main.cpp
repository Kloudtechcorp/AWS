#include <Arduino.h>
#include <math.h>
#include <Wire.h>

/*
  Change whether what station to use
  1. sabang.h     -     Sabang Fish Landing
  2. bpsubagac.h  -     BPSU Bagac Campus
  3. veles.h      -     Mariveles Municipal Hall
  4. limay.h      -     Limay Physical Therapy Center
  5. papermill.h  -     Bataan 2020
  6. hermosa.h    -     Hermosa Water District
  7. cabcaben.h   -     Old Cabcaben Pier
  8. quinawan.h   -     Quinawan Integrated School
  9. kanawan.h    -     Kanawan Integrated School
  10. tanato.h    -     Tanato Elementary School
  11. payangan.h  -     Payangan Elementary School
  12. pagasa.h    -     Pag-asa Elementary School
*/
#include <testWiFi.h>

// void setup()
// {
//   SerialMon.begin(115200); delay(10);
//   SerialMon.println("Station Name: " + stationName);
//   SerialMon.println("Code version: " + versionCode);
//   GSMinit();

//   SerialMon.println("\n=================================== Sensors Status ===================================");
//   Wire.begin(21, 22);
//   getTime();
//   SerialMon.println("Date and Time: " + dateTime);
//   collectTHP();
//   collectLight();
//   collectUV();
//   collectDirection();
//   collectSlave();
//   collectBatteryV();
// }

// void loop()
// {
//   connectAPN();
//   connectServer();
//   startSDCard();
  
//   SerialMon.println("\n=================================== Print results ===================================");
//   printResults();

//   if (connectedAPN && connectedServer) {
//     sendHTTPPostRequest();
//     SerialMon.println("\n========================================Closing Client========================================");
//     client.stop();
//     SerialMon.println(F("Server disconnected"));
//     modem.gprsDisconnect();
//     SerialMon.println(F("GPRS disconnected"));
//   }
//   SerialMon.print("\nSleeping...\n");
//   int sleeptimer = 60000 - (millis() % 60000);
//   esp_sleep_enable_timer_wakeup(sleeptimer * 1000);
//   esp_deep_sleep_start();
// }

void setup()
{
  SerialMon.begin(115200); delay(10);
  SerialMon.println("Station Name: " + stationName);
  SerialMon.println("Code version: " + versionCode);
  connectWiFi();
  timeClient.begin();
  timeClient.update();
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
  connectServer();
  startSDCard();
  
  SerialMon.println("\n=================================== Print results ===================================");
  printResults();

  sendDataToServer();
  SerialMon.println("\n========================================Closing Client========================================");
  client.stop();
  SerialMon.println(F("Server disconnected"));

  SerialMon.print("\nSleeping...\n");
  int sleeptimer = 60000 - (millis() % 60000);
  esp_sleep_enable_timer_wakeup(sleeptimer * 1000);
  esp_deep_sleep_start();
}