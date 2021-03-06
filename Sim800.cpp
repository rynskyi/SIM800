#include <Arduino.h>
#include "Sim800.h"

uint8_t Sim800::init(uint16_t timeout) {
    // disable echo, set text mode for responses, set extended error mode
    unsigned long t = millis();
    while(millis() - t < timeout) {
        if (this->sendCommand("ATE0V1+CMEE=2", NULL, 0, NULL, 1500) == SIM800_RESPONSE_OK) {
            return this->sendCommand("ATE0+CLCC=1;+CLIP=0");
        }
    };
    return SIM800_RESPONSE_ERROR;
}

uint8_t Sim800::isReadyToCall() {
    // example: +CPAS: 0
    char res[32] = {};
    this->sendCommand("AT+CPAS", res, sizeof(res), "+CPAS:");
    return (strstr(res, "+CPAS: 0") != NULL) ? SIM800_RESPONSE_OK : SIM800_RESPONSE_ERROR;
}

uint8_t Sim800::waitForReadyToCall(uint32_t timeout) {
    unsigned long t = millis();
    while(millis() - t < timeout) {
        if (this->isReadyToCall() == SIM800_RESPONSE_OK) {
            return SIM800_RESPONSE_OK;
        }
        delay(2500); // 2.5 sec.
    }
    return SIM800_RESPONSE_ERROR;
}

uint8_t Sim800::makeCall(const char *phone) {
    char cmd[32] = {};
    snprintf(cmd, sizeof(cmd), "ATD%s;", phone);
    return this->sendCommand(cmd);
}

uint8_t Sim800::answerCall() {
    return this->sendCommand("ATA");
}

uint8_t Sim800::hangUpCall() {
    return this->sendCommand("ATH0");
}

uint8_t Sim800::turnOffMic() {
    return this->sendCommand("AT+CEXTERNTONE=1");
}

uint8_t Sim800::turnOnDTMF() {
    return this->sendCommand("AT+DDET=1");
}

uint8_t Sim800::playAudio(const char *fname, uint8_t volume, bool loop) {
    char cmd[64] = {};
    snprintf(cmd, sizeof(cmd), "AT+CREC=4,\"%s\",0,%d,%d", fname, volume, (loop?1:0));
    return this->sendCommand(cmd);
}

uint8_t Sim800::stopAudio() {
    return this->sendCommand("AT+CREC=5");
}

uint8_t Sim800::sendUSSD(const char *ussd) {
    char cmd[32] = {};
    snprintf(cmd, sizeof(cmd), "AT+CUSD=1,\"%s\"", ussd);
    return this->sendCommand(cmd);
}

uint8_t Sim800::resetSettings() {
    return this->sendCommand("ATZ0");
}

uint32_t Sim800::getVoltage() {
    // example: +CBC: 0,100,4498
    char res[32] = {};
    this->sendCommand("AT+CBC", res, sizeof(res), "+CBC:");
    const char *p = strstr(res + strlen("+CBC: 0,"), ",") + 1;
    return strtoul(p, NULL, 10);
}

uint32_t Sim800::getSignal() {
    // return value converted to dBm
    // example: +CSQ: 10,0
    char res[32] = {};
    this->sendCommand("AT+CSQ", res, sizeof(res), "+CSQ:");
    uint32_t k = strtoul(res + strlen("+CSQ: "), NULL, 10);
    return 113 - (k * 2);
}

uint32_t Sim800::freeSpace() {
    char res[32] = {};
    this->sendCommand("AT+CREC=8", res, sizeof(res), "+CREC: 8,");
    uint32_t k = strtoul(res + strlen("+CREC: 8,"), NULL, 10);
    return k;
}

uint8_t Sim800::lsFiles(char *res, uint16_t resSize) {
    // res not working (because get multiline response string)
    return this->sendCommand("AT+FSLS=C:\\User\\");
}

uint8_t Sim800::rmFile(char *fName) {
    char cmd[32] = {};
    snprintf(cmd, sizeof(cmd), "AT+FSDEL=C:\\User\\%s", fName);
    return this->sendCommand(cmd);
}