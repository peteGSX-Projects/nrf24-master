#include "SerialFunctions.h"
#include "NetworkFunctions.h"

/*
This code is likely to be replaced by the HAL driver directly on the EX-CommandStation

When device driver calls the various IODevice functions:
- _begin()
- _configure()
- _write()
- _writeAnalogue()
- _read()
- _loop()

This should send those to the appropriate node to perform that function
- How to get Vpin to node/MUX/channel/device address similar to how a MUX does now?

Updated logic to implement:
- Serial command instructions to set pin states
- <P nodeId pin state> - Set the specified pin on the network node high/low
- <S nodeId muxAddress muxChannel deviceAddress pin servoPWM> - Set servo on I2C device to PWM value
- <V nodeId muxAddress muxChannel deviceAddress pin state> - Set (v)pin on I2C device high/low

Need to receive values also:
- <A nodeId muxAddress muxChannel deviceAddress pin> - Read analogue value from I2C device pin
- <D nodeId muxAddress muxChannel deviceAddress pin> - Read digital state from I2C device pin
- <Q nodeId pin> - Read network node's digital pin

Other things to do:
- How to flag if an I2C device is offline?
*/

void setup() {
  Serial.begin(115200);
  while (!Serial) {}  // some boards need this because of native USB capability
  delay(2000);
  Serial.println(F("nRF24L01 mesh testing"));
  displayHelp();
}

void loop() {
  processMesh();
  processNetwork();
  processSerialInput(&mesh);
}
