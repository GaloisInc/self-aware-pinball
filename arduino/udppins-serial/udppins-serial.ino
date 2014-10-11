#include <Adafruit_CC3000.h>
#include <SPI.h>
#include "utility/debug.h"
#include "utility/socket.h"

#include <Wire.h>
#include <Adafruit_MCP23017.h>
#include <Adafruit_RGBLCDShield.h>

#include "UDPServer.h"

// These are the interrupt and control pins
#define ADAFRUIT_CC3000_IRQ   3  // MUST be an interrupt pin!
// These can be any two pins
#define ADAFRUIT_CC3000_VBAT  5
#define ADAFRUIT_CC3000_CS    10
// Use hardware SPI for the remaining pins
// On an UNO, SCK = 13, MISO = 12, and MOSI = 11

Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT,
                                         SPI_CLOCK_DIVIDER); // you can change this clock speed

#define WLAN_SSID       "David"
#define WLAN_PASS       "nyetwork"
// Security can be WLAN_SEC_UNSEC, WLAN_SEC_WEP, WLAN_SEC_WPA or WLAN_SEC_WPA2
#define WLAN_SECURITY   WLAN_SEC_WPA2

#define LISTEN_PORT           80      // What TCP port to listen on for connections.  
                                      // The HTTP protocol uses port 80 by default.

#define MAX_ACTION            10      // Maximum length of the HTTP action that can be parsed.

#define MAX_PATH              64      // Maximum length of the HTTP request path that can be parsed.
                                      // There isn't much memory available so keep this short!

#define BUFFER_SIZE           MAX_ACTION + MAX_PATH + 20  // Size of buffer for incoming request data.
                                                          // Since only the first line is parsed this
                                                          // needs to be as large as the maximum action
                                                          // and path plus a little for whitespace and
                                                          // HTTP version.

#define TIMEOUT_MS            500    // Amount of time in milliseconds to wait for
                                     // an incoming request to finish.  Don't set this
                                     // too high or your server could be slow to respond.

#define NUM_FLIPPERS          4      // the number of flippers in the machine
#define FIRST_FLIPPER_PIN     6      // the first flipper pin; it is assumed that the
                                     // remaining ones are consecutive

#define NUM_ROWS              8      // the number of rows in the switch matrix
#define NUM_COLS              8      // the number of columns in the switch matrix

#define LCD_DEBUG             0      // print debug information to LCD

#define DEST_IP 192,168,43,255 // broadcast
#define DEST_PORT 2811

uint8_t buffer[BUFFER_SIZE+1];
int bufindex = 0;
char action[MAX_ACTION+1];
char path[MAX_PATH+1];


#define UDP_READ_BUFFER_SIZE 20
#define LISTEN_PORT_UDP 2811
UDPServer udpServer(LISTEN_PORT_UDP);
Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();
Adafruit_CC3000_Client udpClient;

// These #defines make it easy to set the backlight color
#define RED 0x1
#define YELLOW 0x3
#define GREEN 0x2
#define TEAL 0x6
#define BLUE 0x4
#define VIOLET 0x5
#define WHITE 0x7

void setup(void) {
  for (int pin = 0; pin < NUM_FLIPPERS; pin++) {
    pinMode(FIRST_FLIPPER_PIN + pin, OUTPUT);
  }
  
  // set up the LCD's number of columns and rows: 
  lcd.begin(16, 2);
  lcd.setBacklight(RED);
  
  lcd.print("Ahoy!");
  
  Serial.begin(115200);
  Serial.println(F("Hello, CC3000!\n")); 

  Serial.print("Free RAM: "); Serial.println(getFreeRam(), DEC);
  
  // Initialise the module
  Serial.println(F("\nInitializing..."));
  if (!cc3000.begin()) {
    Serial.println(F("Couldn't begin()! Check your wiring?"));
    while(1);
  }
  
  Serial.print(F("\nAttempting to connect to ")); Serial.println(WLAN_SSID);
  lcd.setCursor(0,0);
  lcd.print("Connecting to");
  lcd.setCursor(0,1);
  lcd.print(WLAN_SSID);
  if (!cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY)) {
    Serial.println(F("Failed!"));
    while(1);
  }
   
  Serial.println(F("Connected!"));
  lcd.setCursor(0,0);
  lcd.print("Connected!");
  lcd.setCursor(0,1);
  lcd.print(WLAN_SSID);
  
  Serial.println(F("Request DHCP"));
  while (!cc3000.checkDHCP())
  {
    delay(100); // ToDo: Insert a DHCP timeout!
  }  

  // Display the IP address DNS, Gateway, etc.
  while (! displayConnectionDetails()) {
    delay(1000);
  }

  // ******************************************************
  // You can safely remove this to save some flash memory!
  // ******************************************************
  Serial.println(F("\r\nNOTE: This sketch may cause problems with other sketches"));
  Serial.println(F("since the .disconnect() function is never called, so the"));
  Serial.println(F("AP may refuse connection requests from the CC3000 until a"));
  Serial.println(F("timeout period passes.  This is normal behaviour since"));
  Serial.println(F("there isn't an obvious moment to disconnect with a server.\r\n"));
  
  if (!udpServer.begin()) {
    Serial.println(F("No UDP server!"));
  }
  
  udpClient = cc3000.connectUDP(cc3000.IP2U32(DEST_IP), DEST_PORT);
  if (!udpClient) {
    Serial.println(F("No UDP client!"));
    lcd.setBacklight(TEAL);
    delay(5000);
  }
}

