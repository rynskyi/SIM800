#ifndef SIM800_H
#define SIM800_H

#include "Sim800Base.h"

class Sim800 : public Sim800Base {
public:
    Sim800(Stream *serial, Stream *logger) : Sim800Base(serial, logger) {};
    uint8_t init(uint16_t timeout);
    uint8_t isReadyToCall();
    uint8_t waitForReadyToCall(uint32_t timeout);
    uint8_t makeCall(const char *phone);
    uint8_t answerCall();
    uint8_t hangUpCall();
    uint8_t turnOffMic();
    uint8_t turnOnDTMF();
    uint8_t playAudio(const char *fname, uint8_t volume, bool loop);
    uint8_t stopAudio();
    uint8_t sendUSSD(const char *ussd);
    uint8_t resetSettings();
    uint32_t getVoltage();
    uint32_t getSignal();
    uint32_t freeSpace();
    uint8_t lsFiles(char *res, uint16_t resSize);
    uint8_t rmFile(char *fName);
    // TODO: add SMS functionality
};

#endif
