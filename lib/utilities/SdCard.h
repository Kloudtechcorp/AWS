#pragma once

#include <WString.h>

class SdCard
{
private:
public:
    SdCard();

    void logData(String path, String header, String data);
};