void loop(void) {
    if (udpClient) {
      udpClient.write("hello!", 6);
    }
    uint8_t oldstate = 0; 
    if (udpServer.available()) {

      char buffer[UDP_READ_BUFFER_SIZE];
      int n = udpServer.readData(buffer, UDP_READ_BUFFER_SIZE);  // n contains # of bytes read into buffer

      if (n >= 2) {
	// Message 1: 0xff then byte with new state
	if ((byte) buffer[0] == 0xff) {
	  byte state = buffer[1];
          for (int i = 0; i < NUM_FLIPPERS; i++) {
            digitalWrite(FIRST_FLIPPER_PIN + i, state & (1 << i) ? HIGH : LOW);
          }
          if (oldstate != state) {
            lcd.setBacklight(WHITE);
            lcd.setBacklight(GREEN);
          }
          oldstate = state;
        }
	// Message 2: length then that many bytes with new lcd text
	else {
	  lcd.setCursor(0,0);
	  lcd.print("                ");
	  lcd.setCursor(0,0);
	  for (int i = 1; i <= min(min(buffer[0], n), 16); i++) {
	    lcd.print(buffer[i]);
	  }
	}
      }
   }
}

#define MSG_HEADER 0xff

// called after loop() if there's data in the serial buffer
void serialEvent() {
  if (Serial.available()) {
    // get the new byte:
    byte inB = Serial.read();
    
    if (inB == MSG_HEADER) {
      lcd.setBacklight(YELLOW);
      byte matrix[NUM_ROWS];
      int r = 0;
      while (Serial.available() && r < NUM_ROWS) {
        matrix[r++] = Serial.read();
      }
      if (r < NUM_ROWS) {
        // we got an incomplete switch matrix
        lcd.setBacklight(RED);
        return;
      }
      if (LCD_DEBUG) {
        lcd.setCursor(0,0);
        for (int i = 0; i < 1; i++) {
          for (int j=1; j <= 0xff; j = j<<1) {
            lcd.print(matrix[i] & j ? '1' : '0');
          }
          lcd.print("  ");
          lcd.print(matrix[i]);
        }
      }
      udpClient.write(matrix, NUM_ROWS);
      lcd.setBacklight(GREEN);
    }
  }
}

void printIP(uint32_t ip) {
  lcd.print(ip >> 24);
  lcd.print(F("."));
  lcd.print((ip >> 16) & 0xff);
  lcd.print(F("."));
  lcd.print((ip >> 8) & 0xff);
  lcd.print(F("."));
  lcd.print(ip & 0xff);
}

// Tries to read the IP address and other connection details
bool displayConnectionDetails(void)
{
  uint32_t ipAddress, netmask, gateway, dhcpserv, dnsserv;
  
  if (!cc3000.getIPAddress(&ipAddress, &netmask, &gateway, &dhcpserv, &dnsserv)) {
    Serial.println(F("Unable to retrieve the IP Address!\r\n"));
    return false;
  } else {
    Serial.print(F("\nIP Addr: ")); cc3000.printIPdotsRev(ipAddress);
    Serial.print(F("\nNetmask: ")); cc3000.printIPdotsRev(netmask);
    Serial.print(F("\nGateway: ")); cc3000.printIPdotsRev(gateway);
    Serial.print(F("\nDHCPsrv: ")); cc3000.printIPdotsRev(dhcpserv);
    Serial.print(F("\nDNSserv: ")); cc3000.printIPdotsRev(dnsserv);
    Serial.println();
    
    lcd.setBacklight(GREEN);
    lcd.setCursor(0,0);
    printIP(ipAddress);
    lcd.setCursor(0,1);
    lcd.print("p");
    lcd.print(LISTEN_PORT_UDP);
    lcd.print(" ");
    lcd.print(WLAN_SSID);
    return true;
  }
}
