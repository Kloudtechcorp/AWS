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
// v1server Serial 1
const char server[] = "v1server.kloudtechsea.com"; 
const char resource[] = "https://v1server.kloudtechsea.com/insert-weather?serial=867942a6-bba7-4f98-85e3-ddce529f9c1d";

TinyGsm modem(SerialAT);
const int port = 443;
bool connectedAPN = false;
int retryCountAPN = 0;
const int maxRetriesAPN = 10;
bool connectedServer = false;
int retryCountServer = 0;
const int maxRetriesServer = 10;

// HTTPS Transport
TinyGsmClient base_client(modem, 0);
SSLClient secure_layer(&base_client);
HttpClient client = HttpClient(secure_layer, server, port);

// ToughSonic Parameters
#include <ModbusMaster.h>
#include <SoftwareSerial.h>
SoftwareSerial mySerial(19,18);
ModbusMaster node;
int distanceArray[10];
float distance;
int mode;

// String Parameters
String distanceStr = "";
String communication = "";

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

void getTime() {
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
void logDataToSDCard() {
  if (!SD.begin(CS, spi)) { SerialMon.println(" >Failed. Skipping SD Storage"); }
  else {
    SerialMon.println("SD Card Initialization Complete");
    getTime();
    SerialMon.println("Datetime: " + dateTime);
    SerialMon.println("Filename:" + fileName);
    sprintf(data, "%s, %s", 
            distanceStr, communication);    
    SerialMon.println("Data: " + String(data));
    String log = dateTime + data;

    createHeader(SD, fileName, "Date, Distance, Communication");
    appendFile(SD, fileName, log);

    SerialMon.println("Data logged successfully.");
    SerialMon.println();
  }
}

void GSMinit() {
  SerialMon.println("\n=================================== GSM Initializing ===================================");
  SerialMon.print("Starting GSM...");

  // A7670-GSM Reset
  pinMode(RESET, OUTPUT);
  digitalWrite(RESET, LOW); delay(100);
  digitalWrite(RESET, HIGH); delay(3000);
  digitalWrite(RESET, LOW);

  // A7670-GSM Power
  pinMode(PWR_PIN, OUTPUT);
  digitalWrite(PWR_PIN, LOW); delay(100);
  digitalWrite(PWR_PIN, HIGH); delay(1000);
  digitalWrite(PWR_PIN, LOW); delay(3000);
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

int calculateMode(int arr[], int size) {
  int maxCount = 0, mode = 0;
  for (int i = 0; i < size; i++) {
    int count = 0;
    for (int j = 0; j < size; j++) {
      if (arr[j] == arr[i]) {
        count++;
      }
    }
    if (count > maxCount) {
      maxCount = count;
      mode = arr[i];
    }
  }
  return mode;
}

void resetDistance(int arr[], int size) {
  for (int i = 0; i < size; i++) {
    arr[i] = 0;
  }
}

void getDistance() {
  SerialMon.println("Ultrasonic Sensor");
  // Calculate distance
  for (int i = 0; i <= 9; i++) {
    if (node.readHoldingRegisters(0x0200, 15) == node.ku8MBSuccess) {
      distance = node.getResponseBuffer(14) * 0.003384 * 2.0 * 2.54;
      distanceArray[i] = distance;
      SerialMon.println("Distance " + String(i + 1) + ": " + String(distanceArray[i])); 
      delay(1000);
    }
  }
  mode = calculateMode(distanceArray, 10); // Solve for mode
  resetDistance(distanceArray, 10);
  distanceStr = String(mode);
}

void printResults() {
  SerialMon.println("Distance in cm = " + distanceStr);
  SerialMon.println("Time: " + dateTime);
  SerialMon.println("Communication Status: " + communication);
}

void connectAPN() {
  SerialMon.println("\n=================================== Connecting to APN ===================================");
  SerialMon.printf("Connecting to %s", apn);
  while (!connectedAPN && retryCountAPN < maxRetriesAPN) {
    if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
      communication = "Failed";
      SerialMon.print(".");
      retryCountAPN++;
      delay(1000);
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

void startSDCard() {
  SerialMon.println("\n=================================== SD Card Initializing ===================================");
  SerialMon.println("Connecting to SD Card...");
  spi.begin(SCK, MISO, MOSI, CS);
  getTime();
  logDataToSDCard();
}

void sendHTTPPostRequest() {
  SerialMon.println("\n========================================HTTP Post Request========================================");
  SerialMon.println("Performing HTTP POST request...");
  client.connectionKeepAlive();
  SerialMon.printf("Connecting to %s\n", server);

  SerialMon.println("Making POST request securely");

  String postData = "{\"recordedAt\":\"" + dateTime + "\", \"distance\":\"" + distanceStr + "\"}";

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