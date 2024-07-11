#include <Arduino.h>
#include <math.h>

// ESP32 Serial Monitor
#define SerialMon Serial
String Time;

// Magnetic sensor things
int magnetStatus = 0; // value of the status register (MD, ML, MH)

int lowbyte;    // raw angle 7:0
word highbyte;  // raw angle 7:0 and 11:8
int rawAngle;   // final raw angle
float degAngle; // raw angle in degrees (360/4096 * [value between 0-4095])

int quadrantNumber, previousquadrantNumber; // quadrant IDs
float numberofTurns = 0;                    // number of turns
float correctedAngle = 0;                   // tared angle - based on the startup value
float startAngle = 0;                       // starting angle
float totalAngle = 0;                       // total absolute angular displacement
float previoustotalAngle = 0;               // for the display printing

// UV Parameters
float sensorVoltage;
float sensorValue;
int UV_index;

// I2C Library
#include <Wire.h>
// BME280 Library
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
// BH1750 Library
#include <BH1750.h>

// Time
#include "RTClib.h"
RTC_DS3231 rtc;

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
const char server[] = "development.kloudtechsea.com"; // Change this to your selection
const char resource[] = "https://development.kloudtechsea.com/Kloudtrackv4/weather/WeatherReadings/insertTest.php";
// const char* host = "development.kloudtechsea.com";

// const char server[] = "kloudtrack.kloudtechsea.com"; // Change this to your selection
//const char resource[] = "https://development.kloudtechsea.com/weather/WeatherReadings/insertDecrypt.php";

const int port = 443;
unsigned long timeout;

// CERTIFICATE
const char *root_ca =
    // "-----BEGIN CERTIFICATE-----\n"
    // "MIIEpQIBAAKCAQEAoo//aRIq9zl73S3QDaNrDaqu7690WIFYMY2vk9uO1BWeKf32\n"
    // "msiFF5bSVW74pBXsk8br4sg259ff2KZu7cQIFwiHDE56ma4776WBFqFiGPZF4tw1\n"
    // "uQxV0razxVZG5BGh7SUZl8jxNwl6Xiu7nQJrGm6SPObJS9ww+8WLxGeqQG/4TtkL\n"
    // "slDYHP4iaxb2brqOdsi7mrWo4t4mn84xXvuRrJtABX6HaZ/HN+ffngkbFWvDeZGq\n"
    // "Ybxwzgpqoflez+Cm7WY648uM3vL+i2636ONUfaWjMPqF4+sZKm9ckkmvkt1skI54\n"
    // "r+djCt1AtdJhGnM06sPWaGfofJ3Ye8Zhc8R2GQIDAQABAoIBAQCLRft4tYwCC0nH\n"
    // "kPxy8tNA/j6qMxPmz8oTimQmtTCFG5BKQb8JlUoRj7HcaBq12eK6KO7neMkxkI3D\n"
    // "O7RGGHEl6rRpIsN+7vuCsbULAnDdGgF9/1Q3mg3dXZJfOy+5LjIgzmxC0nF2ybQB\n"
    // "Ak7Wkb/ZAWpgqwlC6P8+dVzthDMkyGrViO/X6F8q/0knwO11iiUgPIzhc0WH33cr\n"
    // "XjrJA9bJWThCpPU7XBlDsCyH12i4cIDk8+fNDSepjPBtXRF1w3fTw8GeZxmXpg5U\n"
    // "K2o/owdGoDK1jH4hWVo1fmU0b7YASkkGhDejGBkUBCG9rKfIWpPfAy4HiPvLcJrj\n"
    // "sFa8UYDRAoGBANvuWieXNtNKtBjKiCyVMTjQmXcbnGkHEBTDeFWnqKTfqWowXIRh\n"
    // "UA9QJfe5Beqk2Ot9l5IzDFO9nkkinWGnBqBeaeZevloTNfNB/uIK3curhHRY87p6\n"
    // "yuVfjffMx2WEQDNA5INtyvjjFKhIYYGOCAfYekytgxSXRyqN9E79YOYzAoGBAL05\n"
    // "ETRPg+j5wyJOkhV82u9vbY/Tl6Pb2unRAOXqESSoGtgYOR6NKmbN1dtPhRMBTQrq\n"
    // "PxVjPn2VPlPSkf7cafw9VpTQu+VGHSDoCmMoWhaqJ8dkiDDy4EgmnMWU4jBznLm3\n"
    // "xrVyxRe4qCk+TeMU/Rfonm3LyUrIxt2xEy/uaK6DAoGBAIl/mfO4kTlGMhRxwvME\n"
    // "j1Jy8A9MPtxW4xHoJzp96GyzvIo8FAtbf58jP6mBjpZlW4zW50UrDyDnoqccsgJU\n"
    // "V0s528GAKEfOTON5ti2CF6p24AeReUdz75e5xttOtrbcGPsyLeuRX918svSG1BcY\n"
    // "QiMNj2CS9imQbpjHSR99P/CLAoGBAIf8DQsk4c16QOvUv1NF6SkNURV9oIqXo5lJ\n"
    // "JHYWYr+PN8t4jQQRuxkqb7guS5o+4xknArLiIbSqnqIqv9UCngAyLJjB8WZxntlZ\n"
    // "KK3d//At1GyNKPrUcK4pYZz0xCDB1S36jokzs8S6Zc9OATdijAi0mbaaL0zZQWFs\n"
    // "sjdOBFj1AoGAHcuTZIyTgtKo61pIeZXQvUxdwsa+Ngr4meUx3KKk1/uOcd/vM79D\n"
    // "nIMpDZyTEknIWxwBqCki7aFdiGwH5yK2490vYzXOiS5IXALE52G2t4hI2nOfquHo\n"
    // "f6YdAF0OEvDcFfof5SKjYqkunPSs9LtjVGyiPJv8R9bdxPhbq7cidMw=\n"
    // "-----END CERTIFICATE-----\n";
    "-----BEGIN CERTIFICATE-----\n" \
