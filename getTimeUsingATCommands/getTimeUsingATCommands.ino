#include "utilities.h"
#include "Arduino.h"

String response= "";

uint32_t AutoBaud()
{
    static uint32_t rates[] = {115200, 9600, 57600,  38400, 19200,  74400, 74880,
                               230400, 460800, 2400,  4800,  14400, 28800
                              };
    for (uint8_t i = 0; i < sizeof(rates) / sizeof(rates[0]); i++) {
        uint32_t rate = rates[i];
        Serial.printf("Trying baud rate %u\n", rate);
        SerialAT.updateBaudRate(rate);
        delay(10);
        for (int j = 0; j < 10; j++) {
            SerialAT.print("AT\r\n");
            String input = SerialAT.readString();
            if (input.indexOf("OK") >= 0) {
                Serial.printf("Modem responded at rate:%u\n", rate);
                return rate;
            }
        }
    }
    SerialAT.updateBaudRate(115200);
    return 0;
}

void setup()
{
    Serial.begin(115200); // Set console baud rate
    SerialAT.begin(115200, SERIAL_8N1, MODEM_RX_PIN, MODEM_TX_PIN);
    Serial.println("Starting Void Setup");
    
#ifdef BOARD_POWERON_PIN
    pinMode(BOARD_POWERON_PIN, OUTPUT);
    digitalWrite(BOARD_POWERON_PIN, HIGH);
#endif

    // Set modem reset pin ,reset modem
    pinMode(MODEM_RESET_PIN, OUTPUT);
    digitalWrite(MODEM_RESET_PIN, !MODEM_RESET_LEVEL); delay(100);
    digitalWrite(MODEM_RESET_PIN, MODEM_RESET_LEVEL); delay(100);
    digitalWrite(MODEM_RESET_PIN, !MODEM_RESET_LEVEL);

    pinMode(BOARD_PWRKEY_PIN, OUTPUT);
    digitalWrite(BOARD_PWRKEY_PIN, LOW); delay(100);
    digitalWrite(BOARD_PWRKEY_PIN, HIGH); delay(100);
    digitalWrite(BOARD_PWRKEY_PIN, LOW);

    if (AutoBaud()) {
        Serial.println(F(" You can now send AT commands"));
    } else {
        Serial.println(F(" Failed to connect to the modem! Check the baud and try again."));
    }
}

void loop()
{
  Serial.println("Starting Void Loop");
  response = ""; // clear the response variable
  SerialAT.print("AT+CCLK?\r\n"); // send the AT command again
  delay(100); // wait for the response
  response = SerialAT.readString(); // store the new response
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

    Serial.println(day + "/" + month + "/" + year);
    Serial.println(hour + ":" + minute + ":" + second);
    Serial.println();
  }
  delay(5000); // wait 5 seconds before printing again
}