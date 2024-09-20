#include <Arduino.h>

// Serial Monitors
#define SerialMon Serial

// GSM Pins - GSM Intiation
#define GSM_PIN "0000"
#define UART_BAUD 115200
#define PIN_DTR 25
#define PIN_TX 26
#define PIN_RX 27
#define PWR_PIN 4
#define PIN_RI 33
#define RESET 5
#define BAT_ADC 35
#define BAT_EN 12

// GSM Library
#define TINY_GSM_MODEM_SIM7600
HardwareSerial SerialAT(1);

// GSM RX/TX Buffer - GSM Intiation
#if !defined(TINY_GSM_RX_BUFFER)
#define TINY_GSM_RX_BUFFER 1024
#endif
#define TINY_GSM_YIELD() \
  {                      \
    delay(2);            \
  }

// GSM and httpclient libraries
#include <TinyGsmClient.h>
#include <ArduinoHttpClient.h>
#include "SSLClient.h"

// Website Credentials
const char apn[] = "smartlte";
const char gprsUser[] = "";
const char gprsPass[] = "";
// Kloudtrackdev
// const char server[] = "development.kloudtechsea.com"; 
// const char resource[] = "https://development.kloudtechsea.com/Kloudtrackv4/weather/WeatherReadings/insert-data.php";
// v1server Serial 1
// const char server[] = "v1server.kloudtechsea.com"; 
// const char resource[] = "https://v1server.kloudtechsea.com/insert-weather?serial=867942a6-bba7-4f98-85e3-ddce529f9c1d";
// v1server Serial 2
const char server[] = "v1server.kloudtechsea.com"; 
const char resource[] = "https://v1server.kloudtechsea.com/insert-weather?serial=b1aceef9-fb78-405c-b3e3-3a6be96f6932";
// v1server Serial 3
// const char server[] = "v1server.kloudtechsea.com"; 
// const char resource[] = "https://v1server.kloudtechsea.com/insert-weather?serial=f0c1169d-6f2a-44c9-a96d-143b77643c9d";
// v1server Serial 4
// const char server[] = "v1server.kloudtechsea.com"; 
// const char resource[] = "https://v1server.kloudtechsea.com/insert-weather?serial=816a2a9a-5f29-4d47-a545-d0ab0e97ffdd";

TinyGsm modem(SerialAT);
const int port = 443;

// HTTPS Transport
TinyGsmClient base_client(modem, 0);
SSLClient secure_layer(&base_client);
HttpClient client = HttpClient(secure_layer, server, port);

// String Parameters
String t1Str = "";
String h1Str = "";
String p1Str = "";
String t2Str = "";
String h2Str = "";
String p2Str = "";
String t3Str = "";
String h3Str = "";
String p3Str = "";
String lightStr = "";
String uvIntensityStr = "";
String windDirStr = "";
String windSpeedStr = "";
String rainStr = "";
String gustStr = "";
String batteryStr = "";

// SD Card Definitions
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#define SCK 14
#define MISO 2
#define MOSI 15
#define CS 13
SPIClass spi = SPIClass(VSPI);
char data[100];

// SD Card Parameters
void appendFile(fs::FS &fs, String path, String message)
{
  File file = fs.open(path, FILE_APPEND);
  file.close();
}

void createHeader(fs::FS &fs, String path, String message)
{
  File file = fs.open(path);
  if (!file)
  {
    File file = fs.open(path, FILE_APPEND);
    return;
  }
  file.close();
}

// Time
String response, dateTime, year, month, day, hour, minute, second, fileName;
int sleeptimer;

uint32_t AutoBaud() {
  static uint32_t rates[] = {115200, 9600, 57600,  38400, 19200,  74400, 74880,
                              230400, 460800, 2400,  4800,  14400, 28800
                            };
  for (uint8_t i = 0; i < sizeof(rates) / sizeof(rates[0]); i++) {
    uint32_t rate = rates[i];
    // Serial.printf("Trying baud rate %u\n", rate);
    SerialAT.updateBaudRate(rate);
    delay(10);
    for (int j = 0; j < 10; j++) {
      SerialAT.print("AT\r\n");
      String input = SerialAT.readString();
      if (input.indexOf("OK") >= 0) {
        // Serial.printf("Modem responded at rate:%u\n", rate);
        return rate;
      }
    }
  }
  SerialAT.updateBaudRate(115200);
  return 0;
}

void getTime()
{
  response = "";
  SerialAT.print("AT+CCLK?\r\n");
  delay(100);
  response = SerialAT.readString();
  if (response != "") {
    int startIndex = response.indexOf("+CCLK: \"");
    int endIndex = response.indexOf("\"", startIndex + 8);
    String dateTimeString = response.substring(startIndex + 8, endIndex);

    int dayIndex = dateTimeString.indexOf("/");
    int monthIndex = dateTimeString.indexOf("/", dayIndex + 1);
    int yearIndex = dateTimeString.indexOf(",");

    String year = dateTimeString.substring(0, dayIndex);
    String month = dateTimeString.substring(dayIndex + 1, monthIndex);
    String day = dateTimeString.substring(monthIndex + 1, yearIndex);

    String timeString = dateTimeString.substring(yearIndex + 1);

    int hourIndex = timeString.indexOf(":");
    int minuteIndex = timeString.indexOf(":", hourIndex + 1);

    String hour = timeString.substring(0, hourIndex);
    String minute = timeString.substring(hourIndex + 1, minuteIndex);
    String second = timeString.substring(minuteIndex + 1);

    int plusIndex = second.indexOf("+");
    if (plusIndex != -1) {
      second = second.substring(0, plusIndex);
    }
    dateTime = "20" + year + "-" + month + "-" + day + " " + hour + ":" + minute + ":" + second;
    fileName = "/" + day + month + year + ".csv";
  }
}

// Saving to SD Card
void logDataToSDCard()
{
  if (!SD.begin(CS, spi))
  {
    SerialMon.println(" >Failed. Skipping SD Storage");
  }
  else
  {
    SerialMon.println("SD Card Initiation Complete");
    getTime();
    SerialMon.println("Datetime: " + dateTime);
    SerialMon.println("Filename:" + fileName);
    sprintf(data, "%s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s", 
            t1Str, h1Str, p1Str, t2Str, h2Str, p2Str, t3Str, h3Str, p3Str, 
            windDirStr, lightStr, uvIntensityStr, rainStr, windSpeedStr);    
    SerialMon.println("Data: " + String(data));
    String log = dateTime + data;

    createHeader(SD, fileName, "Date, Temperature 1, Humidity 1, Pressure 1, Temperature 2, Humidity 2, Pressure 2, Temperature 3, Humidity 3, Pressure 3, Wind Direction, Light Intensity, UV Intensity, Precipitation, Wind Speed");
    appendFile(SD, fileName, log);

    SerialMon.println("Data logged successfully.");
    SerialMon.println();
  }
}