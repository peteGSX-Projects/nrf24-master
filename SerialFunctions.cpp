#include <Arduino.h>
#include "SerialFunctions.h"

bool newSerialData = false;   // Flag for new serial data being received
const byte numSerialChars = 20;   // Max number of chars for serial input
char serialInputChars[numSerialChars];  // Char array for serial input

/*
* Function to read and process serial input for I2C address config
*/
void processSerialInput(RF24Mesh *mesh) {
  static bool serialInProgress = false;
  static byte serialIndex = 0;
  char startMarker = '<';
  char endMarker = '>';
  char serialChar;
  while (Serial.available() > 0 && newSerialData == false) {
    serialChar = Serial.read();
    if (serialInProgress == true) {
      if (serialChar != endMarker) {
        serialInputChars[serialIndex] = serialChar;
        serialIndex++;
        if (serialIndex >= numSerialChars) {
          serialIndex = numSerialChars - 1;
        }
      } else {
        serialInputChars[serialIndex] = '\0';
        serialInProgress = false;
        serialIndex = 0;
        newSerialData = true;
      }
    } else if (serialChar == startMarker) {
      serialInProgress = true;
    }
  }
  if (newSerialData == true) {
    newSerialData = false;
    char * strtokIndex;
    strtokIndex = strtok(serialInputChars," ");
    char activity = strtokIndex[0];    // activity is our first parameter
    strtokIndex = strtok(NULL," ");       // space is null, separator
    uint8_t nodeId;
    uint8_t pin;
    bool state;
    if (activity == 'P') {
      nodeId = strtoul(strtokIndex, NULL, 10);
      strtokIndex = strtok(NULL, " ");
      pin = strtoul(strtokIndex, NULL, 10);
      strtokIndex = strtok(NULL, " ");
      state = strtoul(strtokIndex, NULL, 10); // get value of the angle or dimming
    }
    switch (activity) {
      case 'D':
        displayNetworkNodes(mesh);
        break;

      case 'H':
        displayHelp();
        break;

      case 'P':
        setNodePin(mesh, nodeId, pin, state);
        break;

      default:
        break;
    }
  }
}

void displayHelp() {
  Serial.println(F("Commands:"));
  Serial.println(F("H - Display this help"));
  Serial.println(F("D - Display all network nodes"));
  Serial.println(F("P nodeId pin state - Set state of the pin on the node"));
}

void displayNetworkNodes(RF24Mesh *mesh) {
  Serial.println(" ");
  Serial.println(F("------Assigned Addresses------"));
  for (int i = 0; i < mesh->addrListTop; i++) {
    Serial.print(F("NodeID|RF24Network Address: "));
    Serial.print(mesh->addrList[i].nodeID);
    Serial.print(F("|0"));
    Serial.println(mesh->addrList[i].address, OCT);
  }
  Serial.println(F("------------------------------"));
}

void setNodePin(RF24Mesh *mesh, uint8_t nodeId, uint8_t pin, bool state) {
  Serial.print(F("Set pin Node|Pin|State: "));
  Serial.print(nodeId);
  Serial.print(F("|"));
  Serial.print(pin);
  Serial.print(F("|"));
  Serial.println(state);
}
