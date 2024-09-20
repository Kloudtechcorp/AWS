#include <Arduino.h>
#include <math.h>
#include <Wire.h>
#include <utilities.h>
#include <sensors.h>

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

String getUV()
{
  sensorValue = analogRead(uvPin);
  sensorVoltage = sensorValue * (3.3 / 4095);
  uvIntensity = sensorVoltage * 1000;
  uvIntensityStr = String(uvIntensity);
  return uvIntensityStr;
}

String getLight()
{
  lux = lightMeter.readLightLevel();
  lightStr = String(lux);
  return lightStr;
}

void ReadRawAngle()
{
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

void correctAngle()
{
  correctedAngle = 360 - degAngle + startAngle;
  if (correctedAngle > 360)
  {
    correctedAngle -= 360;
  }
  rtcCorrectAngle = correctedAngle;
  if (correctedAngle == 360)
  {
    correctedAngle = 0;
  }
}

String getDirection()
{
  ReadRawAngle();
  correctAngle();
  windDirStr = String(correctedAngle);
  return windDirStr;
}

void getSlave()
{
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
  // while (Wire.available())
  // {
  //   byte msb = Wire.read();
  //   byte lsb = Wire.read();
  //   receivedGustCount = (msb << 8) | lsb;
  // }
  // gust = (2 * PI * radius * receivedGustCount * 3.6) / (3 * 1000);
  gust = 0;
}

String getBatteryVoltage()
{
  batteryStr = String(ina219.getBusVoltage_V());
  return batteryStr;
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
  SerialMon.print("Starting GSM...");
  GSMinit();
  delay(3000);
  SerialMon.println(" >OK");

  SerialMon.print("Starting Serial Communications...");
  SerialAT.begin(UART_BAUD, SERIAL_8N1, PIN_RX, PIN_TX);
  SerialMon.println(" >OK");

  SerialMon.print("Initializing modem...");
  if (!modem.init())
  {
    SerialMon.println(" >Failed (Restarting in 10s)");
    delay(10000);
    modem.restart();
    GSMinit();
    return;
  }
  SerialMon.println(" >OK");
  AutoBaud();
  
  SerialMon.println("\n========================================Sensors Status========================================");
  Wire.begin(21, 22);

  // BME 1 Connect
  SerialMon.print("BME 1: ");
  bool BME1_status;
  select_bus(2);
  BME1_status = bme1.begin(0x76);
  if (!BME1_status)
  {
    SerialMon.println(" Failed");
  }
  else
  {
    SerialMon.println(" OK");
    getBME(bme1, 2, &t1, &h1, &p1);
    t1Str = String(t1);
    h1Str = String(h1);
    p1Str = String(p1);
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
  }
  else
  {
    SerialMon.println(" OK");
    getBME(bme2, 3, &t2, &h2, &p2);
    t2Str = String(t2);
    h2Str = String(h2);
    p2Str = String(p2);
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
  }
  else
  {
    SerialMon.println(" OK");
    getBME(bme3, 4, &t3, &h3, &p3);
    t3Str = String(t3);
    h3Str = String(h3);
    p3Str = String(p3);
  }
  delay(10);

  // BH1750 Connect
  SerialMon.print("BH1750: ");
  bool light_status;
  light_status = lightMeter.begin();
  if (!light_status)
  {
    SerialMon.println(" Failed");
  }
  else
  {
    SerialMon.println(" OK");
    getLight();
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

  if (uvPrevStatus != uvStatus)
  {
    SerialMon.println(" Failed");
  }
  else
  {
    SerialMon.println(" OK");
    getUV();
  }
  delay(10);

  // AS5600 Connect
  SerialMon.print("AS5600: ");
  startAngle - rtcStartAngle;
  ReadRawAngle();
  if (rtcStartAngle == 0) {
    rtcStartAngle = degAngle;
  }
  startAngle = rtcStartAngle;
  correctedAngle = rtcCorrectAngle;
  bool winddir_status;
  Wire.beginTransmission(0x36);
  winddir_status = (Wire.endTransmission() == 0);
  if (!winddir_status)
  {
    SerialMon.println(" Failed");
  }
  else
  {
    SerialMon.println(" OK");
    getDirection();
  }

  // Slave Connect
  SerialMon.print("Slave: ");
  bool Slave_status;
  Wire.beginTransmission(SLAVE);
  Slave_status = (Wire.endTransmission() == 0);
  if (!Slave_status)
  {
    SerialMon.println(" Failed");
  }
  else
  {
    SerialMon.println(" OK");
    getSlave();
    windSpeedStr = String(windspeed);
    rainStr = String(rain);
    gustStr = String(gust);
  }
  delay(10);

  // INA219 Connect
  SerialMon.print("INA219: ");
  bool battery_status;
  battery_status = ina219.begin();
  if (!battery_status)
  {
    SerialMon.println(" Failed");
  }
  else
  {
    SerialMon.println(" OK");
    getBatteryVoltage();
  }
  delay(10);

  // Start SD Card
  SerialMon.println("\n========================================SD Card Initializing========================================");
  SerialMon.println("Connecting to SD Card...");
  spi.begin(SCK, MISO, MOSI, CS);
  getTime();
  logDataToSDCard();
}

void loop()
{
  // APN Connect
  SerialMon.println("\n========================================Connecting to APN========================================");
  SerialMon.printf("Connecting to %s", apn);
  bool connectedAPN = false;
  int retryCountAPN = 0;
  const int maxRetriesAPN = 10;
  while (!connectedAPN && retryCountAPN < maxRetriesAPN) {
    if (!modem.gprsConnect(apn, gprsUser, gprsPass))
    {
      SerialMon.print(".");
      retryCountAPN++;
      delay(1000);
    }
    else
    {
      SerialMon.println(" >OK");
      connectedAPN = true;
    }
  }
  // Server Connect
  SerialMon.println("\n========================================Connecting to Server========================================");
  SerialMon.printf("Connecting to %s", server);
  bool connectedServer = false;
  int retryCountServer = 0;
  const int maxRetriesServer = 10;
  while (connectedAPN && !connectedServer && retryCountServer < maxRetriesServer)
  {
    if (!base_client.connect(server, port))
    {
      SerialMon.print(".");
      retryCountServer++;
      delay(1000);
    }
    else
    {
      SerialMon.println(" >OK");
      connectedServer = true;
    }
  }

  if (connectedAPN && connectedServer)
  {
    SerialMon.println("\n====================================== Print results =========================================================");

    // Print readings results
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

    SerialMon.println("\n========================================HTTP Post Request========================================");
    SerialMon.println("Performing HTTP POST request...");
    client.connectionKeepAlive();
    SerialMon.printf("Connecting to %s", server);

    SerialMon.println("Making POST request securely");
    String contentType = "Content-Type: application/json";

    String postData = "{\"recordedAt\":\"" + dateTime + "\", \"light\":\"" + lightStr + "\", \"uvIntensity\":\"" + uvIntensityStr + "\", \"windDirection\":\"" + windDirStr + "\", \"windSpeed\":\"" + windSpeedStr + "\", \"precipitation\":\"" + rainStr + "\", \"gust\":\"" + gustStr + "\", \"T1\":\"" + t1Str + "\", \"T2\":\"" + t2Str + "\", \"T3\":\"" + t3Str + "\", \"H1\":\"" + h1Str + "\", \"H2\":\"" + h2Str + "\", \"H3\":\"" + h3Str + "\", \"P1\":\"" + p1Str + "\", \"P2\":\"" + p2Str + "\", \"P3\":\"" + p3Str + "\", \"batteryVoltage\":\"" + batteryStr + "\"}";

    SerialMon.println("\n=========================================POST Data ============================================");
    SerialMon.println(postData);

    int postDataLength = postData.length();
    client.sendHeader("Content-Length", postDataLength);
    client.sendHeader("Connection", "Close");
    int posting = client.post(resource, contentType, postData);
    SerialMon.printf("Reply: %d\n", posting);
    int status_code = client.responseStatusCode();
    String response = client.responseBody();

    SerialMon.printf("Status code: %d\n", status_code);
    SerialMon.println("Response: " + response);

    SerialMon.println("\n========================================Closing Client========================================");
    client.stop();
    SerialMon.println(F("Server disconnected"));
    modem.gprsDisconnect();
    SerialMon.println(F("GPRS disconnected"));
  }
  SerialMon.print("\nSleeping...\n");
  // Set Timer and Sleep
  sleeptimer = 60000 - (millis() % 60000);
  esp_sleep_enable_timer_wakeup(sleeptimer * 1000);
  esp_deep_sleep_start();
}