#include <Arduino.h> // for Serial.print();
#include "Sim800Base.h"

#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))

Sim800Base::Sim800Base(HardwareSerial *_serial, SoftwareSerial *_logger) {
    // init serial
    this->serial = _serial;
    this->logger = _logger;
}

void Sim800Base::sendData(const char *data) {
    this->serial->println(data);
}

uint16_t Sim800Base::dataAvailable() {
    return this->serial->available();
}

bool Sim800Base::getEvent(Sim800_Event *ev) {
    return this->parseEvent(ev, this->readMessage());
}

bool Sim800Base::parseEvent(Sim800_Event *ev, const char *msg) {
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

uint8_t Sim800Base::sendCommand(
        const char *cmd,
        char *resData,
        uint16_t resDataSize,
        const char *prefix,
        uint32_t timeout
) {
    uint8_t status = SIM800_RESPONSE_NONE;
    const char *msg, *_r = NULL;
    char prevMsg[SIM800_BUFFER_MAX_SIZE] = {};
    uint32_t _t = millis() + timeout;

    this->log("sim800<-", cmd);
    this->sendData(cmd);

    while (status == SIM800_RESPONSE_NONE && millis() < _t) {
        msg = this->readMessage();
        if (msg != NULL) {
            // success 
            if (strcmp(msg, "OK") == 0) {
                status = SIM800_RESPONSE_OK;
                if (prefix != NULL && strstr(prevMsg, prefix) == prevMsg) {
                    _r = prevMsg;
                }
            }
            // error
            else if (strstr(msg, "+CME ERROR:") == msg || strstr(msg, "+CMS ERROR:") == msg) {
                status = SIM800_RESPONSE_ERROR;
                _r = msg + 12; // because len("+CME ERROR:") == 12
            } 
            else {
                strncpy(prevMsg, msg, sizeof(prevMsg)); // store last message 
                // handle event
                Sim800_Event ev;
                if (this->parseEvent(&ev, msg)) {
                    // TODO: add to event queue
                }
            }
        }          
    };
    // timeout handler
    if (status == SIM800_RESPONSE_NONE) {
        this->log("sim800->", "TIMEOUT");
        status = SIM800_RESPONSE_ERROR;
        _r = "TIMEOUT";
    }
    // set response data
    if (resData != NULL && resDataSize > 0) {
        memset(resData, 0, resDataSize);
        strncpy(resData, _r, resDataSize - 1);
    }
    return status;
}

const char* Sim800Base::readMessage() {
    if (!this->serial->available()) { return NULL; }
    memset(this->buffer, 0, sizeof(this->buffer));
    uint8_t i = 0;
    uint32_t _t = millis() + SIM800_SERIAL_RX_TIMEOUT;
    while (millis() < _t && i < sizeof(this->buffer) - 1) {
        if (this->serial->available()) {
            this->buffer[i++] = this->serial->read();
            _t = millis() + SIM800_SERIAL_RX_TIMEOUT;
            // cut here if \r\n\r\n found
            if (i > 3 && 
                this->buffer[i-1] == '\n' && this->buffer[i-2] == '\r' &&
                this->buffer[i-3] == '\n' && this->buffer[i-4] == '\r'
            ) {
                this->buffer[i-4] = '\0';
                break;
            }
        }
    }
    char *msg = this->trim(this->buffer);
    this->log("sim800->", msg);
    return msg;
}

void Sim800Base::log(const char *mode, const char *value) {
    if (SIM800_LOG_MESSAGE) {
        this->logger->print("    "); 
        this->logger->print(mode);
        this->logger->print(": ");
        this->logger->print(value); 
        this->logger->print("\n");
    }         
}

char* Sim800Base::trim(char *s) {
    // trim "/r/n" 
    if (s[0] == '\r' && s[1] == '\n') { s+=2; }
    uint16_t len = strlen(s);
    if (s[len - 2] == '\r' && s[len - 1] == '\n') s[len -2] = '\0';
    return s;
}     