#pragma once

#include <WString.h>

#include "BaseClient.h"
#include "DateTime.h"

class WifiClient : public BaseClient
{
private:
    const char *_ssid;
    const char *_password;
    const char *_deviceSerial;

    bool _isConnected = false;
    int _retryCount = 0;

    String _communicationStatus = "";

    DateTime _dateTime;

public:
    WifiClient(const char *ssid, const char *password, const char *deviceSerial);
    ~WifiClient();

    virtual void connect();

    virtual void disconnect();

    virtual bool isConnected()
    {
        return _isConnected;
    }

    virtual String getCommunicationStatus()
    {
        return _communicationStatus;
    }

    virtual void updateDateTime();

    virtual DateTime getDateTime() { return _dateTime; }

    virtual void sendData(String postData);
};
