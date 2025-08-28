#include <ArduinoHttpClient.h>
#include <HttpClient.h>
#include <NTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <WiFiUdp.h>

#include "WifiClient.h"

#define SerialMon Serial

const int MAX_RETRIES = 20;

WiFiClientSecure wifi;
HttpClient client = HttpClient(wifi, SERVER_ADDRESS, SERVER_PORT);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "asia.pool.ntp.org", 28800, 60000);

WifiClient::WifiClient(const char *ssid, const char *password, const char *deviceSerial) : _ssid(ssid), _password(password), _deviceSerial(deviceSerial)
{
}

WifiClient::~WifiClient()
{
    disconnect();
}

void WifiClient::connect()
{
    SerialMon.println("\n=================================== Connecting to WiFi ===================================");

    SerialMon.print("Connecting to ");
    SerialMon.println(_ssid);
    WiFi.mode(WIFI_STA);
    WiFi.begin(_ssid, _password);

    while (WiFi.status() != WL_CONNECTED && _retryCount < MAX_RETRIES)
    {
        delay(500);
        SerialMon.print(".");
        _retryCount++;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        _communicationStatus = "Success";
        _isConnected = true;
        SerialMon.println("\nWiFi connected!");
        SerialMon.print("IP Address: ");
        SerialMon.println(WiFi.localIP());
    }
    else
    {
        _communicationStatus = "Failed";
        SerialMon.println("\nWiFi not connected!");
    }

    timeClient.begin();
    timeClient.update();
}

void WifiClient::disconnect()
{
    SerialMon.println("\n========================================Closing Client========================================");
    client.stop();
    SerialMon.println(F("Server disconnected"));
}

void WifiClient::updateDateTime()
{
    timeClient.update();                                  // Update time from NTP server
    unsigned long unix_epoch = timeClient.getEpochTime(); // Get current epoch time
    // second_ = second(unix_epoch);  // Extract second from epoch time
    // if (last_second != second_)
    // {
    //   minute_ = minute(unix_epoch);  // Extract minute from epoch time
    //   hour_ = hour(unix_epoch);  // Extract hour from epoch time
    //   day_ = day(unix_epoch);  // Extract day from epoch time
    //   month_ = month(unix_epoch);  // Extract month from epoch time
    //   year_ = year(unix_epoch);  // Extract year from epoch time

    //   // Format and print NTP time on Serial monitor
    //   sprintf(d, "%02d-%02d-%02d %02d:%02d:%02d", year_, month_, day_, hour_, minute_, second_);
    //   dateTime = String(d);
    //   sprintf(f, "/%02d%02d%02d.csv", month_, day_, year_);
    //   fileName = String(f);
    // }

    // Convert Unix epoch time to readable format
    const struct tm *timeinfo;
    time_t rawtime = unix_epoch;
    timeinfo = localtime(&rawtime);

    if (timeinfo->tm_year >= 70)
    { // Avoid 1970 issue
        byte second_ = timeinfo->tm_sec;
        byte minute_ = timeinfo->tm_min;
        byte hour_ = timeinfo->tm_hour;
        byte day_ = timeinfo->tm_mday;
        byte month_ = timeinfo->tm_mon + 1;   // Month is 0-11 in struct tm
        int year_ = timeinfo->tm_year + 1900; // Year starts from 1900

        _dateTime = DateTime(year_, month_, day_, hour_, minute_, second_);

        Serial.print("Updated Time: ");
        Serial.println(_dateTime.toString());
    }
}

void WifiClient::sendData(String postData)
{
    wifi.setInsecure();
    SerialMon.println("\n=========================================Making POST request============================================");
    SerialMon.println(postData);

    client.beginRequest();
    client.post(RESOURCE_PATH_PREFIX + _deviceSerial);
    client.sendHeader("Content-Type", "application/json");
    client.sendHeader("Content-Length", postData.length());
    client.print(postData);
    client.endRequest();

    int status_code = client.responseStatusCode();
    String response = client.responseBody();
    SerialMon.printf("Status code: %d\n", status_code);
    SerialMon.println("Response: " + response);
}
