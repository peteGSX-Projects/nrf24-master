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
  ReadAnalogueVPin = 65,
  ReadDigitalVPin = 66,
  SetDigitalPin = 67,
  ReadDigitalPin = 68,
  SetServoPWM = 69,
  SetDigitalVpin = 70,
};

extern RF24 radio;
extern RF24Network network;
extern RF24Mesh mesh;

void setupNetwork();
void processMesh();
void processNetwork();
uint16_t readAnaloguePin(RF24Mesh *mesh, uint8_t nodeId, uint8_t muxAddress, uint8_t muxChannel, uint8_t deviceAddress, uint8_t pin);  // <A ...>
bool readDigitalPin(RF24Mesh *mesh, uint8_t nodeId, uint8_t muxAddress, uint8_t muxChannel, uint8_t deviceAddress, uint8_t pin);  // <D ...>
void setNetworkNodePin(RF24Mesh *mesh, uint8_t nodeId, uint8_t pin, bool state); // <P ...>
bool readNetworkNodePin(RF24Mesh *mesh, uint8_t nodeId, uint8_t pin);  // <Q ...>
void setPWMPin(RF24Mesh *mesh, uint8_t nodeId, uint8_t muxAddress, uint8_t muxChannel, uint8_t deviceAddress, uint8_t pin, uint16_t pwmValue);  // <S ...>
void setDigitalPin(RF24Mesh *mesh, uint8_t nodeId, uint8_t muxAddress, uint8_t muxChannel, uint8_t deviceAddress, uint8_t pin, bool state);  // <V ...>

#endif
