#include <Arduino.h>
#include <math.h>
#include <Wire.h>

// ESP32 Serial Monitor
#define SerialMon Serial

// Sleep Factors
// #define uS_TO_S_FACTOR 1000000ULL /* Conversion factor for micro seconds to seconds *///( changed)
// #define TIME_TO_SLEEP 60          /* Time ESP32 will go to sleep (in seconds) *///( changed)

// SLEEP TIMER
int sleeptimer;

// GSM Library
#define TINY_GSM_MODEM_SIM7600
// GSM Serial Monitor
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

TinyGsm modem(SerialAT);

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

// Website Credentials
const char apn[] = "smartlte"; // Change this to your Provider details
const char gprsUser[] = "";
const char gprsPass[] = "";
const char server[] = "v1server.kloudtechsea.com"; // Change this to your selection
const char resource[] = "https://v1server.kloudtechsea.com/insert-weather?serial=867942a6-bba7-4f98-85e3-ddce529f9c1d";

const int port = 443;
unsigned long timeout;

// HTTPS Transport
TinyGsmClient base_client(modem, 0);
SSLClient secure_layer(&base_client);
HttpClient client = HttpClient(secure_layer, server, port);

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

// Time
String response, dateTime, year, month, day, hour, minute, second;
String fileName;

// BME280 Library
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
Adafruit_BME280 bme1;
Adafruit_BME280 bme2;
Adafruit_BME280 bme3;
// BME280 Variables
float t1, h1, p1;
float t2, h2, p2;
float t3, h3, p3;

// AS5600 Variables
int magnetStatus;
int lowbyte;
word highbyte;
int rawAngle;
float degAngle;
int correctedAngle;
float startAngle;
RTC_DATA_ATTR float rtcStartAngle;
RTC_DATA_ATTR int rtcCorrectAngle;

// UV Variables
#define UVPIN 32
float sensorVoltage;
float sensorValue;
int uvIntensity;

// BH1750 Library
#include <BH1750.h>
// BH1750 Name
BH1750 lightMeter;
float lux;
float irradiance;

// Slave Address
#define SLAVE 0x03

// Rain Gauge
float tipValue = 0.1099, rain;
uint16_t receivedRainCount;

// Wind Speed and Gust var
float windspeed;
//int REV, radius = 100, period = TIME_TO_SLEEP;//( changed)
int REV, radius = 50, period = 60;
uint16_t receivedWindCount;
float gust;
uint16_t receivedGustCount;

// Battery Voltage Library
#include <Adafruit_INA219.h>
Adafruit_INA219 ina219;
// Battery Voltage Variables
float busVoltage;

// String Parameters
String t1_str;
String h1_str;
String p1_str;
String t2_str;
String h2_str;
String p2_str;
String t3_str;
String h3_str;
String p3_str;
String light_str;
String uvintensity_str;
String winddir_str;
String windspeed_str;
String rain_str;
String gust_str;
String battery_str;

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
    dateTime = day + "/" + month + "/" + year + " " + hour + ":" + minute + ":" + second;
    fileName = "/" + day + "/" + month + "/" + year + ".csv";
  }
  
}

// String getFileName()
// {
//   DateTime now = rtc.now();
//   char fileNameString[20]; // adjust the size as needed
//   sprintf(fileNameString, "/%04d%02d%02d.csv", now.year(), now.month(), now.day());
//   return String(fileNameString);
// }

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
    sprintf(data, "%.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f", t1_str, h1_str, p1_str, t2_str, h2_str, p2_str, t3_str, h3_str, p3_str, winddir_str, light_str, uvintensity_str, rain_str, windspeed_str);
    SerialMon.println("Data: " + String(data));
    String log = dateTime + data;

    createHeader(SD, fileName, "Date, Temperature 1, Humidity 1, Pressure 1, Temperature 2, Humidity 2, Pressure 2, Temperature 3, Humidity 3, Pressure 3, Wind Direction, Light Intensity, UV Intensity, Precipitation, Wind Speed");
    appendFile(SD, fileName, log);

    SerialMon.println("Data logged successfully.");
    SerialMon.println();
  }
}

void select_bus(uint8_t bus)
{
  Wire.beginTransmission(0x70);
  Wire.write(1 << bus);
  Wire.endTransmission();
}

