#ifndef SIM800EVENT_H
#define SIM800EVENT_H

#include <stdint.h>
#include <string.h>

#define SIM800_EVENT_UNKNOWN 0
#define SIM800_EVENT_CALL_RING 11
#define SIM800_EVENT_CALL_DIALING 12
#define SIM800_EVENT_CALL_ACTIVE 13
#define SIM800_EVENT_CALL_BUSY 14
#define SIM800_EVENT_CALL_DISCONNECT 18
#define SIM800_EVENT_NO_CARRIER 19
#define SIM800_EVENT_SMS_INCOME 21
#define SIM800_EVENT_USSD_INCOME 31
#define SIM800_EVENT_DTMF 41

struct Sim800_Event {
    uint8_t type;
    char data[32] = {0};
};

bool parseEvent(Sim800_Event *ev, const char *msg);

#endif