#ifndef SIM800BASE_H
#define SIM800BASE_H

#include <stdint.h>
#include <Stream.h>
#include <CircularBuffer.h>
#include "sim800Event.h"

#define SIM800_LOG_MESSAGE true // for dev only 
#define SIM800_AT_COMMAND_TIMEOUT 5000  // ms
#define SIM800_SERIAL_RX_TIMEOUT 100  // ms
#define SIM800_BUFFER_MAX_SIZE 128 // serial RX buffer
#define SIM800_EVENET_QUEUE_SIZE 3

#define SIM800_RESPONSE_NONE 0
#define SIM800_RESPONSE_OK 1
#define SIM800_RESPONSE_ERROR 2

#define SIM800_LOG_L 1
#define SIM800_LOG_R 2

class Sim800Base {
private:
    Stream *serial;
    Stream *logger;
    char buffer[SIM800_BUFFER_MAX_SIZE];
    void log(uint8_t mode, const char *value);
    char* trim(char *s);  
    CircularBuffer<Sim800_Event, SIM800_EVENET_QUEUE_SIZE> events;
public:
    Sim800Base(Stream *serial, Stream *log);
    void sendData(const char *data);
    uint16_t dataAvailable();
    bool getEvent(Sim800_Event *ev);
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
