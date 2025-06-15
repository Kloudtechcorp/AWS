#pragma once

#include <WString.h>

#include "DateTime.h"

const char SERVER_ADDRESS[] = "app.kloudtechsea.com";
const int SERVER_PORT = 443;
const String RESOURCE_PATH_PREFIX = "/api/v1/weather/insert-data?serial=";

class BaseClient
{
public:
    /// Connects to the server.
    virtual void connect() = 0;

    /// Disconnects from the server.
    virtual void disconnect() = 0;

    /// Returns true if connected to the server.
    virtual bool isConnected() = 0;

    /// Returns a human-readable status of the last communication attempt.
    virtual String getCommunicationStatus() = 0;

    /// Updates the current date and time.
    virtual void updateDateTime() = 0;

    /// Returns the current date and time.
    ///
    /// Make sure to call updateDateTime() first to refresh the time.
    virtual DateTime getDateTime() = 0;

    /// Sends data to the server.
    virtual void sendData(String postData) = 0;
};
