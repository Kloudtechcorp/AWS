#ifndef GSM_COMM_H
#define GSM_COMM_H

// Change to #include <sensor.h> if you want to use other station
#include <ptorivas.h>
// Serial Monitors
#define SerialMon Serial

// GSM Pins - GSM Initiation
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

// GSM RX/TX Buffer - GSM Initiation
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
extern const char deviceApn[];
extern String deviceSerial;
const char gprsUser[] = "";
const char gprsPass[] = "";
const char server[] = "app.kloudtechsea.com";
String resource = "/api/v1/weather/insert-data?serial=" + deviceSerial;
TinyGsm modem(SerialAT);
const int port = 443;
bool connectedAPN = false;
int retryCountAPN = 0;
const int maxRetriesAPN = 5;
bool connectedServer = false;
int retryCountServer = 0;
const int maxRetriesServer = 10;

// HTTPS Transport
TinyGsmClient base_client(modem, 0);
SSLClient secure_layer(&base_client);
HttpClient client = HttpClient(secure_layer, server, port);

// Time
String response, dateTime, year, month, day, hour, minute, second, fileName;

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

#define TIME_THRESHOLD 70  // Allow up to 70s jump (for 1-min sleep)
String lastValidTime = "";  // Store last valid time
unsigned long lastEpoch = 0;  // Store last valid epoch timestamp

// Function to convert date/time to UNIX timestamp
unsigned long convertToEpoch(String year, String month, String day, String hour, String minute, String second) {
  struct tm t;
  t.tm_year = year.toInt() + 2000 - 1900;
  t.tm_mon = month.toInt() - 1;
  t.tm_mday = day.toInt();
  t.tm_hour = hour.toInt();
  t.tm_min = minute.toInt();
  t.tm_sec = second.toInt();
  return mktime(&t);
}

void getTime() {
  response = "";
  SerialAT.print("AT+CCLK?\r\n");
  delay(100);
  response = SerialAT.readString();
  if (response != "") {
    int startIndex = response.indexOf("+CCLK: \"");
    int endIndex = response.indexOf("\"", startIndex + 8);
    if (startIndex == -1 || endIndex == -1) return;  // Invalid response
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
    // dateTime = "20" + year + "-" + month + "-" + day + " " + hour + ":" + minute + ":" + second;
    // fileName = "/" + month + day + "20" + year + ".csv";

    // Convert to epoch time
    unsigned long newEpoch = convertToEpoch(year, month, day, hour, minute, second);

    // Filtering: Ignore large jumps
    if (lastEpoch == 0 || abs((long)newEpoch - (long)lastEpoch) <= TIME_THRESHOLD) {  
      lastEpoch = newEpoch;
      lastValidTime = "20" + year + "-" + month + "-" + day + " " + hour + ":" + minute + ":" + second;
      dateTime = lastValidTime;
      fileName = "/" + month + day + "20" + year + ".csv";
    } else {
      Serial.print("Time jump detected ("); 
      Serial.print(abs((long)newEpoch - (long)lastEpoch));
      Serial.println("s), ignoring...");
    }
  }
}

// SD Card Definitions
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#define SCK 14
#define MISO 2
#define MOSI 15
#define CS 13
SPIClass spi = SPIClass(VSPI);
char data[256];

// SD Card Parameters
void appendFile(fs::FS &fs, String path, String message)
{
  SerialMon.printf("Appending to file: %s\n", path);
  File file = fs.open(path, FILE_APPEND);
  if (!file) { SerialMon.println("Failed to open file for appending"); return; }
  if (file.println(message)) {  SerialMon.println("Message appended"); }
  else { SerialMon.println("Append failed"); }
  file.close();
  SerialMon.println("File Closed");
}

void createHeader(fs::FS &fs, String path, String message)
{
  SerialMon.printf("Checking if %s exists...", path);
  File file = fs.open(path);
  if (!file)
  {
    SerialMon.print("\nFile does not exist creating header files now...");
    File file = fs.open(path, FILE_APPEND);
    if (file.println(message)) { SerialMon.println(" >OK"); }
    else { SerialMon.println(" >Failed"); }
    return;
  }
  SerialMon.println(" >OK");
  file.close();
  SerialMon.println("File Closed");
}

