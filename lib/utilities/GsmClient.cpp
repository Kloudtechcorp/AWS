#define TINY_GSM_MODEM_SIM7600

// GSM RX / TX Buffer - GSM Initiation
#if !defined(TINY_GSM_RX_BUFFER)
#define TINY_GSM_RX_BUFFER 1024
#endif
#define TINY_GSM_YIELD() \
    {                    \
        delay(2);        \
    }

#include <ArduinoHttpClient.h>
#include <SSLClient.h>
#include <TinyGsmClient.h>
#include <WString.h>

#include "GsmClient.h"

#define SerialMon Serial

#define UART_BAUD 115200
#define RESET_PIN 5
#define POWER_PIN 4
#define TX_PIN 26
#define RX_PIN 27

const int MAX_RETRIES_APN = 5;
const int MAX_RETRIES_SERVER = 10;

const int TIME_THRESHOLD = 70; // Allow up to 70s jump (for 1-min sleep)

HardwareSerial serialAt(1);

TinyGsm modem(serialAt);
TinyGsmClient baseClient(modem, 0);
SSLClient secureClient(&baseClient);
HttpClient client = HttpClient(secureClient, SERVER_ADDRESS, SERVER_PORT);

uint32_t autoBaud()
{
    static uint32_t rates[] = {115200, 9600, 57600, 38400, 19200, 74400, 74880,
                               230400, 460800, 2400, 4800, 14400, 28800};

    for (uint8_t i = 0; i < sizeof(rates) / sizeof(rates[0]); i++)
    {
        uint32_t rate = rates[i];

        // Serial.printf("Trying baud rate %u\n", rate);
        serialAt.updateBaudRate(rate);

        delay(10);

        for (int j = 0; j < 10; j++)
        {
            serialAt.print("AT\r\n");
            String input = serialAt.readString();
            if (input.indexOf("OK") >= 0)
            {
                // Serial.printf("Modem responded at rate:%u\n", rate);
                return rate;
            }
        }
    }

    serialAt.updateBaudRate(115200);
    return 0;
}

GsmClient::GsmClient(const char *apn, const char *deviceSerial) : _apn(apn), _deviceSerial(deviceSerial)
{
    SerialMon.println("\n=================================== GSM Initializing ===================================");
    SerialMon.print("Starting GSM...");

    // A7670-GSM Reset
    pinMode(RESET_PIN, OUTPUT);
    digitalWrite(RESET_PIN, LOW);
    delay(100);
    digitalWrite(RESET_PIN, HIGH);
    delay(100);
    digitalWrite(RESET_PIN, LOW);
    delay(100);

    // A7670-GSM Power
    pinMode(POWER_PIN, OUTPUT);
    digitalWrite(POWER_PIN, LOW);
    delay(100);
    digitalWrite(POWER_PIN, HIGH);
    delay(100);
    digitalWrite(POWER_PIN, LOW);
    delay(100);
    SerialMon.println(" >OK");

    SerialMon.print("Starting Serial Communications...");
    serialAt.begin(UART_BAUD, SERIAL_8N1, RX_PIN, TX_PIN);
    SerialMon.println(" >OK");

    SerialMon.print("Initializing modem...");
    if (!modem.init())
    {
        SerialMon.println(" >Failed (Restarting in 10s)");
        return;
    }
    SerialMon.println(" >OK");

    autoBaud();
}

GsmClient::~GsmClient()
{
    disconnect();
}

void GsmClient::apnConnect()
{
    SerialMon.println("\n=================================== Connecting to APN ===================================");
    SerialMon.printf("Connecting to %s", _apn);

    while (!_isApnConnected && _apnRetryCount < MAX_RETRIES_APN)
    {
        if (!modem.gprsConnect(_apn))
        {
            _apnCommunicationStatus = "Failed";
            SerialMon.print(".");
            _apnRetryCount++;
            delay(15000);
        }
        else
        {
            SerialMon.println(" >OK");
            _apnCommunicationStatus = "Success";
            _isApnConnected = true;
        }
    }
}

void GsmClient::serverConnect()
{
    SerialMon.println("\n=================================== Connecting to Server ===================================");
    SerialMon.printf("Connecting to %s", SERVER_ADDRESS);

    while (_isApnConnected && !_isServerConnected && _serverRetryCount < MAX_RETRIES_SERVER)
    {
        if (!baseClient.connect(SERVER_ADDRESS, SERVER_PORT))
        {
            SerialMon.print(".");
            _serverRetryCount++;
            delay(1000);
        }
        else
        {
            SerialMon.println(" >OK");
            _isServerConnected = true;
        }
    }
}

void GsmClient::disconnect()
{
    SerialMon.println("\n========================================Closing Client========================================");
    client.stop();
    SerialMon.println(F("Server disconnected"));
    modem.gprsDisconnect();
    SerialMon.println(F("GPRS disconnected"));
}

// Converts date/time to UNIX timestamp
unsigned long convertToEpoch(String year, String month, String day, String hour, String minute, String second)
{
    struct tm t;
    t.tm_year = year.toInt() + 2000 - 1900;
    t.tm_mon = month.toInt() - 1;
    t.tm_mday = day.toInt();
    t.tm_hour = hour.toInt();
    t.tm_min = minute.toInt();
    t.tm_sec = second.toInt();
    return mktime(&t);
}

void GsmClient::updateDateTime()
{
    serialAt.print("AT+CCLK?\r\n");
    delay(100);
    String response = serialAt.readString();

    if (response == "")
        return;

    int startIndex = response.indexOf("+CCLK: \"");
    int endIndex = response.indexOf("\"", startIndex + 8);
    if (startIndex == -1 || endIndex == -1)
        return; // Invalid response
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
    if (plusIndex != -1)
    {
        second = second.substring(0, plusIndex);
    }
    // dateTime = "20" + year + "-" + month + "-" + day + " " + hour + ":" + minute + ":" + second;
    // fileName = "/" + month + day + "20" + year + ".csv";

    // Convert to epoch time
    unsigned long newEpoch = convertToEpoch(year, month, day, hour, minute, second);

    // Filtering: Ignore large jumps
    if (_lastEpoch == 0 || abs((long)newEpoch - (long)_lastEpoch) <= TIME_THRESHOLD)
    {
        _lastEpoch = newEpoch;
        _dateTime = DateTime("20" + year, month, day, hour, minute, second);
    }
    else
    {
        Serial.print("Time jump detected (");
        Serial.print(abs((long)newEpoch - (long)_lastEpoch));
        Serial.println("s), ignoring...");
    }
}

void GsmClient::sendData(String postData)
{
    SerialMon.println("\n========================================HTTP Post Request========================================");
    SerialMon.println("Performing HTTP POST request...");
    client.connectionKeepAlive();
    SerialMon.printf("Connecting to %s\n", SERVER_ADDRESS);

    SerialMon.println("Making POST request securely");

    SerialMon.println("\n=========================================POST Data ============================================");
    SerialMon.println(postData);

    client.beginRequest();
    client.post(RESOURCE_PATH_PREFIX + _deviceSerial);
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
