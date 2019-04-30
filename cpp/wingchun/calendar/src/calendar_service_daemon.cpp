//
// Created by PolarAir on 2019-02-28.
//

#include "calendar/include/calendar_service.h"
using namespace kungfu;
int main()
{
    CalendarService service;
    service.run();
    service.join();

    return 0;
}
