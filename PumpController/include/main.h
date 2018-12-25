#ifndef _main_h
#define _main_h

#define EV_REQUEST_CONVERSION   0x0001
#define EV_START_CONVERSION     0x0002
#define EV_CONVERSION_COMPLETE  0x0004
#define EV_PID                  0x0008
#define EV_PID_UPDATE           0x0010

#define EV_TIMER_SEC            0x8000

extern word Events;

#endif //_main_h
