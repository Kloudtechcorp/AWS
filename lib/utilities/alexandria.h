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
// AWS Server 1 - Axe
const char server[] = "app.kloudtechsea.com";
const char resource[] = "https://app.kloudtechsea.com/api/v1/weather/insert-data?serial=99NL-VII0-BCFW-TUO1";
String stationName = "Colegio Santa Catarina De Alexandria";
String versionCode = "AWS";

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
    sprintf(data, "%s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s", 
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

// BME280 Library and Variables
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
Adafruit_BME280 bme1;
Adafruit_BME280 bme2;
Adafruit_BME280 bme3;
float t1, h1, p1, t2, h2, p2, t3, h3, p3;

// AS5600 Variables
int magnetStatus, lowbyte, rawAngle, correctedAngle;
word highbyte;
float degAngle, startAngle;
RTC_DATA_ATTR float rtcStartAngle;
RTC_DATA_ATTR int rtcCorrectAngle;

// UV Variables
#define uvPin 32
float sensorVoltage, sensorValue;
int uvIntensity;

// BH1750 Library and Variables
#include <BH1750.h>
BH1750 lightMeter;
float lux, irradiance;

// Slave Address
#define SLAVE 0x03

// Rain Gauge
float tipValue = 0.1099, rain;
uint16_t receivedRainCount = 0;

// Wind Speed and Gust var
float windspeed;
int radius = 50, period = 60;
uint16_t receivedWindCount = 0;
float gust;
uint16_t receivedGustCount = 0;

// Battery Voltage Library and Variable
#include <Adafruit_INA219.h>
Adafruit_INA219 ina219;

void select_bus(uint8_t bus) {
  Wire.beginTransmission(0x70);
  Wire.write(1 << bus);
  Wire.endTransmission();
}

void getBME(Adafruit_BME280 bme, int bus, float *temp, float *hum, float *pres) {
  select_bus(bus);
  *temp = bme.readTemperature();
  *hum = bme.readHumidity();
  *pres = bme.readPressure() / 100.0F;
}

String getUV() {
  sensorValue = analogRead(uvPin);
  sensorVoltage = sensorValue * (3.3 / 4095);
  uvIntensity = sensorVoltage * 1000;
  uvIntensityStr = String(uvIntensity);
  return uvIntensityStr;
}

String getLight() {
  lux = lightMeter.readLightLevel();
  lightStr = String(lux);
  return lightStr;
}

void ReadRawAngle() {
  Wire.beginTransmission(0x36);
  Wire.write(0x0D);
  Wire.endTransmission();
  Wire.requestFrom(0x36, 1);
  lowbyte = Wire.read();

  Wire.beginTransmission(0x36);
  Wire.write(0x0C);
  Wire.endTransmission();
  Wire.requestFrom(0x36, 1);
  highbyte = Wire.read();

  highbyte = highbyte << 8;
  rawAngle = highbyte | lowbyte;
  degAngle = rawAngle * 0.087890625;
}

void correctAngle() {
  correctedAngle = 360 - degAngle + startAngle;
  if (correctedAngle > 360) { correctedAngle -= 360; }
  rtcCorrectAngle = correctedAngle;
  if (correctedAngle == 360) { correctedAngle = 0; }
}

String getDirection() {
  ReadRawAngle();
  correctAngle();
  windDirStr = String(correctedAngle);
  return windDirStr;
}

void getSlave() {
  Wire.requestFrom(SLAVE, 6);

  // Rain
  if (Wire.available() >= 4) {
    byte msb = Wire.read();
    byte lsb = Wire.read();
    receivedRainCount = (msb << 8) | lsb;
  }
  rain = receivedRainCount * tipValue;

  // Wind speed
  if (Wire.available() >= 2) {
    byte msb = Wire.read();
    byte lsb = Wire.read();
    receivedWindCount = (msb << 8) | lsb;
  }
  windspeed = (((2 * PI * radius * receivedWindCount) / period) / 1000) * 3.6;

  // Gust
  if (Wire.available() >= 2) {
    byte msb = Wire.read();
    byte lsb = Wire.read();
    receivedGustCount = (msb << 8) | lsb; // Correctly assign receivedGustCount
  }
  gust = (((2 * PI * radius * receivedWindCount) / 3) / 1000) * 3.6;
}

String getBatteryVoltage() {
  batteryStr = String(ina219.getBusVoltage_V());
  return batteryStr;
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
    delay(10000);
    modem.restart();
    return;
  }
  SerialMon.println(" >OK");
  AutoBaud();
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

void collectTHP() {
  // BME 1 Connect
  SerialMon.print("BME 1: ");
  select_bus(2);
  if (!bme1.begin(0x76)) { SerialMon.println(" Failed"); }
  else {
    SerialMon.println(" OK");
    getBME(bme1, 2, &t1, &h1, &p1);
    t1Str = String(t1);
    h1Str = String(h1);
    p1Str = String(p1);
  }
  delay(10);
  // BME 2 Connect
  SerialMon.print("BME 2: ");
  select_bus(3);
  if (!bme2.begin(0x76)) { SerialMon.println(" Failed"); }
  else {
    SerialMon.println(" OK");
    getBME(bme2, 3, &t2, &h2, &p2);
    t2Str = String(t2);
    h2Str = String(h2);
    p2Str = String(p2);
  }
  delay(10);
  // BME 3 Connect
  SerialMon.print("BME 3: ");
  select_bus(4);
  if (!bme3.begin(0x76)) { SerialMon.println(" Failed"); }
  else {
    SerialMon.println(" OK");
    getBME(bme3, 4, &t3, &h3, &p3);
    t3Str = String(t3);
    h3Str = String(h3);
    p3Str = String(p3);
  }
  delay(10);
}

void collectLight() {
  SerialMon.print("BH1750: ");
  if (!lightMeter.begin()) { SerialMon.println(" Failed"); }
  else {
    SerialMon.println(" OK");
    getLight();
  }
  delay(10);
}

void collectUV() {
  // UV Connect
  const int debounceThreshold = 10;
  int uvPrevStatus = 0;
  SerialMon.print("UV: ");
  int uvStatus = analogRead(32);
  if (abs(uvStatus - uvPrevStatus) > debounceThreshold) {
    uvPrevStatus = uvStatus;
    delay(50);
    uvStatus = analogRead(32);
  }

  if (uvPrevStatus != uvStatus) { SerialMon.println(" Failed"); }
  else {
    SerialMon.println(" OK");
    getUV();
  }
  delay(10);
}

void collectDirection() {
  SerialMon.print("AS5600: ");
  startAngle - rtcStartAngle;
  ReadRawAngle();
  if (rtcStartAngle == 0) { rtcStartAngle = degAngle; }
  startAngle = rtcStartAngle;
  correctedAngle = rtcCorrectAngle;
  Wire.beginTransmission(0x36);
  if (!Wire.endTransmission() == 0) { SerialMon.println(" Failed"); }
  else {
    SerialMon.println(" OK");
    getDirection();
  }
}

void collectSlave() {
  SerialMon.print("Slave: ");
  Wire.beginTransmission(SLAVE);
  if (!Wire.endTransmission() == 0) { SerialMon.println(" Failed"); }
  else {
    SerialMon.println(" OK");
    getSlave();
    windSpeedStr = String(windspeed);
    rainStr = String(rain);
    gustStr = String(gust);
  }
  delay(10);
}

void collectBatteryV() {
  SerialMon.print("INA219: ");
  if (!ina219.begin()) { SerialMon.println(" Failed"); }
  else {
    SerialMon.println(" OK");
    getBatteryVoltage();
  }
  delay(10);
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