"MIIGJTCCBQ2gAwIBAgISBNKBw5RrEabvEQ8qXuXIicRZMA0GCSqGSIb3DQEBCwUA\n" \
"MDIxCzAJBgNVBAYTAlVTMRYwFAYDVQQKEw1MZXQncyBFbmNyeXB0MQswCQYDVQQD\n" \
"EwJSMzAeFw0yNDA1MDMwNTAwMTlaFw0yNDA4MDEwNTAwMThaMCYxJDAiBgNVBAMT\n" \
"G2tsb3VkdHJhY2sua2xvdWR0ZWNoc2VhLmNvbTCCAiIwDQYJKoZIhvcNAQEBBQAD\n" \
"ggIPADCCAgoCggIBAJ2jaT1JSGu/EB3ZasUd1iqGWoUypAxCuSD5unPIWrMePVj4\n" \
"lJ/DTyrusWzbnkSfN/IiObhZR0hfdUMJVDXJ5kVw21iGFTiG6Q88G1mnICH7Dmju\n" \
"fAXVrTdPtPRDnM17nGKVv/eNtxA9tqrvnfAA+aaRiWe2zI+8r/FBPSMr1/sEzLCb\n" \
"cpbNSSA56EFQ6KN7nJNFLCZyiJq+3DyIWvhxTW5nmTsj4GTz9LNA1KxQTIxPdS7P\n" \
"4DYfPoXbwGOQJd6Qp+rBNp0q8P2zazFwCUctJMCzUvTPkXiLOQa10ZmgXMS+m+qd\n" \
"xcnV+OBBhm5A4ePkWxj3gG/qMs9xL46u3oYO5a8XFevtQtFefZ/XOh3kWFd1HcMe\n" \
"aLeaZSizZhqZLKMl5jjhnytdbJLqr3XMqHTwYBrqzDjIIQrz05QtwiO2rgmAueTQ\n" \
"I5PkCFoM/BCMYncXWr2ut2dpaUmjnc+m+0Tj9UhC8EywjomDHuxoz88jLCeXcucT\n" \
"OqAn4CnNGPqnCpjZsi8eDc/UScwX9/wvKK3SjVoxkQ79JYEMJrt9T8QpCdz7Jnlz\n" \
"o7mLkjIg1myrNPaXT9e43+2BBsB0Z45dzCwSCdfQa6Yiq9hhL35jH/9gewwG0Cts\n" \
"2ZeAOgOLBfdDIZ0iMmgJrjqJMquGJo9jhjitZ2n12kR9kqXXMH+whut2FJ9tAgMB\n" \
"AAGjggI/MIICOzAOBgNVHQ8BAf8EBAMCBaAwHQYDVR0lBBYwFAYIKwYBBQUHAwEG\n" \
"CCsGAQUFBwMCMAwGA1UdEwEB/wQCMAAwHQYDVR0OBBYEFA+bIIZGnU749LOPr1Zm\n" \
"JsTDDFMbMB8GA1UdIwQYMBaAFBQusxe3WFbLrlAJQOYfr52LFMLGMFUGCCsGAQUF\n" \
"BwEBBEkwRzAhBggrBgEFBQcwAYYVaHR0cDovL3IzLm8ubGVuY3Iub3JnMCIGCCsG\n" \
"AQUFBzAChhZodHRwOi8vcjMuaS5sZW5jci5vcmcvMEcGA1UdEQRAMD6CG2tsb3Vk\n" \
"dHJhY2sua2xvdWR0ZWNoc2VhLmNvbYIfd3d3Lmtsb3VkdHJhY2sua2xvdWR0ZWNo\n" \
"c2VhLmNvbTATBgNVHSAEDDAKMAgGBmeBDAECATCCAQUGCisGAQQB1nkCBAIEgfYE\n" \
"gfMA8QB3AEiw42vapkc0D+VqAvqdMOscUgHLVt0sgdm7v6s52IRzAAABjz0KDG4A\n" \
"AAQDAEgwRgIhAOcRiEp+2Ci1hTER/7D53V/ffcWqViZjqRlpzCA2ob1eAiEAqt7B\n" \
"+w5YGGcbninZX4zhX+KDjpHZroSuBr/6i9L+V/IAdgA/F0tP1yJHWJQdZRyEvg0S\n" \
"7ZA3fx+FauvBvyiF7PhkbgAAAY89Cgx7AAAEAwBHMEUCIEeNLdSPgsped5lnQvbD\n" \
"K3KC5HVQo5qaFk71RB7XbF+dAiEAoG42c29IPxepsHcno1hedijlr6As/FSTeAYa\n" \
"nh5lcSQwDQYJKoZIhvcNAQELBQADggEBAGkNCgI4kxfP5LfpqHYBJXsqfIXBLEca\n" \
"4nfp4j894K56du9age2XbzTg/c5nlQJAUEoMPZyN0yimqpdPwDR89Eq+m9EdVcNl\n" \
"5imJzeM766eWM+K78d2bE+1reqij63Y34lurZUIH0CAP5cH9q85CYok53ZFTQque\n" \
"mkfsYT+furu3I88RGfAgpEpWczZTJO+TK0Yudm8EipwUsdlKCOkreFMUwOpBCPdk\n" \
"Th2/SDDI9OHFSFYbQZVKabOkQaW4UbVgjydehXnWnzOuzRkQD64rdPBk9Xw3DRS9\n" \
"oeges2GJDrMBrUvFL4uetweyPEeDJktOAsQKEqeD3O3t4Qqi2uKpGns=\n" \
"-----END CERTIFICATE-----\n";


