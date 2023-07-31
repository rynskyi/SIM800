#include <Arduino.h>
#include "Sim800Base.h"

Sim800Base::Sim800Base(Stream *_serial, Stream *_logger) : serial(_serial), logger(_logger) {}

uint8_t Sim800Base::sendCommand(const char *command, const char *okFlag, uint32_t timeout) {
    return _sendCommand(command, okFlag, timeout);
}

uint8_t Sim800Base::sendCommand(const char *command, uint32_t timeout) {
    return _sendCommand(command, "OK", timeout);
}

uint8_t Sim800Base::_sendCommand(const char *command, const char *okFlag, uint32_t timeout) {
    uint8_t status = SIM800_RES_NONE;
    const char *msg = NULL;

    uint32_t t = millis();
    log(SIM800_LOG_L, command);
    sendMessage(command);
    memset(prevMsg, 0, sizeof(prevMsg));

    while (status == SIM800_RES_NONE && (millis() - t < timeout)) {
        msg = readMessage();
        if (msg && *msg) {
            log(SIM800_LOG_R, msg);
            // success
            if (strcmp(msg, okFlag) == 0) {
                status = SIM800_RES_OK;
            // error
            } else if (strstr(msg, "+CME ERROR:") == msg || strstr(msg, "+CMS ERROR:") == msg) {
                status = SIM800_RES_ERR;
            // else
            } else {
                strncpy(prevMsg, msg, sizeof(prevMsg) - 1);
                Sim800_Event ev;
                if (parseEvent(&ev, msg)) {
                    events.push(ev);
                }
            }
        }
    }

    if (status == SIM800_RES_NONE) {
        log(SIM800_LOG_R, "TIMEOUT");
        status = SIM800_RES_ERR;
    }

    return status;
}

char* Sim800Base::getResponse() {
    return prevMsg;
}

const char* Sim800Base::readMessage() {
    if (!serial->available()) return NULL;

    uint8_t i = 0;
    uint32_t t = millis();
    memset(buffer, 0, sizeof(buffer));

    while (millis() - t < SIM800_SERIAL_RX_TIMEOUT && i < sizeof(buffer) - 1) {
        if (!serial->available()) continue;
        t = millis();
        char c = serial->read();
        buffer[i++] = c;
        if (c == '\n' || c == '\r') break;
    }
    return trim(buffer);
}

void Sim800Base::sendMessage(const char *data) {
    serial->println(data);
}

uint16_t Sim800Base::dataAvailable() {
    return serial->available();
}

char* Sim800Base::trim(char *str) {
    if (!str || !*str) return str;

    char *start = str;
    char *end = str + strlen(str) - 1;
    while (*start && (*start == ' ' || *start == '\r' || *start == '\n')) start++;
    while (end > start && (*end == ' ' || *end == '\r' || *end == '\n')) end--;

    *(end + 1) = '\0';

    if (start != str) {
        while (*start) *str++ = *start++;
        *str = '\0';
    }
    return str;
}

void Sim800Base::log(uint8_t mode, const char *value) {
    if (SIM800_LOG_MESSAGE) {
        logger->print(F("    sim "));
        logger->print(mode == SIM800_LOG_L ? F("< ") : F("> "));
        logger->print("\"");
        logger->print(value);
        logger->print("\"\n");
    }
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
    events.clear();
}
