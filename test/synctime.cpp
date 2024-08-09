#include <RTClib.h>
#include <Wire.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <TimeLib.h>  
#include <SPI.h>


RTC_DS3231 rtc;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "asia.pool.ntp.org", 28800);


char t[32];
byte last_second, second_, minute_, hour_, day_, month_;
int year_;


void setup()
{
  Serial.begin(115200);  // Initialize serial communication with a baud rate of 9600
  Wire.begin();  // Begin I2C communication
  rtc.begin();  // Initialize DS3231 RTC module


  // Connect to WiFi
  char* ssid = "KT 2.4"; // Replace with your WiFi SSID
  char* password = "J@yGsumm!t"; // Replace with your WiFi password
  WiFi.begin(ssid, password);


  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");


  timeClient.begin();  // Start NTP client
  timeClient.update();  // Retrieve current epoch time from NTP server
  unsigned long unix_epoch = timeClient.getEpochTime();  // Get epoch time
  rtc.adjust(DateTime(unix_epoch));  // Set RTC time using NTP epoch time


  DateTime now = rtc.now();  // Get initial time from RTC
  last_second = now.second();  // Store initial second
}


void loop()
{
  
  timeClient.update();  // Update time from NTP server
  unsigned long unix_epoch = timeClient.getEpochTime();  // Get current epoch time


  second_ = second(unix_epoch);  // Extract second from epoch time
  if (last_second != second_)
  {
    minute_ = minute(unix_epoch);  // Extract minute from epoch time
    hour_ = hour(unix_epoch);  // Extract hour from epoch time
    day_ = day(unix_epoch);  // Extract day from epoch time
    month_ = month(unix_epoch);  // Extract month from epoch time
    year_ = year(unix_epoch);  // Extract year from epoch time


    // Format and print NTP time on Serial monitor
    sprintf(t, "NTP Time: %02d:%02d:%02d %02d/%02d/%02d", hour_, minute_, second_, day_, month_, year_);
    Serial.println(t);


    DateTime rtcTime = rtc.now();  // Get current time from RTC


    // Format and print RTC time on Serial monitor
    sprintf(t, "RTC Time: %02d:%02d:%02d %02d/%02d/%02d", rtcTime.hour(), rtcTime.minute(), rtcTime.second(), rtcTime.day(), rtcTime.month(), rtcTime.year());
    Serial.println(t);


    // Compare NTP time with RTC time
    if (rtcTime == DateTime(year_, month_, day_, hour_, minute_, second_))
    {
      Serial.println("Time is synchronized!");  // Print synchronization status
    }
    else
    {
      Serial.println("Time is not synchronized!");  // Print synchronization status
    }


    last_second = second_;  // Update last second
  }


  delay(1000);  // Delay for 1 second before the next iteration
}