#include "sim800Event.h"

#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))


bool parseEvent(Sim800_Event *ev, const char *msg) {
        if (msg == NULL) { 
        return false; 
    }
    // init event 
    memset(ev, 0, sizeof(Sim800_Event));
    ev->type = SIM800_EVENT_UNKNOWN;
    // SMS handler
    if (strncmp(msg, "+CMTI: ",  strlen("+CMTI: ")) == 0) {
        // example msg: +CMTI: "ME",30  or +CMTI: "SM",30
        ev->type = SIM800_EVENT_SMS_INCOME;
        const char *c= strstr(msg, "\",") + strlen("\",");
        if (c != NULL) {
            strncpy(ev->data, c, sizeof(ev->data) - 1);
        }
    }
    // CLCC Call handlers
    else if (strncmp(msg, "+CLCC: ", strlen("+CLCC: ")) == 0) {
        // example msg: +CLCC: 1,0,6,0,0,"+380681231212",145,""
        // parse call params
        // char direction = msg[strlen("+CLCC: ") + 2];
        char state = msg[strlen("+CLCC: ") + 4];
        // parse phone num
        const char *phoneStart = msg + strlen("+CLCC: 1,0,6,0,0,\"");
        const char *phoneEnd = strstr(msg, "\",");
        if (phoneEnd != NULL) {
            strncpy(ev->data, phoneStart, phoneEnd - phoneStart);
        }
        if (state == '0') {
            ev->type = SIM800_EVENT_CALL_ACTIVE;
        }
        if (state == '3') {
            ev->type = SIM800_EVENT_CALL_DIALING;
        }
        if (state == '4') {
            ev->type = SIM800_EVENT_CALL_RING;
        }
        if (state == '6') {
            ev->type = SIM800_EVENT_CALL_DISCONNECT;
        }
    }
    // Call busy handler
    else if (strncmp(msg, "BUSY",  strlen("BUSY")) == 0) {
        ev->type = SIM800_EVENT_CALL_BUSY;
    }
    // NO CARRIER
    else if (strncmp(msg, "NO CARRIER",  strlen("NO CARRIER")) == 0) {
        ev->type = SIM800_EVENT_NO_CARRIER;
    }
    // DTMF
    else if (strncmp(msg, "+DTMF: ", strlen("+DTMF: ")) == 0) {
        ev->type = SIM800_EVENT_DTMF;
        strncpy(ev->data, msg + strlen("+DTMF: "), sizeof(ev->data) - 1);
    }
    // USSD handler
    else if (strncmp(msg, "+CUSD: ", strlen("+CUSD: ")) == 0) {
        // example msg: +CUSD: 0, "Na Vashem schete 2.00 grn. Tarif 'Vodafone SuperNet Pro'. Nomer deystvitelen do 04.06.2021. Zashchitite svoinomer ot krazhi - *181*7# (0 grn)", 15
        ev->type = SIM800_EVENT_USSD_INCOME;
        const char *bgn = strstr(msg, ", \"") + 3;
        const char *end = strstr(msg, "\", ");
        if (end == NULL) {
            end = &msg[strlen(msg)-1];
        }
        if (bgn != NULL && end != NULL && end > bgn) {
            strncpy(ev->data, bgn, MIN(end - bgn, (int16_t)sizeof(ev->data) - 1));
        }
    }

    return ev->type == SIM800_EVENT_UNKNOWN ? false : true;
}