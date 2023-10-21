/**
 * Simple digital watch example
 * M. Samek, 01/07/00
 */
#include "hsm.hpp"

#include <stdio.h>
#include <assert.h>

class Watch : public Hsm {
protected:
    State timekeeping, time, date;
    State setting, hour, minute, day, month;
    State *timekeepingHist;
private:
    int tsec, tmin, thour, dday, dmonth;

public:
    Watch();
    Msg const *topHndlr(Msg const *msg);
    Msg const *timekeepingHndlr(Msg const *msg);
    Msg const *timeHndlr(Msg const *msg);
    Msg const *dateHndlr(Msg const *msg);
    Msg const *settingHndlr(Msg const *msg);
    Msg const *hourHndlr(Msg const *msg);
    Msg const *minuteHndlr(Msg const *msg);
    Msg const *dayHndlr(Msg const *msg);
    Msg const *monthHndlr(Msg const *msg);
    void tick();
    void showTime();
    void showDate();
};

enum WatchEvents {
    Watch_MODE_EVT,
    Watch_SET_EVT,
    Watch_TICK_EVT
};

// lookup table for the days of a month
static int const day_of_month_lut[] = {
    0, /* unused month #0 */
    31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

void Watch::showTime() {
    printf("time: %2d:%02d:%02d", thour, tmin, tsec);
}

void Watch::showDate() {
    printf("date: %02d-%02d", dmonth, dday);
}

void Watch::tick() {
    if (++tsec == 60) {
        tsec = 0;
        if (++tmin == 60) {
            tmin = 0;
            if (++thour == 24) {
                thour = 0;
                if (++dday > day_of_month_lut[dmonth]) {
                    dday = 1;
                    if (++dmonth == 13) {
                        dmonth = 1;
                    }
                }
            }
        }
    }
}

Msg const *Watch::topHndlr(Msg const *msg) {
    switch (msg->evt) {
    case START_EVT:
        STATE_START(&setting);
        return 0;
    }
    return msg;
}

Msg const *Watch::timekeepingHndlr(Msg const *msg) {
    switch (msg->evt) {
    case START_EVT:
        STATE_START(timekeepingHist);
        return 0;
    case EXIT_EVT:
        timekeepingHist = STATE_CURR();
        return 0;
    case Watch_SET_EVT:
        STATE_TRAN(&setting);
        printf("Watch::timekeeping-SET;");
        return 0;
    case Watch_TICK_EVT:
        tick();
        printf("Watch::timekeeping-TICK;");
        return 0;
    }
    return msg;
}

Msg const *Watch::timeHndlr(Msg const *msg) {
    switch (msg->evt) {
    case ENTRY_EVT:
        showTime();
        return 0;
    case Watch_MODE_EVT:
        STATE_TRAN(&date);
        printf("Watch::time-MODE;");
        return 0;
    case Watch_TICK_EVT:
        printf("Watch::time-TICK;");
        tick();
        showTime();
        return 0;
    }
    return msg;
}

Msg const *Watch::dateHndlr(Msg const *msg) {
    switch (msg->evt) {
    case ENTRY_EVT:
        showDate();
        return 0;
    case Watch_MODE_EVT:
        STATE_TRAN(&time);
        printf("Watch::date-MODE;");
        return 0;
    case Watch_TICK_EVT:
        printf("Watch::date-TICK;");
        tick();
        showDate();
        return 0;
    }
    return msg;
}

Msg const *Watch::settingHndlr(Msg const *msg) {
    switch (msg->evt) {
    case START_EVT:
        STATE_START(&hour);
        printf("Watch::setting-START->hour;");
        return 0;
    }
    return msg;
}

Msg const *Watch::hourHndlr(Msg const *msg) {
    switch (msg->evt) {
    case Watch_MODE_EVT:
        if (++thour == 60) {
            thour = 0;
        }
        showTime();
        return 0;
    case Watch_SET_EVT:
        STATE_TRAN(&minute);
        printf("Watch::hour-SET;");
        return 0;
    }
    return msg;
}

Msg const *Watch::minuteHndlr(Msg const *msg) {
    switch (msg->evt) {
    case Watch_MODE_EVT:
        if (++tmin == 60) {
            tmin = 0;
        }
        showTime();
        return 0;
    case Watch_SET_EVT:
        STATE_TRAN(&day);
        return 0;
    }
    return msg;
}

Msg const *Watch::dayHndlr(Msg const *msg) {
    switch (msg->evt) {
    case Watch_MODE_EVT:
        if (++dday > day_of_month_lut[dmonth]) {
            dday = 1;
        }
        showDate();
        return 0;
    case Watch_SET_EVT:
        STATE_TRAN(&month);
        printf("Watch::day-SET;");
        return 0;
    }
    return msg;
}

Msg const *Watch::monthHndlr(Msg const *msg) {
    switch (msg->evt) {
    case Watch_MODE_EVT:
        if (++dmonth > 12 ) {
            dmonth = 1;
        }
        showDate();
        return 0;
    case Watch_SET_EVT:
        STATE_TRAN(&timekeeping);
        printf("Watch::month-SET;");
        return 0;
    }
    return msg;
}

Watch::Watch()
  : Hsm("Watch", reinterpret_cast<EvtHndlr>(&topHndlr)),
    timekeeping("timekeeping", &top, reinterpret_cast<EvtHndlr>(&timekeepingHndlr)),
    time("time",       &timekeeping, reinterpret_cast<EvtHndlr>(&timeHndlr)),
    date("date",       &timekeeping, reinterpret_cast<EvtHndlr>(&dateHndlr)),
    setting("setting", &top,         reinterpret_cast<EvtHndlr>(&settingHndlr)),
    hour("hour",       &setting,     reinterpret_cast<EvtHndlr>(&hourHndlr)),
    minute("minute",   &setting,     reinterpret_cast<EvtHndlr>(&minuteHndlr)),
    day("day",         &setting,     reinterpret_cast<EvtHndlr>(&dayHndlr)),
    month("month",     &setting,     reinterpret_cast<EvtHndlr>(&monthHndlr)),
    tsec(0), tmin(0), thour(0), dday(1), dmonth(1)
{
    timekeepingHist = &time;
}

const Msg watchMsg[] = {
    Watch_MODE_EVT,
    Watch_SET_EVT,
    Watch_TICK_EVT
};

int main() {
    Watch watch;

    printf("Events:\n"
           "0 for MODE_EVT\n"
           "1 for SET_EVT\n"
           "2 for TICK_EVT\n"
           "3+ to exit\n\n");

    watch.onStart();
    for (;;)  {
        char c;
        printf("\nEvent<-");
        c = getc(stdin);
        getc(stdin);
        if (c < '0' || '2' < c) {
            break;
        }
        watch.onEvent(&watchMsg[c - '0']);
    }
    return 0;
}
