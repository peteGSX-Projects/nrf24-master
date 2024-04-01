#ifndef SERIAL_FUNCTIONS_H
#define SERIAL_FUNCTIONS_H

#include <Arduino.h>
#include "RF24Mesh.h"

void processSerialInput(RF24Mesh *mesh);
void displayHelp();
void displayNetworkNodes(RF24Mesh *mesh);
void setNodePin(RF24Mesh *mesh, uint8_t nodeId, uint8_t pin, bool state);

#endif