#include "options.ih"

void Options::setSyslogFacility()
{
    d_syslogFacility = s_syslogFacilities.find(s_defaultSyslogFacility);

    string option;
    if (d_arg.option(&option, "syslog-facility"))
    {
        auto facility = s_syslogFacilities.find(option);

        if (facility != s_syslogFacilities.end())
            d_syslogFacility = facility;
        else
            d_syslogFacilityError = option;
    }            
}