// HTTPS Transport
TinyGsmClient base_client(modem,0);
SSLClient secure_layer(&base_client);
HttpClient client = HttpClient(secure_layer, server, port);

// BME280 Name
Adafruit_BME280 bme;
// BME280 var
float temperature = 0;
float humidity = 0;
float pressure = 0;
float lux = 0;
float irradiance = 0;

// BH1750 Name
BH1750 lightMeter;

// Sleep Factors
#define uS_TO_S_FACTOR 1000000ULL /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP 60          /* Time ESP32 will go to sleep (in seconds) */
// SLEEP TIMER
int sleeptimer(int x, int y)
{
  // note f2 cannot be 0
  return y - (x % y);
}
// Slave Address
#define SLAVE 0x03

// Rain Gauge
float tipValue = 0.1099, rain;
uint16_t receivedRainCount;
int currentRainCount;
RTC_DATA_ATTR int prevRainCount;

// Wind Speed var
float windspeed;
int REV, radius = 100, period = TIME_TO_SLEEP;
uint16_t receivedWindCount;
int currentWindCount;
RTC_DATA_ATTR int prevWindCount;

// naming
#define countof(a) (sizeof(a) / sizeof(a[0]))

// DHT22 Library
#include <DHT.h>
// DHT22 Variables
#define DHTPIN 04
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
float h = 0;

// BMP Library
#include <Adafruit_BMP085.h>
// BMP Variables
#define seaLevelPressure_hPa 1013.25
Adafruit_BMP085 bmp;
float t = 0;
float p = 0;

// Heat Index Variables
float T;
float RH;
float HI;
float hi;

// Redundancy Variables
float FTemp = 0;
float FHum = 0;
float FPres = 0;

