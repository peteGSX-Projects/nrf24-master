#ifndef NETWORKFUNCTIONS_H
#define NETWORKFUNCTIONS_H

#include <Arduino.h>
#include "RF24.h"
#include "RF24Network.h"
#include "RF24Mesh.h"
#include <SPI.h>
#include "config.h"

/**
 * @brief Type of the packet. 0 - 127 are user-defined types, 128 - 255 are reserved for system.

User message types 1 through 64 will NOT be acknowledged by the network, while message types 65 through 127 will receive a network ACK.
System message types 192 through 255 will NOT be acknowledged by the network. Message types 128 through 192 will receive a network ACK.
 
 */
enum PacketType : unsigned char {
  NodeCheck = 1,
  ReadAnalogueVPin = 65,
  ReadDigitalVPin = 66,
  SetDigitalPin = 67,
  ReadDigitalPin = 68,
  SetServoPWM = 69,
  SetDigitalVpin = 70,
  InitialiseEXIO = 71,
};

enum EXIOExpander: unsigned char {
  EXIOINIT = 0xE0,    // Flag to initialise setup procedure
  EXIORDY = 0xE1,     // Flag we have completed setup procedure, also for EX-IO to ACK setup
  EXIODPUP = 0xE2,    // Flag we're sending digital pin pullup configuration
  EXIOVER = 0xE3,     // Flag to get version
  EXIORDAN = 0xE4,    // Flag to read an analogue input
  EXIOWRD = 0xE5,     // Flag for digital write
  EXIORDD = 0xE6,     // Flag to read digital input
  EXIOENAN = 0xE7,    // Flag to enable an analogue pin
  EXIOINITA = 0xE8,   // Flag we're receiving analogue pin mappings
  EXIOPINS = 0xE9,    // Flag we're receiving pin counts for buffers
  EXIOWRAN = 0xEA,   // Flag we're sending an analogue write (PWM)
  EXIOERR = 0xEF,     // Flag we've received an error
};

extern RF24 radio;
extern RF24Network network;
extern RF24Mesh mesh;

void setupNetwork();
void processMesh();
void processNetwork();
uint16_t readAnaloguePin(RF24Mesh *mesh, uint8_t nodeId, uint8_t muxAddress, uint8_t muxChannel, uint8_t deviceAddress, uint8_t pin);  // <A ...>
bool readDigitalPin(RF24Mesh *mesh, uint8_t nodeId, uint8_t muxAddress, uint8_t muxChannel, uint8_t deviceAddress, uint8_t pin);  // <D ...>
bool initialiseEXIOExpander(RF24Mesh *mesh, uint8_t nodeId, uint8_t muxAddress, uint8_t muxChannel, uint8_t deviceAddress); // <I ...>
void processEXIOInit(RF24NetworkHeader &header, RF24Network &network, RF24Mesh &mesh);
bool setNetworkNodePin(RF24Mesh *mesh, uint8_t nodeId, uint8_t pin, bool state); // <P ...>
bool readNetworkNodePin(RF24Mesh *mesh, uint8_t nodeId, uint8_t pin);  // <Q ...>
void processNetworkNodePin(RF24NetworkHeader &header, RF24Network &network, RF24Mesh &mesh);
void setPWMPin(RF24Mesh *mesh, uint8_t nodeId, uint8_t muxAddress, uint8_t muxChannel, uint8_t deviceAddress, uint8_t pin, uint16_t pwmValue);  // <S ...>
void setDigitalPin(RF24Mesh *mesh, uint8_t nodeId, uint8_t muxAddress, uint8_t muxChannel, uint8_t deviceAddress, uint8_t pin, bool state);  // <V ...>

#endif
