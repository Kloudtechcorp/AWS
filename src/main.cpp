#include <Arduino.h>
#include <math.h>
#include <Wire.h>
#include <utilities.h>

void setup()
{
  // Start Serial Monitor
  SerialMon.begin(115200); delay(10); // feedback buffer
  GSMinit(); // Initialize GSM

  SerialMon.println("\n=================================== Sensors Status ===================================");
  Wire.begin(21, 22);

  collectTHP(); // Connect Temperature, Humidity, Pressure
  collectLight(); // Connect Light
  collectUV(); // Connect UV
  collectDirection(); // Connect Wind Direction
  collectSlave(); // Connect to Precipitation and Wind Speed
  collectBatteryV(); // Get Battery Voltage
}

void loop()
{
  connectAPN(); // APN Connect
  connectServer(); // Server Connect
  startSDCard(); // Start SD Card

  SerialMon.println("\n=================================== Print results ===================================");
  printResults();

  if (connectedAPN && connectedServer)
  {
    sendHTTPPostRequest();
    SerialMon.println("\n========================================Closing Client========================================");
    client.stop();
    SerialMon.println(F("Server disconnected"));
    modem.gprsDisconnect();
    SerialMon.println(F("GPRS disconnected"));
  }

  // Set Timer and Sleep
  SerialMon.print("\nSleeping...\n");
  int sleeptimer = 60000 - (millis() % 60000);
  esp_sleep_enable_timer_wakeup(sleeptimer * 1000);
  esp_deep_sleep_start();
}