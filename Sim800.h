#ifndef SIM800_H
#define SIM800_H

#include <SoftwareSerial.h>
#include <HardWareserial.h>
#include "Sim800Base.h"

class Sim800 : public Sim800Base {
public:
    Sim800(HardwareSerial *serial, SoftwareSerial *logger) : Sim800Base(serial, logger) {};
    bool init();
    bool redyForCall();
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
    // TODO: add SMS functions
};

#endif
