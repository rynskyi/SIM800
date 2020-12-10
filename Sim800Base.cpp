#include <Arduino.h>
#include "Sim800Base.h"
// #include <MemoryFree.h>

Sim800Base::Sim800Base(Stream *_serial, Stream *_logger) {
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
    Sim800_Event _ev;
    uint8_t result = false;
    
    // get from queue if not empty
    if (!this->events.isEmpty()) {
        *ev = this->events.shift();
        result = true;
    }
    // parse and push event to queue
    if (parseEvent(&_ev, this->readMessage())) {
        this->events.push(_ev);
    }
    return result;
}

void Sim800Base::clearEvents() {
    this->events.clear();
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

    this->log(SIM800_LOG_L, cmd);
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
                if (parseEvent(&ev, msg)) {
                    this->events.push(ev);
                }
            }
        }          
    };
    // timeout handler
    if (status == SIM800_RESPONSE_NONE) {
        this->log(SIM800_LOG_R, "TIMEOUT");
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
    this->log(SIM800_LOG_R, msg);
    // this->logger->print("Free RAM:");
    // this->logger->println(freeMemory());
    return msg;
}

void Sim800Base::log(uint8_t mode, const char *value) {
    if (SIM800_LOG_MESSAGE) {
        this->logger->print(F("    sim ")); 
        this->logger->print(mode == SIM800_LOG_L ? F("< ") : F("> "));
        this->logger->println(value); 
    } 
}

char* Sim800Base::trim(char *s) {
    // trim "/r/n" 
    if (s[0] == '\r' && s[1] == '\n') { s+=2; }
    uint16_t len = strlen(s);
    if (s[len - 2] == '\r' && s[len - 1] == '\n') s[len -2] = '\0';
    return s;
}     