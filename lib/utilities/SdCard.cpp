#include <FS.h>
#include <SD.h>
#include <SPI.h>

#include "SdCard.h"

#define SerialMon Serial

#define SCK 14
#define MISO 2
#define MOSI 15
#define CS 13

SPIClass spi = SPIClass(VSPI);

SdCard::SdCard()
{
    SerialMon.println("=================================================");
    SerialMon.println("Connecting to SD Card...");
    spi.begin(SCK, MISO, MOSI, CS);
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
            SerialMon.printf("File %s exists", path);
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

void SdCard::logData(String path, String header, String data)
{
    SD.end();

    if (!SD.begin(CS, spi))
    {
        SerialMon.println(" >Failed. Skipping SD Storage");
        return;
    }

    SerialMon.println("SD Card Initialization Complete");

    createHeader(SD, path, header);
    appendFile(SD, path, data);

    SerialMon.println("Data logged successfully.");
}
