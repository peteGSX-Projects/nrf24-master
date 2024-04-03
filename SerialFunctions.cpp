#include <Arduino.h>
#include "SerialFunctions.h"
#include "NetworkFunctions.h"

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
    uint8_t nodeId = 0;
    uint8_t muxAddress = 0;
    uint8_t muxChannel = 0;
    uint8_t deviceAddress = 0;
    uint8_t pin = 0;
    bool state = 0;
    uint16_t pwm = 0;
    if (activity == 'P' || activity == 'Q') {
      nodeId = strtoul(strtokIndex, NULL, 10);
      strtokIndex = strtok(NULL, " ");
      pin = strtoul(strtokIndex, NULL, 10);
      if (activity == 'P') {
        strtokIndex = strtok(NULL, " ");
        state = strtoul(strtokIndex, NULL, 10); // get value of the angle or dimming
      }
    } else if (activity == 'A' || activity == 'D' || activity == 'S' || activity == 'V') {
      nodeId = strtoul(strtokIndex, NULL, 10);
      strtokIndex = strtok(NULL, " ");
      muxAddress = strtoul(strtokIndex, NULL, 16);
      strtokIndex = strtok(NULL, " ");
      muxChannel = strtoul(strtokIndex, NULL, 10);
      strtokIndex = strtok(NULL, " ");
      deviceAddress = strtoul(strtokIndex, NULL, 16);
      strtokIndex = strtok(NULL, " ");
      pin = strtoul(strtokIndex, NULL, 10);
      if (activity == 'S') {
        strtokIndex = strtok(NULL, " ");
        pwm = strtoul(strtokIndex, NULL, 10);
      } else if (activity == 'V') {
        strtokIndex = strtok(NULL, " ");
        state = strtoul(strtokIndex, NULL, 10);
      }
    }
    switch (activity) {
      case 'A':
        // <A nodeId muxAddress muxChannel deviceAddress pin> - Read analogue value from I2C device pin
        Serial.println(F("Read remote I2C device's analogue pin"));
        readAnaloguePin(mesh, nodeId, muxAddress, muxChannel, deviceAddress, pin);
        break;
      
      case 'D':
        // <D nodeId muxAddress muxChannel deviceAddress pin> - Read digital state from I2C device pin
        Serial.println(F("Read remote I2C device's digital pin"));
        readDigitalPin(mesh, nodeId, muxAddress, muxAddress, deviceAddress, pin);
        break;
      
      case 'H':
        // <H> display help
        displayHelp();
        break;
      
      case 'N':
        // <N> display network node info
        displayNetworkNodes(mesh);
        break;

      case 'P':
        // <P nodeId pin state> - Set the specified pin on the network node high/low
        Serial.println(F("Set network node's digital pin state"));
        if (!setNetworkNodePin(mesh, nodeId, pin, state)) {
          Serial.println(F("Failed to write to mesh node"));
        }
        break;

      case 'Q':
        // <Q nodeId pin> - Read network node's digital pin
        Serial.println(F("Read network node's digital pin state"));
        if (!readNetworkNodePin(mesh, nodeId, pin)) {
          Serial.println(F("Failed to write to mesh node"));
        }
        break;

      case 'S':
        // <S nodeId muxAddress muxChannel deviceAddress pin servoPWM> - Set servo on I2C device to PWM value
        Serial.println(F("Set remote I2C device's PWM value"));
        setPWMPin(mesh, nodeId, muxAddress, muxChannel, deviceAddress, pin, pwm);
        break;

      case 'V':
        // <V nodeId muxAddress muxChannel deviceAddress pin state> - Set (v)pin on I2C device high/low
        Serial.println(F("Set remote I2C device's digital pin state"));
        setDigitalPin(mesh, nodeId, muxAddress, muxChannel, deviceAddress, pin, state);
        break;

      default:
        Serial.println(F("Invalid activity"));
        displayHelp();
        break;
    }
  }
}

void displayHelp() {
  Serial.println(F("Commands:"));
  Serial.println(F("<A nodeId muxAddress muxChannel deviceAddress pin> - Read analogue value from I2C device pin"));
  Serial.println(F("<D nodeId muxAddress muxChannel deviceAddress pin> - Read digital state from I2C device pin"));
  Serial.println(F("<H> - Display this help"));
  Serial.println(F("<N> - Display all network nodes"));
  Serial.println(F("<P nodeId pin state> - Set state of the pin on the node"));
  Serial.println(F("<S nodeId muxAddress muxChannel deviceAddress pin servoPWM> - Set servo on I2C device to PWM value"));
  Serial.println(F("<V nodeId muxAddress muxChannel deviceAddress pin state> - Set (v)pin on I2C device high/low"));
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
