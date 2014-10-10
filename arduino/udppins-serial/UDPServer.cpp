#include "UDPServer.h"

<<<<<<< HEAD
// hardcoded destination address
#define DEST_IP 0x782BA8C0  // 192.168.43.120

// the destination address and port for our outgoing packets
sockaddr destAddress;
socklen_t destAddressLen;

=======
>>>>>>> d76048ccfef0a008abba96ebc5812f26186a28b9
UDPServer::UDPServer(uint16_t port) {
   _port = port;
   _socket = -1;
   
   memset(&destAddress, 0x00, sizeof(destAddress));
   destAddress.sa_family = AF_INET;
   destAddress.sa_data[0] = (port & 0xFF00) >> 8;  // Set the Port Number
   destAddress.sa_data[1] = (port & 0x00FF);
   destAddress.sa_data[2] = DEST_IP >> 24;
   destAddress.sa_data[3] = DEST_IP >> 16;
   destAddress.sa_data[4] = DEST_IP >> 8;
   destAddress.sa_data[5] = DEST_IP;
   destAddressLen = sizeof(destAddress);
}

bool UDPServer::begin() {
   //Serial.print("START of udpServer::Begin() _socket: "); Serial.println(_socket);
   // Open the socket if it isn't already open.
   if (_socket == -1) {
      // Create the UDP socket
      int soc = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
      if (soc < 0) {
	 Serial.println("socket() call failed");
	 return false;
      }

    
      sockaddr_in address;
      memset(&address, 0, sizeof(address));
      address.sin_family = AF_INET;
      address.sin_port = htons(_port);
      address.sin_addr.s_addr = 0;  // 0 => auto use own ip address
      socklen_t len = sizeof(address);
      if (bind(soc, (sockaddr*) &address, sizeof(address)) < 0) {
	 Serial.println("bind() call failed");
	 return false;
      }

      _socket = soc;
   }

   Serial.print("END of udpServer::begin() _socket: "); Serial.println(_socket);

   return true;
}

bool UDPServer::available() {
   timeval timeout;
   timeout.tv_sec = 0;
   timeout.tv_usec = 5000;
   fd_set reads;
   FD_ZERO(&reads);
   FD_SET(_socket, &reads);
   select(_socket + 1, &reads, NULL, NULL, &timeout);
   if (!FD_ISSET(_socket, &reads)) {
      // No data to read.
      //Serial.println("No data to read.");
      return false;
   }

   return true;
}

int UDPServer::readData(char *buffer, int bufferSize) {
   // If there is data, then stores it into buffer &
   // returns the length of buffer. (-1 if none)

   if (available()) {  // Make sure data is really available

      int n = recv(_socket, buffer, bufferSize, 0);

      if (n < 1) {
	 // Error getting data.
	 Serial.println("Error getting data");
	 return -1;
      }

      return n;
   }

   return -1;
}

int UDPServer::sendData(char *buf, int len)
{
    return sendto(_socket, buf, len, 0, (sockaddr*)&destAddress, destAddressLen);
}

