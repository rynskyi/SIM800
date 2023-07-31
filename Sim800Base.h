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

#define SIM800_RES_NONE 0
#define SIM800_RES_OK 1
#define SIM800_RES_ERR 2

#define SIM800_LOG_L 1
#define SIM800_LOG_R 2

class Sim800Base {
private:
    Stream *serial;
    Stream *logger;
    char buffer[SIM800_BUFFER_MAX_SIZE];
    char prevMsg[SIM800_BUFFER_MAX_SIZE] = {};
    CircularBuffer<Sim800_Event, SIM800_EVENET_QUEUE_SIZE> events;

    void log(uint8_t mode, const char *value);
    char* trim(char *str);
    uint16_t dataAvailable();
    void sendMessage(const char *data);
    const char* readMessage();

public:
    Sim800Base(Stream *serial, Stream *log);

    bool getEvent(Sim800_Event *ev);
    void clearEvents();

    char* getResponse();

    uint8_t _sendCommand(const char *command, const char *okFlag, uint32_t timeout);
    uint8_t sendCommand(const char *command, const char *okFlag, uint32_t timeout = SIM800_AT_COMMAND_TIMEOUT);
    uint8_t sendCommand(const char *command, uint32_t timeout = SIM800_AT_COMMAND_TIMEOUT);
};

#endif
