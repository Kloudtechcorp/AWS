#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <PageIndex.h>
#include <SPIFFS.h>

//----------------------------------------SSID and Password of your WiFi router
const char* ssid = "KT 2.4";
const char* password = "J@yGsumm!t";
//----------------------------------------

WebServer server(80);  // Web server on port 80

#define BME_SDA 21
#define BME_SCL 22
Adafruit_BME280 bme;

//----------------------------------------This routine is executed when you open ESP32 IP Address in browser
void handleRoot() {
  String s = MAIN_page; // Read HTML contents
  server.send(200, "text/html", s); // Send web page
}

void saveData(float data, const char* filename) {
  File file = SPIFFS.open(filename, FILE_APPEND);
  if (!file) {
    Serial.print("Failed to open ");
    Serial.print(filename);
    Serial.println(" for writing");
    return;
  }
  if (file.print(data)) {
    Serial.print("Data written to ");
    Serial.println(filename);
  } else {
    Serial.print("Write to ");
    Serial.print(filename);
    Serial.println(" failed");
  }
  
  file.println(); // Add newline to ensure proper line ending
  file.close();
}

String generateCSV() {
  String csv = "Temperature,Humidity\n";
    
  File tempFile = SPIFFS.open("/temperature.txt", FILE_READ);
  File humFile = SPIFFS.open("/humidity.txt", FILE_READ);

  if (!tempFile || !humFile) {
    Serial.println("Failed to open one of the data files");
    return csv;
  }

  while (tempFile.available() && humFile.available()) {
    float temp = tempFile.parseFloat();
    float hum = humFile.parseFloat();
    csv += String(temp) + "," + String(hum) + "\n";
  }

  tempFile.close();
  humFile.close();

  return csv;
}

void deleteFileContent(const char* filename) {
  File file = SPIFFS.open(filename, FILE_WRITE);
  if (!file) {
    Serial.print("Failed to open ");
    Serial.print(filename);
    Serial.println(" for writing");
    return;
  }

  file.close();
}

//----------------------------------------Procedure for reading the temperature value of a BME280 sensor
void handleBME280Temperature() {
  float t = bme.readTemperature();
  String Temperature_Value = String(t);
 
  server.send(200, "text/plain", Temperature_Value); // Send Temperature value only to client ajax request
  saveData(t, "/temperature.txt");  // Save temperature data to SPIFFS

  Serial.print("BME280 || Temperature : ");
  Serial.print(t);
  Serial.println(" °C");
}

//----------------------------------------Procedure for reading humidity values from BME280 sensor
void handleBME280Humidity() {
  float h = bme.readHumidity();
  String Humidity_Value = String(h);
 
  server.send(200, "text/plain", Humidity_Value); // Send Humidity value only to client ajax request
  saveData(h, "/humidity.txt");  // Save humidity data to SPIFFS

  Serial.print("BME280 || Humidity : ");
  Serial.print(h);
  Serial.println(" %");
}

void handleDownloadCSV() {
  String csv = generateCSV();
  server.sendHeader("Content-Disposition", "attachment; filename=data.csv");
  server.send(200, "text/csv", csv);

  // // Delete contents of temperature and humidity files after download
  // deleteFileContent("/temperature.txt");
  // deleteFileContent("/humidity.txt");
}

//----------------------------------------Setup
void setup(void) {
  Serial.begin(115200);
  delay(500);
  Serial.println("");

  if (!SPIFFS.begin(true)) {
    Serial.println("Failed to mount file system");
    return;
  }
  else {
    Serial.println("SPIFFS mounted succesfully...");
  }

  Wire.begin(BME_SDA, BME_SCL); // Initialize I2C communication for BME280
  if (!bme.begin(0x76)) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }
  else {
    Serial.println("BME Initiation Complete");
  }

  WiFi.begin(ssid, password); // Connect to your WiFi router
  Serial.println("");
  
  //----------------------------------------Wait for connection
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(250);
  }
  //----------------------------------------
  
  Serial.println("");
  Serial.print("Successfully connected to : ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  //----------------------------------------

  // Setup Web server routes
  server.on("/", handleRoot);
  server.on("/readTemperature", handleBME280Temperature);
  server.on("/readHumidity", handleBME280Humidity);
  server.on("/downloadCSV", handleDownloadCSV);

  // Start server
  server.begin();
  Serial.println("HTTP server started");
}

//----------------------------------------Loop
void loop() {
  server.handleClient();  // Handle client requests
}