// Saving to SD Card
void logDataToSDCard() {
  SD.end();
  if (!SD.begin(CS, spi)) { SerialMon.println(" >Failed. Skipping SD Storage"); }
  else {
    SerialMon.println("SD Card Initialization Complete");
    getTime();
    SerialMon.println("Datetime: " + dateTime);
    SerialMon.println("Filename:" + fileName);
    sprintf(data, ", %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s", 
            t1Str, h1Str, p1Str, t2Str, h2Str, p2Str, t3Str, h3Str, p3Str, 
            windDirStr, lightStr, uvIntensityStr, rainStr, windSpeedStr, communication);    
    SerialMon.println("Data: " + String(data));
    String log = dateTime + data;

    createHeader(SD, fileName, "Date, Temperature 1, Humidity 1, Pressure 1, Temperature 2, Humidity 2, Pressure 2, Temperature 3, Humidity 3, Pressure 3, Wind Direction, Light Intensity, UV Intensity, Precipitation, Wind Speed, Communication");
    appendFile(SD, fileName, log);

    SerialMon.println("Data logged successfully.");
    SerialMon.println();
  }
}

void startSDCard() {
  SerialMon.println("\n=================================== SD Card Initializing ===================================");
  SerialMon.println("Connecting to SD Card...");
  spi.begin(SCK, MISO, MOSI, CS);
  getTime();
  logDataToSDCard();
}

void printResults() {
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
  SerialMon.println("Time: " + dateTime);
  SerialMon.println("Communication Status: " + communication);
}

void GSMinit() {
  SerialMon.println("\n=================================== GSM Initializing ===================================");
  SerialMon.print("Starting GSM...");

  // A7670-GSM Reset
  pinMode(RESET, OUTPUT);
  digitalWrite(RESET, LOW); delay(100);
  digitalWrite(RESET, HIGH); delay(100);
  digitalWrite(RESET, LOW); delay(100);

  // A7670-GSM Power
  pinMode(PWR_PIN, OUTPUT);
  digitalWrite(PWR_PIN, LOW); delay(100);
  digitalWrite(PWR_PIN, HIGH); delay(100);
  digitalWrite(PWR_PIN, LOW); delay(100);
  SerialMon.println(" >OK");

  SerialMon.print("Starting Serial Communications...");
  SerialAT.begin(UART_BAUD, SERIAL_8N1, PIN_RX, PIN_TX);
  SerialMon.println(" >OK");

  SerialMon.print("Initializing modem...");
  if (!modem.init()) {
    SerialMon.println(" >Failed (Restarting in 10s)");
    return;
  }
  SerialMon.println(" >OK");
  AutoBaud();
}

void connectAPN() {
  SerialMon.println("\n=================================== Connecting to APN ===================================");
  SerialMon.printf("Connecting to %s", deviceApn);
  while (!connectedAPN && retryCountAPN < maxRetriesAPN) {
    if (!modem.gprsConnect(deviceApn)) {
      communication = "Failed";
      SerialMon.print(".");
      retryCountAPN++;
      delay(15000);
    }
    else {
      SerialMon.println(" >OK");
      communication = "Success";
      connectedAPN = true;
    }
  }
}

void connectServer() {
  SerialMon.println("\n=================================== Connecting to Server ===================================");
  SerialMon.printf("Connecting to %s", server);

  while (connectedAPN && !connectedServer && retryCountServer < maxRetriesServer) {
    if (!base_client.connect(server, port)) {
      SerialMon.print(".");
      retryCountServer++;
      delay(1000);
    }
    else {
      SerialMon.println(" >OK");
      connectedServer = true;
    }
  }
}

void sendHTTPPostRequest() {
  SerialMon.println("\n========================================HTTP Post Request========================================");
  SerialMon.println("Performing HTTP POST request...");
  client.connectionKeepAlive();
  SerialMon.printf("Connecting to %s\n", server);

  SerialMon.println("Making POST request securely");

  String postData = "{\"recordedAt\":\"" + dateTime + "\", \"light\":\"" + lightStr + "\", \"uvIntensity\":\"" + uvIntensityStr + "\", \"windDirection\":\"" + windDirStr + "\", \"windSpeed\":\"" + windSpeedStr + "\", \"precipitation\":\"" + rainStr + "\", \"gust\":\"" + gustStr + "\", \"T1\":\"" + t1Str + "\", \"T2\":\"" + t2Str + "\", \"T3\":\"" + t3Str + "\", \"H1\":\"" + h1Str + "\", \"H2\":\"" + h2Str + "\", \"H3\":\"" + h3Str + "\", \"P1\":\"" + p1Str + "\", \"P2\":\"" + p2Str + "\", \"P3\":\"" + p3Str + "\", \"batteryVoltage\":\"" + batteryStr + "\"}";

  SerialMon.println("\n=========================================POST Data ============================================");
  SerialMon.println(postData);

  client.beginRequest();
  client.post(resource);
  client.sendHeader("Content-Type", "application/json");
  client.sendHeader("Content-Length", postData.length());
  client.sendHeader("Connection", "Close");
  client.beginBody();
  client.print(postData);
  client.endRequest();
  
  int status_code = client.responseStatusCode();
  String response = client.responseBody();
  SerialMon.printf("Status code: %d\n", status_code);
  SerialMon.println("Response: " + response);
}

#endif