void getBME(Adafruit_BME280 bme, int bus, float *temp, float *hum, float *pres)
{
  select_bus(bus);
  *temp = bme.readTemperature();
  *hum = bme.readHumidity();
  *pres = bme.readPressure() / 100.0F;
}

void getUV()
{
  sensorValue = analogRead(UVPIN);
  sensorVoltage = sensorValue * (3.3 / 4095);
  uvIntensity = sensorVoltage * 1000;
}

void getLight()
{
  lux = lightMeter.readLightLevel();
}

void ReadRawAngle()
{
  // 7:0 - bits
  Wire.beginTransmission(0x36); // connect to the sensor
  Wire.write(0x0D);             // figure 21 - register map: Raw angle (7:0)
  Wire.endTransmission();       // end transmission
  Wire.requestFrom(0x36, 1);    // request from the sensor
  lowbyte = Wire.read();        // Reading the data after the request

  // 11:8 - 4 bits
  Wire.beginTransmission(0x36);
  Wire.write(0x0C); // figure 21 - register map: Raw angle (11:8)
  Wire.endTransmission();
  Wire.requestFrom(0x36, 1);
  highbyte = Wire.read();

  // 4 bits have to be shifted to its proper place as we want to build a 12-bit number
  highbyte = highbyte << 8;      // shifting to left
  rawAngle = highbyte | lowbyte; // int is 16 bits (as well as the word)
  degAngle = rawAngle * 0.087890625;
}

void correctAngle()
{
  // recalculate angle
  correctedAngle = 360 - degAngle + startAngle; // this tares the position

  if (correctedAngle > 360) // if the calculated angle is negative, we need to "normalize" it
  {
    correctedAngle -= 360; // correction for negative numbers (i.e. -15 becomes +345)
  }
  rtcCorrectAngle = correctedAngle;
}

void getDirection()
{
  ReadRawAngle();
  correctAngle(); // make a reading so the degAngle gets updated
}

void getSlave()
{
  // Wire.begin();
  Wire.requestFrom(SLAVE, 4);

  // Rain
  while (4 < Wire.available())
  {
    byte msb = Wire.read();
    byte lsb = Wire.read();
    receivedRainCount = (msb << 8) | lsb;
  }
  rain = receivedRainCount * tipValue;

  // Wind speed
  while (2 < Wire.available())
  {
    byte msb = Wire.read();
    byte lsb = Wire.read();
    receivedWindCount = (msb << 8) | lsb;
  }
  windspeed = (2 * PI * radius * receivedWindCount * 3.6) / (period * 1000);

  // Gust
  while (Wire.available())
  {
    byte msb = Wire.read();
    byte lsb = Wire.read();
    receivedGustCount = (msb << 8) | lsb;
  }
  gust = (2 * PI * radius * receivedGustCount * 3.6) / (3 * 1000);
}

void getBatteryVoltage()
{
  busVoltage = ina219.getBusVoltage_V();
}

void GSMinit()
{
  // A7670-GSM Reset
  pinMode(RESET, OUTPUT);
  digitalWrite(RESET, LOW);
  delay(100);
  digitalWrite(RESET, HIGH);
  delay(3000);
  digitalWrite(RESET, LOW);

  // A7670-GSM Power
  pinMode(PWR_PIN, OUTPUT);
  digitalWrite(PWR_PIN, LOW);
  delay(100);
  digitalWrite(PWR_PIN, HIGH);
  delay(1000);
  digitalWrite(PWR_PIN, LOW);
}

