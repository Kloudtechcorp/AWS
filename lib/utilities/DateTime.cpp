#include "DateTime.h"

DateTime::DateTime() : year(""), month(""), day(""), hour(""), minute(""), second("")
{
}

DateTime::DateTime(String year, String month, String day, String hour, String minute, String second)
    : year(year), month(month), day(day), hour(hour), minute(minute), second(second)
{
}

DateTime::DateTime(int year, int month, int day, int hour, int minute, int second) : year(String(year)), month(String(month)), day(String(day)), hour(String(hour)), minute(String(minute)), second(String(second))
{
}

String DateTime::toString()
{
    return year + "-" + month + "-" + day + " " + hour + ":" + minute + ":" + second;
}

String DateTime::toFileName()
{
    return "/" + month + day + year + ".csv";
}
