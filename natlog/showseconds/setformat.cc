#include "showseconds.ih"

void ShowSeconds::setFormat()
{
    Options::Time time = Options::instance().time();

    s_show = 
        time == Options::RAW ? rawTime :
                Options::UTC ? utcTime :
                               localTime;

    if (time == Options::UTC)
        s_utcMarker = " (UTC)";
}
