#ifndef SERIAL_FUNCTIONS_H
#define SERIAL_FUNCTIONS_H

#include <Arduino.h>
#include "RF24Mesh.h"

void processSerialInput(RF24Mesh *mesh);
uint16_t readAnaloguePin(RF24Mesh *mesh, uint8_t nodeId, uint8_t muxAddress, uint8_t muxChannel, uint8_t pin);  // <A ...>
void displayHelp(); // <H>
void displayNetworkNodes(RF24Mesh *mesh); // <N>
void setNodePin(RF24Mesh *mesh, uint8_t nodeId, uint8_t pin, bool state); // <P ...>

#endif