void setup()
{
  // Start Serial Monitor
  SerialMon.begin(115200);
  delay(10); // feedback buffer

  // Initialize GSM
  SerialMon.println("\n========================================GSM Initializing========================================");
  SerialMon.println("Starting GSM...");
  GSMinit();
  SerialMon.print("Waiting for 3s...");
  delay(3000);
  SerialMon.println(" >OK");

  SerialMon.print("Starting Serial Communications...");
  SerialAT.begin(UART_BAUD, SERIAL_8N1, PIN_RX, PIN_TX);
  SerialMon.println(" >OK");

  SerialMon.print("Initializing modem...");
  if (!modem.init())
  {
    SerialMon.println(" >Failed (Restarting in 10s)");
    return;
  }
  SerialMon.println(" >OK");
  // SerialMon.print("Selecting Network Mode...");
  // String result;
  // result = modem.setNetworkMode(2);
  // if (!modem.waitResponse(10000L) == 1)
  // {
  //   SerialMon.println(" >OK");
  //   return;
  // }
  // SerialMon.println(" >Failed");
  AutoBaud();
  
  SerialMon.println("\n========================================Sensors Status========================================");
  // Sensors Status
  Wire.begin(21, 22);

  // BME 1 Connect
  SerialMon.print("BME 1: ");
  bool BME1_status;
  select_bus(2);
  BME1_status = bme1.begin(0x76);
  if (!BME1_status)
  {
    SerialMon.println(" Failed");
    t1_str = "";
    h1_str = "";
    p1_str = "";
  }
  else
  {
    SerialMon.println(" OK");
    getBME(bme1, 2, &t1, &h1, &p1);
    t1_str = String(t1);
    h1_str = String(h1);
    p1_str = String(p1);
  }
  delay(10);
  // BME 2 Connect
  SerialMon.print("BME 2: ");
  bool BME2_status;
  select_bus(3);
  BME2_status = bme2.begin(0x76);
  if (!BME2_status)
  {
    SerialMon.println(" Failed");
    t2_str = "";
    h2_str = "";
    p2_str = "";
  }
  else
  {
    SerialMon.println(" OK");
    getBME(bme2, 3, &t2, &h2, &p2);
    t2_str = String(t2);
    h2_str = String(h2);
    p2_str = String(p2);
  }
  delay(10);
  // BME 3 Connect
  SerialMon.print("BME 3: ");
  bool BME3_status;
  select_bus(4);
  BME3_status = bme3.begin(0x76);
  if (!BME3_status)
  {
    SerialMon.println(" Failed");
    t3_str = "";
    h3_str = "";
    p3_str = "";
  }
  else
  {
    SerialMon.println(" OK");
    getBME(bme3, 4, &t3, &h3, &p3);
    t3_str = String(t3);
    h3_str = String(h3);
    p3_str = String(p3);
  }
  delay(10);

  // BH1750 Connect
  SerialMon.print("BH1750: ");
  bool light_status;
  light_status = lightMeter.begin();
  if (!light_status)
  {
    SerialMon.println(" Failed");
    light_str = "";
  }
  else
  {
    SerialMon.println(" OK");
    getLight();
    light_str = String(lux);
  }
  delay(10);

  // UV Connect
  const int debounceThreshold = 10;
  int uvPrevStatus = 0;
  SerialMon.print("UV: ");
  int uvStatus = analogRead(32);
  if (abs(uvStatus - uvPrevStatus) > debounceThreshold)
  {
    uvPrevStatus = uvStatus;
    delay(50);
    uvStatus = analogRead(32);
  }

  if (uvPrevStatus == uvStatus)
  {
    SerialMon.println(" Failed");
    uvintensity_str = "";
  }
  else
  {
    SerialMon.println(" OK");
    getUV();
    uvintensity_str = String(uvIntensity);
  }
  delay(10);

  // AS5600 Connect
  SerialMon.print("AS5600: ");
  bool winddir_status;
  Wire.beginTransmission(0x36);
  winddir_status = (Wire.endTransmission() == 0);
  if (!winddir_status)
  {
    SerialMon.println(" Failed");
    winddir_str = "";
  }
  else
  {
    SerialMon.println(" OK");
    getDirection();
    winddir_str = String(correctedAngle);
  }

  // Slave Connect
  SerialMon.print("Slave: ");
  bool Slave_status;
  Wire.beginTransmission(SLAVE);
  Slave_status = (Wire.endTransmission() == 0);
  if (!Slave_status)
  {
    SerialMon.println(" Failed");
    windspeed_str = "";
    rain_str = "";
    gust_str = "";
  }
  else
  {
    SerialMon.println(" OK");
    getSlave();
    windspeed_str = String(windspeed);
    rain_str = String(rain);
    gust_str = String(gust);
  }
  delay(10);

  // INA219 Connect
  SerialMon.print("INA219: ");
  bool battery_status;
  battery_status = ina219.begin();
  if (!battery_status)
  {
    SerialMon.println(" Failed");
    battery_str = "";
  }
  else
  {
    SerialMon.println(" OK");
    getBatteryVoltage();
    battery_str = String(busVoltage);
  }
  delay(10);
  
  getTime();
  SerialMon.println("Time: " + dateTime);

  // Start SD Card
  SerialMon.println("\n========================================SD Card Initializing========================================");
  SerialMon.println("Connecting to SD Card...");
  spi.begin(SCK, MISO, MOSI, CS);
  logDataToSDCard();
}

