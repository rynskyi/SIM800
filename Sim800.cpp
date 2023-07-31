#include <Arduino.h>
#include "Sim800.h"

uint8_t Sim800::init(uint16_t timeout) {
    unsigned long t = millis();
    while (millis() - t < timeout) {
        if (sendCommand("ATE0V1+CMEE=2", 1500) == SIM800_RES_OK) {
            return sendCommand("ATE0+CLCC=1;+CLIP=0");
        }
    }
    return SIM800_RES_ERR;
}

uint8_t Sim800::isReadyToCall() {
    // char buf[32] = {};
    sendCommand("AT+CPAS");
    char *res = getResponse();
    return (strstr(res, "+CPAS: 0") != NULL) ? SIM800_RES_OK : SIM800_RES_ERR;
}

uint8_t Sim800::waitForReadyToCall(uint32_t timeout) {
    unsigned long t = millis();
    while (millis() - t < timeout) {
        if (isReadyToCall() == SIM800_RES_OK) {
            return SIM800_RES_OK;
        }
        delay(2500); // 2.5 sec.
    }
    return SIM800_RES_ERR;
}

uint8_t Sim800::makeCall(const char *phone) {
    char buf[32] = {};
    snprintf(buf, sizeof(buf), "ATD%s;", phone);
    return sendCommand(buf);
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
    char buf[64] = {};
    snprintf(buf, sizeof(buf), "AT+CREC=4,\"%s\",0,%d,%d", fname, volume, (loop?1:0));
    return this->sendCommand(buf);
}

uint8_t Sim800::stopAudio() {
    return this->sendCommand("AT+CREC=5");
}

uint8_t Sim800::sendUSSD(const char *ussd) {
    char buf[32] = {};
    snprintf(buf, sizeof(buf), "AT+CUSD=1,\"%s\"", ussd);
    return this->sendCommand(buf);
}

uint8_t Sim800::resetSettings() {
    return this->sendCommand("ATZ0");
}

uint8_t Sim800::getVoltage(uint32_t *voltage) {
    // example: +CBC: 0,100,4498
    uint8_t status = this->sendCommand("AT+CBC");
    char *res = getResponse();
    const char *p = strstr(res + strlen("+CBC: 0,"), ",") + 1;
    *voltage  = strtoul(p, NULL, 10);
    return status;
}

uint8_t Sim800::getSignal(uint32_t *strength) {
    // return value converted to dBm
    // example: +CSQ: 10,0
    uint8_t status = this->sendCommand("AT+CSQ");
    char *res = getResponse();
    uint32_t v = strtoul(res + strlen("+CSQ: "), NULL, 10);
    *strength = 113 - (v * 2);
    return status;
}

uint8_t Sim800::freeSpace(uint32_t *bytes) {
    uint8_t status = this->sendCommand("AT+CREC=8");
    char *res = getResponse();
    *bytes = strtoul(res + strlen("+CREC: 8,"), NULL, 10);
    return status;
}

uint8_t Sim800::lsFiles(char *res, uint16_t resSize) {
    // TODO: res not working (because get multiline response string)
    return this->sendCommand("AT+FSLS=C:\\User\\");
}

uint8_t Sim800::rmFile(char *fName) {
    char buf[32] = {};
    snprintf(buf, sizeof(buf), "AT+FSDEL=C:\\User\\%s", fName);
    return this->sendCommand(buf);
}

uint8_t Sim800::deleteAllSms() {
    uint8_t status = sendCommand("AT+CMGF=1");
    if (status != SIM800_RES_OK) return status;
    status = sendCommand("AT+CMGDA=\"DEL ALL\"");
    return status;
}

uint8_t Sim800::sendSms(const char *phone, const char *text) {
    uint8_t status;
    char buf[128] = {};

    status = sendCommand("AT+CMGF=1");
    if (status != SIM800_RES_OK) return status;

    snprintf(buf, sizeof(buf), "AT+CMGS=\"%s\"", phone);
    status = sendCommand(buf, ">");
    if (status != SIM800_RES_OK) return status;

    snprintf(buf, sizeof(buf), "%s\032", text); // Ctrl+Z = 26 = '\032' = '\x1A'
    return sendCommand(buf, 10000);
}