// Encryption libraries
#include <base64.h>
#include <AESLib.h>
#include <AES.h>
#include <AES_config.h>

// Encryption Variables
String key_str = "aaaaaaaaaaaaaaaa"; // 16 bytes
String iv_str = "aaaaaaaaaaaaaaaa";  // 16 bytes

AES aes;

byte encryptedTemp1[1000];
byte encryptedTemp2[1000];

byte encryptedHum1[1000];
byte encryptedHum2[1000];

byte encryptedPress1[1000];
byte encryptedPress2[1000];

byte encryptedlight[1000];
byte encryptedirradiance[1000];
byte encryptedUVIntensity[1000];
byte encryptedWindDirection[1000];
byte encryptedHeatIndex[1000];
byte encryptedWindSpeed[1000];
byte encryptedRainGauge[1000];

char b64Temp1[1000];
char b64Temp2[1000];
char b64Hum1[1000];
char b64Hum2[1000];
char b64Press1[1000];
char b64Press2[1000];
char b64Light[1000];
char b64Irradiance[1000];
char b64UVIntensity[1000];
char b64WindDirection[1000];
char b64HeatIndex[1000];
char b64Windspeed[1000];
char b64RainGuage[1000];

void getDHT()
{
  h = dht.readHumidity();
  SerialMon.print("Humidity: ");
  SerialMon.print(h);
  SerialMon.println(" %");
}

void getBMP()
{
  t = bmp.readTemperature();
  p = bmp.readPressure() / 100;
  SerialMon.print("Temperature: ");
  SerialMon.print(t);
  SerialMon.println(" C");
  SerialMon.print("Pressure: ");
  SerialMon.print(p);
  SerialMon.println(" Pa");
}

void getUV()
{
  sensorValue = analogRead(32);
  sensorVoltage = sensorValue * (3.3 / 4095);
  UV_index = sensorVoltage / 0.1;
  SerialMon.print("sensor reading = ");
  SerialMon.println(sensorValue);
  SerialMon.print("sensor voltage = ");
  SerialMon.print(sensorVoltage);
  SerialMon.println(" V");
  SerialMon.print("Uv index = ");
  SerialMon.print(UV_index);
  SerialMon.println(" V");
}

void getBH()
{
  lux = lightMeter.readLightLevel();
  char lightString[8];
  dtostrf(lux, 1, 2, lightString);
  SerialMon.print("Light: ");
  SerialMon.println(lightString);

  irradiance = lightMeter.readLightLevel() / 683;
  char irradString[8];
  dtostrf(irradiance, 1, 2, irradString);
  SerialMon.print("Light(Irradiance): ");
  SerialMon.println(irradString);
}

void getBME()
{
  temperature = bme.readTemperature();
  char tempString[8];
  dtostrf(temperature, 1, 2, tempString);
  SerialMon.print("Temperature: ");
  SerialMon.println(tempString);

  humidity = bme.readHumidity();
  char humString[8];
  dtostrf(humidity, 1, 2, humString);
  SerialMon.print("Humidity: ");
  SerialMon.println(humString);

  pressure = bme.readPressure() / 100;
  char preString[8];
  dtostrf(pressure, 1, 2, preString);
  SerialMon.print("Pressure: ");
  SerialMon.println(preString);
}

void getSlave()
{
  Wire.begin();
  Wire.requestFrom(SLAVE, 4);
  while (2 < Wire.available())
  {
    byte msb = Wire.read();
    byte lsb = Wire.read();
    receivedRainCount = (msb << 8) | lsb;
  }
  currentRainCount = receivedRainCount;
  if ((currentRainCount - prevRainCount) > -1)
  {
    rain = (currentRainCount - prevRainCount) * tipValue;
  }
  else
  {
    rain = (65535 + currentRainCount - prevRainCount) * tipValue;
  }
  prevRainCount = currentRainCount;

  Serial.print("Rain Gauge: ");
  Serial.print(rain, 4);
  Serial.println(" mm");

  while (Wire.available())
  {
    byte msb = Wire.read();
    byte lsb = Wire.read();
    receivedWindCount = (msb << 8) | lsb;
  }
  currentWindCount = receivedWindCount;
  if ((currentWindCount - prevWindCount) > -1)
  {
    REV = (currentWindCount - prevWindCount);
  }
  else
  {
    REV = (65355 + currentWindCount - prevWindCount);
  }

  windspeed = (2 * PI * radius * REV * 3.6) / (period * 1000);
  prevWindCount = currentWindCount;
  Serial.print("Wind Speed: ");
  Serial.print(windspeed, 4);
  Serial.println(" km/h ");
}

