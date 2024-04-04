#ifndef SERIAL_FUNCTIONS_H
#define SERIAL_FUNCTIONS_H

#include <Arduino.h>
#include "RF24Mesh.h"

void processSerialInput(RF24Mesh *mesh);
void displayHelp(); // <H>
void displayNetworkNodes(RF24Mesh *mesh); // <N>

#endif