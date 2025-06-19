#pragma once

#include <WString.h>

class DateTime
{
private:
public:
    String year;
    String month;
    String day;
    String hour;
    String minute;
    String second;

    DateTime();
    DateTime(String year, String month, String day, String hour, String minute, String second);
    DateTime(int year, int month, int day, int hour, int minute, int second);

    String toString();
    String toFileName();
};