void appendFile(fs::FS &fs, String path, String message)
{
  SerialMon.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file)
  {
    SerialMon.println("Failed to open file for appending");
    return;
  }
  if (file.println(message))
  {
    SerialMon.println("Message appended");
  }
  else
  {
    SerialMon.println("Append failed");
  }
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
    if (file.println(message))
    {
      SerialMon.println(" >OK");
    }
    else
    {
      SerialMon.println(" >Failed");
    }
    return;
  }
  SerialMon.println(" >OK");
  file.close();
  SerialMon.println("File Closed");
}

String getFileName()
{
  DateTime now = rtc.now();
  char fileNameString[20]; // adjust the size as needed
  sprintf(fileNameString, "/%04d%02d%02d.csv", now.year(), now.month(), now.day());
  return String(fileNameString);
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

void ReadRawAngle()
{
  // 7:0 - bits
  Wire.beginTransmission(0x36); // connect to the sensor
  Wire.write(0x0D);             // figure 21 - register map: Raw angle (7:0)
  Wire.endTransmission();       // end transmission
  Wire.requestFrom(0x36, 1);    // request from the sensor

  while (Wire.available() == 0)
    ;                    // wait until it becomes available
  lowbyte = Wire.read(); // Reading the data after the request

  // 11:8 - 4 bits
  Wire.beginTransmission(0x36);
  Wire.write(0x0C); // figure 21 - register map: Raw angle (11:8)
  Wire.endTransmission();
  Wire.requestFrom(0x36, 1);

  while (Wire.available() == 0)
    ;
  highbyte = Wire.read();

  // 4 bits have to be shifted to its proper place as we want to build a 12-bit number
  highbyte = highbyte << 8;      // shifting to left
  rawAngle = highbyte | lowbyte; // int is 16 bits (as well as the word)
  degAngle = rawAngle * 0.087890625;

  SerialMon.print("Deg angle: ");
  SerialMon.println(degAngle, 2); // absolute position of the encoder within the 0-360 circle
}

void correctAngle()
{
  // recalculate angle
  correctedAngle = degAngle - startAngle; // this tares the position

  // correctedAngle = correctedAngle + 48.75;

  if (correctedAngle < 0) // if the calculated angle is negative, we need to "normalize" it
  {
    correctedAngle = correctedAngle + 360; // correction for negative numbers (i.e. -15 becomes +345)
  }
  SerialMon.print("Corrected angle: ");
  SerialMon.println(correctedAngle, 2); // print the corrected/tared angle
}

void getDirection()
{
  ReadRawAngle();
  correctAngle(); // make a reading so the degAngle gets updated
}

// Encryption function
void do_encrypt(String msg, String key_str, String iv_str, byte *cipher, char *b64)
{
  byte iv[16];
  // copy the iv_str content to the array.
  memcpy(iv, (byte *)iv_str.c_str(), 16);

  // use base64 encoder to encode the message content. It is optional step.
  int blen = base64_encode(b64, (char *)msg.c_str(), msg.length());

  // calculate the output size:
  aes.calc_size_n_pad(blen);
  // custom padding, in this case, we use zero padding:
  int len = aes.get_size();
  byte plain_p[len];
  for (int i = 0; i < blen; ++i)
    plain_p[i] = b64[i];
  for (int i = blen; i < len; ++i)
    plain_p[i] = '\0';

  // do AES-128-CBC encryption:
  int blocks = len / 16;
  aes.set_key((byte *)key_str.c_str(), 16);
  aes.cbc_encrypt(plain_p, cipher, blocks, iv);

  // use base64 encoder to encode the encrypted data:
  base64_encode(b64, (char *)cipher, len);
  SerialMon.println("Encrypted Data output: " + String((char *)b64));
}

String getTime()
{
  DateTime now = rtc.now();
  String timeString = String(now.year(), DEC) + "-" +
                      String(now.month(), DEC) + "-" +
                      String(now.day(), DEC) + " " +
                      String(now.hour(), DEC) + ":" +
                      String(now.minute(), DEC) + ":" +
                      String(now.second(), DEC);
  return timeString;
}

void setup()
{
  // Start Serial Monitor
  SerialMon.begin(115200);
  delay(10); // feedback buffer

  // Enable Battery
  pinMode(BAT_EN, OUTPUT);
  digitalWrite(BAT_EN, HIGH);

  // Sensors Status
  SerialMon.println("\n========================================Sensors Status========================================");
  Wire.begin(21, 22);

  // BME Connect
  SerialMon.print("BME280: ");
  bool BMEstatus;
  BMEstatus = bme.begin(0x76);
  if (!BMEstatus)
  {
    SerialMon.println(" Failed");
  }
  else
  {
    SerialMon.println(" OK");
  }
  delay(10);

  // DHT Connect
  SerialMon.print("DHT22: ");
  dht.begin();
  if (isnan(h))
  {
    SerialMon.println(" Failed");
  }
  else
  {
    SerialMon.println(" OK");
  }
  delay(10);

  // BMP180 Connect
  SerialMon.print("BMP180: ");
  bool BMPStatus;
  BMPStatus = bmp.begin();
  if (!BMPStatus)
  {
    SerialMon.println(" Failed");
  }
  else
  {
    SerialMon.println(" OK");
  }
  delay(10);

  // BH1750 Connect
  SerialMon.print("BH1750: ");
  bool BHStatus;
  BHStatus = lightMeter.begin();
  if (!BHStatus)
  {
    SerialMon.println(" Failed");
  }
  else
  {
    SerialMon.println(" OK");
  }
  delay(10);

  // UV Connect
  SerialMon.print("UV: ");
  if (isnan(sensorValue))
  {
    SerialMon.println(" Failed");
  }
  else
  {
    SerialMon.println(" OK");
  }
  delay(10);

  // Slave Connect
  SerialMon.print("Slave: ");
  // Wire.begin(21, 22);
  bool SlaveStatus;
  SlaveStatus = Wire.begin();
  if (!SlaveStatus)
  {
    SerialMon.println(" Failed");
  }
  else
  {
    SerialMon.println(" OK");
  }
  delay(10);

  // Get Data
  SerialMon.println("");
  getBME();
  getDHT();
  getBMP();
  getBH();
  getUV();
  getSlave();
  getDirection();

  // Redundancy
  SerialMon.println("\n========================================Solving for Redundancy========================================");
  FTemp = (temperature + t) / 2;
  FHum = (humidity + h) / 2;
  FPres = (pressure + p) / 2;
  SerialMon.println(String("Temperature: ") + FTemp);
  SerialMon.println(String("Humidity: ") + FHum);
  SerialMon.println(String("Pressure: ") + FPres);

  // Start SD Card
  SerialMon.println("\n========================================SD Card Initializing========================================");
  SerialMon.print("Connecting to SD Card...");
  spi.begin(SCK, MISO, MOSI, CS);
  if (!SD.begin(CS, spi))
  {
    SerialMon.println(" >Failed. Skipping SD Storage");
  }
  else
  {
    SerialMon.println(" >OK");
    SerialMon.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
    SerialMon.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));
    float sdUsage = (float)SD.usedBytes() / SD.totalBytes() * 100;
    SerialMon.printf("SD usage: %.2f%%\n", sdUsage);

    String filename = getFileName();
    String datetime = getTime();
    sprintf(data, ",%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f",
            FHum, FTemp, FPres, lux, irradiance, sensorValue, rain, windspeed, correctedAngle, hi);
    String log = datetime + data;

    createHeader(SD, filename, "Time,Date,Humidity,Temperature,Pressure,Lux,Irradiance,UV,Precipitation,WindSpeed,WindDirection,HeatIndex");
    appendFile(SD, filename, log);

    SerialMon.print("Data Logged: ");
    SerialMon.println(log);
  }

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
  /*
  2 Automatic
  13 GSM Only
  14 WCDMA Only
  38 LTE Only
  */
  SerialMon.print("Selecting Network Mode...");
  String result;
  result = modem.setNetworkMode(2);
  if (modem.waitResponse(10000L) == 1)
  {
    SerialMon.println(" >OK");
    return;
  }
  SerialMon.println(" >Failed");

  // Setting the SSL cert to the secure_layer obj
  // secure_layer.setCACert(root_ca);
}

