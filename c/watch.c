/**
 * Simple digital watch example
 * M. Samek, 01/07/00
 */
#include "hsm.h"

#include <stdio.h>
#include <assert.h>

typedef struct Watch Watch;
struct Watch {
    Hsm super;
    State timekeeping, time, date;
    State setting, hour, minute, day, month;
    State *timekeepingHist;
    int tsec, tmin, thour, dday, dmonth;
};

enum WatchEvents {
    Watch_MODE_EVT,
    Watch_SET_EVT,
    Watch_TICK_EVT
};

/* lookup table for the days of a month */
static int const day_of_month_lut[] = {
    0, /* unused month #0 */
    31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

void WatchShowTime(Watch *me) {
    printf("time: %2d:%02d:%02d",
    me->thour, me->tmin, me->tsec);
}

void WatchShowDate(Watch *me) {
    printf("date: %02d-%02d", me->dmonth, me->dday);
}

void WatchTick(Watch *me) {
    if (++me->tsec == 60) {
        me->tsec = 0;
        if (++me->tmin == 60) {
            me->tmin = 0;
            if (++me->thour == 24) {
                me->thour = 0;
                if (++me->dday > day_of_month_lut[me->dmonth]) {
                    me->dday = 1;
                    if (++me->dmonth == 13) {
                        me->dmonth = 1;
                    }
                }
            }
        }
    }
}

Msg const *Watch_top(Watch *me, Msg const *msg) {
    switch (msg->evt) {
    case START_EVT:
        STATE_START(me, &me->setting);
        return 0;
    }
    return msg;
}

Msg const *Watch_timekeeping(Watch *me, Msg const *msg) {
    switch (msg->evt) {
    case START_EVT:
        STATE_START(me, me->timekeepingHist);
        return 0;
    case EXIT_EVT:
        me->timekeepingHist = STATE_CURR(me);
        return 0;
    case Watch_SET_EVT:
        STATE_TRAN(me, &me->setting);
        printf("Watch::timekeeping-SET;");
        return 0;
    case Watch_TICK_EVT:
        WatchTick(me);
        printf("Watch::timekeeping-TICK;");
        return 0;
    }
    return msg;
}

Msg const *Watch_time(Watch *me, Msg const *msg) {
    switch (msg->evt) {
    case ENTRY_EVT:
        WatchShowTime(me);
        return 0;
    case Watch_MODE_EVT:
        STATE_TRAN(me, &me->date);
        printf("Watch::time-MODE;");
        return 0;
    case Watch_TICK_EVT:
        printf("Watch::time-TICK;");
        WatchTick(me);
        WatchShowTime(me);
        return 0;
    }
    return msg;
}

Msg const *Watch_date(Watch *me, Msg const *msg) {
    switch (msg->evt) {
    case ENTRY_EVT:
        WatchShowDate(me);
        return 0;
    case Watch_MODE_EVT:
        STATE_TRAN(me, &me->time);
        printf("Watch::date-MODE;");
        return 0;
    case Watch_TICK_EVT:
        printf("Watch::date-TICK;");
        WatchTick(me);
        WatchShowDate(me);
        return 0;
    }
    return msg;
}

Msg const *Watch_setting(Watch *me, Msg const *msg) {
    switch (msg->evt) {
    case START_EVT:
        STATE_START(me, &me->hour);
        printf("Watch::setting-START->hour;");
        return 0;
    }
    return msg;
}

Msg const *Watch_hour(Watch *me, Msg const *msg) {
    switch (msg->evt) {
    case Watch_MODE_EVT:
        if (++me->thour == 60) {
            me->thour = 0;
        }
        WatchShowTime(me);
        return 0;
    case Watch_SET_EVT:
        STATE_TRAN(me, &me->minute);
        printf("Watch::hour-SET;");
        return 0;
    }
    return msg;
}

Msg const *Watch_minute(Watch *me, Msg const *msg) {
    switch (msg->evt) {
    case Watch_MODE_EVT:
        if (++me->tmin == 60) {
            me->tmin = 0;
        }
        WatchShowTime(me);
        return 0;
    case Watch_SET_EVT:
        STATE_TRAN(me, &me->day);
        printf("Watch::minute-SET;");
        return 0;
    }
    return msg;
}

Msg const *Watch_day(Watch *me, Msg const *msg) {
    switch (msg->evt) {
    case Watch_MODE_EVT:
        if (++me->dday > day_of_month_lut[me->dmonth]) {
            me->dday = 1;
        }
        WatchShowDate(me);
        return 0;
    case Watch_SET_EVT:
        STATE_TRAN(me, &me->month);
        printf("Watch::day-SET;");
        return 0;
    }
    return msg;
}

Msg const *Watch_month(Watch *me, Msg const *msg) {
    switch (msg->evt) {
    case Watch_MODE_EVT:
        if (++me->dmonth > 12 ) {
            me->dmonth = 1;
        }
        WatchShowDate(me);
        return 0;
    case Watch_SET_EVT:
        STATE_TRAN(me, &me->timekeeping);
        printf("Watch::month-SET;");
        return 0;
    }
    return msg;
}

void WatchCtor(Watch *me) {
    HsmCtor((Hsm *)me, "Watch", (EvtHndlr)Watch_top);
    StateCtor(&me->timekeeping, "timekeeping",
              &((Hsm *)me)->top, (EvtHndlr)Watch_timekeeping);
    StateCtor(&me->time, "time", &me->timekeeping,
              (EvtHndlr)Watch_time);
    StateCtor(&me->date, "date", &me->timekeeping,
              (EvtHndlr)Watch_date);
    StateCtor(&me->setting, "setting", &((Hsm *)me)->top,
              (EvtHndlr)Watch_setting);
    StateCtor(&me->hour, "hour", &me->setting,
              (EvtHndlr)Watch_hour);
    StateCtor(&me->minute, "minute", &me->setting,
              (EvtHndlr)Watch_minute);
    StateCtor(&me->day, "day", &me->setting,
              (EvtHndlr)Watch_day);
    StateCtor(&me->month, "month", &me->setting,
              (EvtHndlr)Watch_month);

    me->timekeepingHist = &me->time;
    me->tsec = me->tmin = me->thour = 0;
    me->dday = me->dmonth = 1;
}

const Msg watchMsg[] = {
    { Watch_MODE_EVT },
    { Watch_SET_EVT  },
    { Watch_TICK_EVT }
};

int main() {
    Watch watch;

    printf("Enter:\n"
           "0 for MODE_EVT\n"
           "1 for SET_EVT\n"
           "2 for TICK_EVT\n"
           "3+ to exit\n\n");

    WatchCtor(&watch);
    HsmOnStart((Hsm *)&watch);
    for (;;)  {
        char c;
        printf("\nEvent<-");
        c = getc(stdin);
        getc(stdin);
        if (c < '0' || '2' < c) {
            break;
        }
        HsmOnEvent((Hsm *)&watch, &watchMsg[c - '0']);
    }
    return 0;
}
