#ifndef SIM800BASE_H
#define SIM800BASE_H

#include <stdint.h>
#include <SoftwareSerial.h>
#include <HardWareserial.h>

#define SIM800_LOG_MESSAGE true // for dev only 
#define SIM800_AT_COMMAND_TIMEOUT 5000  // ms
#define SIM800_SERIAL_RX_TIMEOUT 100  // ms
#define SIM800_BUFFER_MAX_SIZE 128 // serial RX buffer

#define SIM800_RESPONSE_NONE 0
#define SIM800_RESPONSE_OK 1
#define SIM800_RESPONSE_ERROR 2

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

class Sim800Base {
private:
    HardwareSerial *serial;
    SoftwareSerial *logger;
    char buffer[SIM800_BUFFER_MAX_SIZE];
    void log(const char *mode, const char *value);
    char* trim(char *s);  
public:
    Sim800Base(HardwareSerial *serial, SoftwareSerial *log);
    void sendData(const char *data);
    uint16_t dataAvailable();
    bool getEvent(Sim800_Event *ev);
    bool parseEvent(Sim800_Event *ev, const char *msg = NULL);
    uint8_t sendCommand(
        const char *cmd,
        char *resData = NULL,
        uint16_t resDataSize = 0,
        const char *prefix = NULL,
        uint32_t timeout = SIM800_AT_COMMAND_TIMEOUT
    );
    const char* readMessage();   
};

#endif