void loop()
{
  // APN Connect
  SerialMon.println("\n========================================Connecting to APN========================================");
  SerialMon.print("Connecting to ");
  SerialMon.print(apn);

  if (!modem.gprsConnect(apn, gprsUser, gprsPass))
  {
    SerialMon.println(" >Failed");
  }
  else
  {
    SerialMon.println(" >OK");

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
        Serial.println("Success to connect to network.");
      }

      // Check GPRS connection status
      if (modem.isGprsConnected())
      {
        Serial.println("Success to connect to GPRS.");
      }
      // Variables that contain parameters
      float T1 = temperature; // BME
      float T2 = t;           // BMP
      float H1 = humidity;    // BME
      float H2 = h;           // DHT
      float P1 = pressure;    // BME
      float P2 = p;           // BMP

      String lightdata = String(lux);
      String irradiancedata = String(irradiance);
      String uvintensitydata = String(sensorValue);
      String winddirectiondata = String(correctedAngle);
      String HeatIndexdata = String(hi);
      String windspeeddata = String(windspeed);
      String raingaugedata = String(rain);

      SerialMon.println("====================================== Encryption =========================================================");

      // Print readings results
      SerialMon.println("T1 => " + String(T1));
      SerialMon.println("T2 => " + String(T2));
      SerialMon.println("H1 => " + String(H1));
      SerialMon.println("H2 => " + String(H2));
      SerialMon.println("P1 => " + String(P1));
      SerialMon.println("P2 => " + String(P2));

      SerialMon.println("lightdata => " + String(lightdata));
      SerialMon.println("irradiancedata => " + String(irradiancedata));
      SerialMon.println("uvintensitydata => " + String(uvintensitydata));
      SerialMon.println("winddirectiondata => " + String(winddirectiondata));
      SerialMon.println("heatindexdata => " + String(HeatIndexdata));
      SerialMon.println("windspeeddata => " + String(windspeeddata));
      SerialMon.println("rainguagedata => " + String(raingaugedata));

      // Encrypt data
      do_encrypt(String(T1), key_str, iv_str, encryptedTemp1, b64Temp1);
      do_encrypt(String(T2), key_str, iv_str, encryptedTemp2, b64Temp2);
      do_encrypt(String(H1), key_str, iv_str, encryptedHum1, b64Hum1);
      do_encrypt(String(H2), key_str, iv_str, encryptedHum2, b64Hum2);
      do_encrypt(String(P1), key_str, iv_str, encryptedPress1, b64Press1);
      do_encrypt(String(P2), key_str, iv_str, encryptedPress2, b64Press2);

      do_encrypt(String(lightdata), key_str, iv_str, encryptedlight, b64Light);
      do_encrypt(String(irradiancedata), key_str, iv_str, encryptedirradiance, b64Irradiance);
      do_encrypt(String(uvintensitydata), key_str, iv_str, encryptedUVIntensity, b64UVIntensity);
      do_encrypt(String(winddirectiondata), key_str, iv_str, encryptedWindDirection, b64WindDirection);
      do_encrypt(String(HeatIndexdata), key_str, iv_str, encryptedHeatIndex, b64HeatIndex);
      do_encrypt(String(windspeeddata), key_str, iv_str, encryptedWindSpeed, b64Windspeed);
      do_encrypt(String(raingaugedata), key_str, iv_str, encryptedRainGauge, b64RainGuage);

      String encryptedB64Temp1 = String((char *)b64Temp1);
      String encryptedB64Temp2 = String((char *)b64Temp2);
      String encryptedB64Hum1 = String((char *)b64Hum1);
      String encryptedB64Hum2 = String((char *)b64Hum2);
      String encryptedB64Press1 = String((char *)b64Press1);
      String encryptedB64Press2 = String((char *)b64Press2);

      String encryptedB64Light = String((char *)b64Light);
      String encryptedB64Irradiance = String((char *)b64Irradiance);
      String encryptedB64UVIntensity = String((char *)b64UVIntensity);
      String encryptedB64WindDirection = String((char *)b64WindDirection);
      String encryptedB64HeatIndex = String((char *)b64HeatIndex);
      String encryptedB64Windspeed = String((char *)b64Windspeed);
      String encryptedB64RainGuage = String((char *)b64RainGuage);

      // Making an HTTP POST request

        // Start RTC
      SerialMon.println("\n========================================RTC Initializing========================================");
      // Start RTC
      SerialMon.print("Connecting to RTC...");
      if (!rtc.begin())
      {
        Serial.println("Couldn't connect to RTC");
      }
      else
      {

        // Set RTC Time when power is lost
        if (rtc.lostPower())
        {
          Serial.println("RTC lost power, let's set the time!...");
          rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
        }

        // Show Time
        SerialMon.print("Time is ");
        Time = getTime();
        Serial.println(Time);
        delay(10);
      }

      SerialMon.println("\n========================================HTTP Post Request========================================");
      SerialMon.println("Performing HTTP POST request...");

      SerialMon.println("Connecting to ");
      SerialMon.println(server);

      SerialMon.println("Making POST request securely");
      String contentType = "Content-Type: application/json";

      String postData = "{\"esp32_id\":\"ktrack_sibacan\",\"key\":\"A197C\",\"timeRec\":\"" + Time + "\" ,\"serial\":\"5bb2ah\",\"loc\":\"Sibacan, Balanga, Bataan\",\"light\" :\" " + encryptedB64Light + " \",\"irradiance\": \" " + encryptedB64Irradiance + " \" ,\"uvIntensity\": \" " + encryptedB64UVIntensity + " \" ,\"windDirection\": \" " + encryptedB64WindDirection + " \" ,\"windSpeed\":" + " \" " + encryptedB64Windspeed + " \" ,\"rainfall\":" + " \" " + encryptedB64RainGuage + " \" ,\"heatindexdata\":" + " \" " + encryptedB64HeatIndex + " \" " + ",\"T1\":" + " \" " + encryptedB64Temp1 + " \" ,\"T2\": \" " + encryptedB64Temp2 + " \" ,\"H1\": \" " + encryptedB64Hum1 + " \" , \"H2\": \" " + encryptedB64Hum2 + " \" ,\"P1\": \" " + encryptedB64Press1 + " \" ,\"P2\": \" " + encryptedB64Press2 + " \" " + "}";
      // String postData = "{\"key\":\"A197C\",\"loc\":\"Pto. Rivas Ibaba, Balanga City, Bataan\",\"timeRec\":\""+ Time+"\",\"light\" :\" " + encryptedB64Light + " \",\"irradiance\": \" " + encryptedB64Irradiance + " \" ,\"uvIntensity\": \" " + encryptedB64UVIntensity + " \" ,\"windDirection\": \" " + encryptedB64WindDirection + " \" ,\"windSpeed\":"+ " \" " + encryptedB64Windspeed+" \" ,\"rainGuage\":"+ " \" " + encryptedB64RainGuage +" \" ,\"heatindexdata\":"+ " \" " + encryptedB64HeatIndex +" \" " +",\"T1\":" + " \" " + encryptedB64Temp1 +" \" ,\"T2\": \" " + encryptedB64Temp2 +" \" ,\"H1\": \" " +  encryptedB64Hum1 + " \" , \"H2\": \" " + encryptedB64Hum2 +" \" ,\"P1\": \" "+encryptedB64Press1+" \" ,\"P2\": \" " + encryptedB64Press2 +" \" "+"}";

      SerialMon.println("");
      SerialMon.println("\n=========================================POST Data ============================================");
      SerialMon.println(postData);

      int postDataLength = postData.length();
      client.sendHeader("Content-Length", postDataLength);
      client.sendHeader("Connection", "Close");
      int posting = client.post(resource, contentType, postData);
      SerialMon.print("Reply:");
      SerialMon.print(posting);
      int status_code = client.responseStatusCode();
      String response = client.responseBody();

      SerialMon.print("Status code: ");
      SerialMon.println(status_code);
      SerialMon.print("Response: ");
      SerialMon.println(response);
      // int err = client.post(resource, contentType, postData);
      // if (err != 0)
      // {
      //   SerialMon.printf("Connection to %s failed, error code: %d\n", server, err);
      //   return;
      // }

      // // Wait for the server's response
      // int statusCode = client.responseStatusCode();
      // if (statusCode < 0)
      // {
      //   SerialMon.printf("Failed to get a response, error code: %d\n", statusCode);
      //   return;
      // }

      // // Print the response status code
      // SerialMon.printf("Response status code: %d\n", statusCode);

      // // Read and print the response body
      // String response = client.responseBody();
      // SerialMon.println("Response:");
      // SerialMon.println(response);
      // Close client and disconnect
      SerialMon.println("\n========================================Closing Client========================================");
      client.stop();
      SerialMon.println(F("Server disconnected"));
      modem.gprsDisconnect();
      SerialMon.println(F("GPRS disconnected"));
    }
  }
  // Set Timer and Sleep
  esp_sleep_enable_timer_wakeup(sleeptimer(millis(), TIME_TO_SLEEP) * uS_TO_S_FACTOR);
  esp_deep_sleep_start();
}
