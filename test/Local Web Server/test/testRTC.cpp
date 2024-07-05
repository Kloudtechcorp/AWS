#include <WiFi.h>
#include <WiFiClient.h>
#include <Wire.h>
#include <FS.h>
#include <SPI.h>
#include <Adafruit_I2CDevice.h>
#include "RtcDS3231.h"
RtcDS3231<TwoWire> Rtc(Wire);

#define countof(a) (sizeof(a) / sizeof(a[0]))
void printDateTime(const RtcDateTime& dt)
{
  char datestring[26];

  snprintf_P(datestring, 
          countof(datestring),
          PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
          dt.Month(),
          dt.Day(),
          dt.Year(),
          dt.Hour(),
          dt.Minute(),
          dt.Second() );
  Serial.println(datestring);
}

void setup() {
  Serial.begin(115200);
  Serial.println("");
  Serial.println("Setup started");

  // Initialize RTC
  Rtc.Begin();
  Serial.println("RTC initialized");
  Serial.println("Syncing Time to RTC...");
  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  printDateTime(compiled);
  Serial.println();
  Rtc.Enable32kHzPin(false);
  Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone); 
}

void loop() {
  RtcDateTime now = RtcDateTime(__DATE__, __TIME__);
  printDateTime(now);
  
  delay(10000);
}