void loop()
{
  // APN Connect
  SerialMon.println("\n========================================Connecting to APN========================================");
  SerialMon.print("Connecting to ");
  SerialMon.print(apn);
  bool connected = false;
  int retryCount = 0;
  const int maxRetries = 10;
  while (!connected && retryCount < maxRetries) {
    if (!modem.gprsConnect(apn, gprsUser, gprsPass))
    {
      SerialMon.println(" >Failed");
      retryCount++;
      delay(1000);
    }
    else
    {
      SerialMon.println(" >OK");
      connected = true;
      // Server Connect
      SerialMon.println("\n========================================Connecting to Server========================================");
      SerialMon.print("Connecting to ");
      SerialMon.print(server);
      if (!base_client.connect(server, port))
      {
        SerialMon.println(" >Failed");
      }
      else
      {
        SerialMon.println(" >OK");
        // Check network connection status
        if (modem.isNetworkConnected())
        {
          SerialMon.println("Success to connect to network.");
        }

        // Check GPRS connection status
        if (modem.isGprsConnected())
        {
          SerialMon.println("Success to connect to GPRS.");
        }

        SerialMon.println("====================================== Print results =========================================================");

        // Print readings results
        SerialMon.println("T1 = " + t1_str);
        SerialMon.println("T2 = " + t2_str);
        SerialMon.println("T3 = " + t3_str);
        SerialMon.println("H1 = " + h1_str);
        SerialMon.println("H2 = " + h2_str);
        SerialMon.println("H3 = " + h3_str);
        SerialMon.println("P1 = " + p1_str);
        SerialMon.println("P2 = " + p2_str);
        SerialMon.println("P3 = " + p3_str);
        SerialMon.println("Light Intensity = " + light_str);
        SerialMon.println("UV Intensity = " + uvintensity_str);
        SerialMon.println("Wind Direction = " + winddir_str);
        SerialMon.println("Wind Speed = " + windspeed_str);
        SerialMon.println("Rain = " + rain_str);
        SerialMon.println("Gust = " + gust_str);
        SerialMon.println("Battery Voltage = " + battery_str);
        SerialMon.println("Time: " + dateTime);

        SerialMon.println("\n========================================HTTP Post Request========================================");
        SerialMon.println("Performing HTTP POST request...");
        client.connectionKeepAlive();
        SerialMon.print("Connecting to ");
        SerialMon.println(server);

        SerialMon.println("Making POST request securely");
        String contentType = "Content-Type: application/json";

        String postData = "{\"recordedAt\":\"" + dateTime + "\", \"light\":\"" + light_str + "\", \"uvIntensity\":\"" + uvintensity_str + "\", \"windDirection\":\"" + winddir_str + "\", \"windSpeed\":\"" + windspeed_str + "\", \"precipitation\":\"" + rain_str + "\", \"gust\":\"" + gust_str + "\", \"T1\":\"" + t1_str + "\", \"T2\":\"" + t2_str + "\", \"T3\":\"" + t3_str + "\", \"H1\":\"" + h1_str + "\", \"H2\":\"" + h2_str + "\", \"H3\":\"" + h3_str + "\", \"P1\":\"" + p1_str + "\", \"P2\":\"" + p2_str + "\", \"P3\":\"" + p3_str + "\", \"batteryVoltage\":\"" + battery_str + "\"}";

        SerialMon.println("");
        SerialMon.println("\n=========================================POST Data ============================================");
        SerialMon.println(postData);

        int postDataLength = postData.length();
        client.sendHeader("Content-Length", postDataLength);
        client.sendHeader("Connection", "Close");
        int posting = client.post(resource, contentType, postData);
        SerialMon.print("Reply:");
        SerialMon.println(posting);
        int status_code = client.responseStatusCode();
        String response = client.responseBody();

        SerialMon.print("Status code: ");
        SerialMon.println(status_code);
        SerialMon.print("Response: ");
        SerialMon.println(response);

        SerialMon.println("\n========================================Closing Client========================================");
        client.stop();
        SerialMon.println(F("Server disconnected"));
        modem.gprsDisconnect();
        SerialMon.println(F("GPRS disconnected"));
      }
    }
  }
  // Set Timer and Sleep
  sleeptimer= 6000 - (millis() % 60000);
  esp_sleep_enable_timer_wakeup(sleeptimer * 1000);
  esp_deep_sleep_start();
}