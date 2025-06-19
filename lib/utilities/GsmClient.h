#pragma once

#include <WString.h>

#include "BaseClient.h"
#include "DateTime.h"

class GsmClient : public BaseClient
{
private:
    const char *_apn;
    const char *_deviceSerial;

    bool _isApnConnected = false;
    int _apnRetryCount = 0;

    bool _isServerConnected = false;
    int _serverRetryCount = 0;

    String _apnCommunicationStatus = "";

    unsigned long _lastEpoch = 0;
    DateTime _dateTime;

    void apnConnect();
    void serverConnect();

public:
    GsmClient(const char *apn, const char *deviceSerial);
    ~GsmClient();

    virtual void connect()
    {
        apnConnect();
        serverConnect();
    }

    virtual void disconnect();

    virtual bool isConnected()
    {
        return _isApnConnected && _isServerConnected;
    }

    virtual String getCommunicationStatus()
    {
        return _apnCommunicationStatus;
    }

    virtual void updateDateTime();

    virtual DateTime getDateTime() { return _dateTime; }

    virtual void sendData(String postData);
};
