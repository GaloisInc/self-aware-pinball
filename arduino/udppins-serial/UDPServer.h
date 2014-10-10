#ifndef UDPSERVER_H
#define UDPSERVER_H

#include <Arduino.h>

#include "Adafruit_CC3000.h"
#include "utility/socket.h"

class UDPServer {

private:
   uint16_t _port;
   int _socket;

public:
   UDPServer(uint16_t port);

   bool begin();
   bool available();
   int readData(char *buffer, int bufferSize);
   int sendData(char *buffer, int bufferSize);
};

#endif  // UDPSERVER_H

