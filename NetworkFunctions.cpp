#include "NetworkFunctions.h"

/**** Configure the nrf24l01 CE and CS pins ****/
RF24 radio(CE_PIN, CS_PIN);
RF24Network network(radio);
RF24Mesh mesh(radio, network);

void setupNetwork() {
  Serial.print(F("Setup network master, node ID: "));
  mesh.setNodeID(0);
  Serial.println(mesh.getNodeID());
  radio.begin();
  radio.setPALevel(RF24_PA_MIN, 0);
  if (!mesh.begin()) {
    Serial.println(F("Radio hardware not responding"));
    while(1) {}
  } else {
    Serial.println(F("Radio online"));
  }
}


void processMesh() {
  mesh.update();
  mesh.DHCP();
}

/**
 * @brief Process incoming network traffic according to type.
 * 
 */
void processNetwork() {
  if (network.available()) {
    RF24NetworkHeader header;
    network.peek(header);
    Serial.print(F("Packet type "));
    Serial.print(header.type);
    Serial.print(F(" from "));
    Serial.println(header.from_node, OCT);
    switch (header.type) {
      case PacketType::InitialiseEXIO:
        processEXIOInit(header, network, mesh);
        break;
      
      case PacketType::ReadDigitalPin:
        processNetworkNodePin(header, network, mesh);
        break;
      
      default:
        network.read(header, 0, 0);
        Serial.print(F("Unknown header received: "));
        Serial.println(header.type);
        break;
    }
  }
}

uint16_t readAnaloguePin(RF24Mesh *mesh, uint8_t nodeId, uint8_t muxAddress, uint8_t muxChannel, uint8_t deviceAddress, uint8_t pin) {
  int16_t nodeAddress = mesh->getAddress(nodeId);
  Serial.print(F("Read analogue pin: Node|Address|MUX|Channel|Device|Pin: "));
  Serial.print(nodeId);
  Serial.print(F("|"));
  Serial.print(nodeAddress, OCT);
  Serial.print(F("|0x"));
  Serial.print(muxAddress, HEX);
  Serial.print(F("|"));
  Serial.print(muxChannel);
  Serial.print(F("|0x"));
  Serial.print(deviceAddress, HEX);
  Serial.print(F("|"));
  Serial.println(pin);
  return 0;
}

bool readDigitalPin(RF24Mesh *mesh, uint8_t nodeId, uint8_t muxAddress, uint8_t muxChannel, uint8_t deviceAddress, uint8_t pin) {
  int16_t nodeAddress = mesh->getAddress(nodeId);
  Serial.print(F("Read digital pin: Node|Address|MUX|Channel|Device|Pin: "));
  Serial.print(nodeId);
  Serial.print(F("|"));
  Serial.print(nodeAddress, OCT);
  Serial.print(F("|0x"));
  Serial.print(muxAddress, HEX);
  Serial.print(F("|"));
  Serial.print(muxChannel);
  Serial.print(F("|0x"));
  Serial.print(deviceAddress, HEX);
  Serial.print(F("|"));
  Serial.println(pin);
  return 0;
}

bool initialiseEXIOExpander(RF24Mesh *mesh, uint8_t nodeId, uint8_t muxAddress, uint8_t muxChannel, uint8_t deviceAddress) {
  int16_t nodeAddress = mesh->getAddress(nodeId);
  Serial.print(F("Initialise EX-IOExpander Node|Address|MUX|Channel|Device: "));
  Serial.print(F("|"));
  Serial.print(nodeId);
  Serial.print(F("|"));
  Serial.print(nodeAddress, OCT);
  Serial.print(F("|0x"));
  Serial.print(muxAddress, HEX);
  Serial.print(F("|"));
  Serial.print(muxChannel);
  Serial.print(F("|0x"));
  Serial.println(deviceAddress, HEX);
  byte buffer[7] = {(uint8_t) muxAddress, (uint8_t) muxChannel, (uint8_t) deviceAddress, EXIOExpander::EXIOINIT, 18, (800 & 0xFF), (800 >> 8)};
  if (mesh->write(nodeAddress, &buffer, PacketType::InitialiseEXIO, sizeof(buffer))) return true;
  return false;
}

void processEXIOInit(RF24NetworkHeader &header, RF24Network &network, RF24Mesh &mesh) {
  byte deviceInfo[4];
  network.read(header, &deviceInfo, sizeof(deviceInfo));
  int16_t nodeAddress = header.from_node;
  uint8_t nodeId = mesh.getNodeID(nodeAddress);
  Serial.print(F("EXIO initialised Node|Address|MUX|Channel|Device|State: "));
  Serial.print(nodeId);
  Serial.print(F("|"));
  Serial.print(nodeAddress, OCT);
  Serial.print(F("|0x"));
  Serial.print(deviceInfo[0], HEX);
  Serial.print(F("|"));
  Serial.print(deviceInfo[1]);
  Serial.print(F("|0x"));
  Serial.print(deviceInfo[2], HEX);
  Serial.print(F("|"));
  Serial.println(deviceInfo[3]);
}

bool setNetworkNodePin(RF24Mesh *mesh, uint8_t nodeId, uint8_t pin, bool state) {
  int16_t nodeAddress = mesh->getAddress(nodeId);
  Serial.print(F("Set pin state Node|Address|Pin|State: "));
  Serial.print(nodeId);
  Serial.print(F("|"));
  Serial.print(nodeAddress, OCT);
  Serial.print(F("|"));
  Serial.print(pin);
  Serial.print(F("|"));
  Serial.println(state);
  byte pinState[2] = {pin, state};
  if (mesh->write(nodeAddress, &pinState, PacketType::SetDigitalPin, sizeof(pinState))) return true;
  return false;
}

bool readNetworkNodePin(RF24Mesh *mesh, uint8_t nodeId, uint8_t pin) {
  int16_t nodeAddress = mesh->getAddress(nodeId);
  Serial.print(F("Read pin state Node|Address|Pin: "));
  Serial.print(nodeId);
  Serial.print(F("|"));
  Serial.print(nodeAddress, OCT);
  Serial.print(F("|"));
  Serial.println(pin);
  if (mesh->write(nodeAddress, &pin, PacketType::ReadDigitalPin, sizeof(pin))) return true;
  return false;
}

void processNetworkNodePin(RF24NetworkHeader &header, RF24Network &network, RF24Mesh &mesh) {
  byte pinState[2];
  network.read(header, &pinState, sizeof(pinState));
  int16_t nodeAddress = header.from_node;
  uint8_t nodeId = mesh.getNodeID(nodeAddress);
  Serial.print(F("Received pin state Node|Address|Pin|State: "));
  Serial.print(nodeId);
  Serial.print(F("|"));
  Serial.print(nodeAddress, OCT);
  Serial.print(F("|"));
  Serial.print(pinState[0]);
  Serial.print(F("|"));
  Serial.println(pinState[1]);
}

void setPWMPin(RF24Mesh *mesh, uint8_t nodeId, uint8_t muxAddress, uint8_t muxChannel, uint8_t deviceAddress, uint8_t pin, uint16_t pwmValue) {
  int16_t nodeAddress = mesh->getAddress(nodeId);
  Serial.print(F("Set PWM pin: Node|Address|MUX|Channel|Device|Pin|PWM: "));
  Serial.print(nodeId);
  Serial.print(F("|"));
  Serial.print(nodeAddress, OCT);
  Serial.print(F("|0x"));
  Serial.print(muxAddress, HEX);
  Serial.print(F("|"));
  Serial.print(muxChannel);
  Serial.print(F("|0x"));
  Serial.print(deviceAddress, HEX);
  Serial.print(F("|"));
  Serial.print(pin);
  Serial.print(F("|"));
  Serial.println(pwmValue);
}

void setDigitalPin(RF24Mesh *mesh, uint8_t nodeId, uint8_t muxAddress, uint8_t muxChannel, uint8_t deviceAddress, uint8_t pin, bool state) {
  int16_t nodeAddress = mesh->getAddress(nodeId);
  Serial.print(F("Set digital pin: Node|Address|MUX|Channel|Device|Pin|State: "));
  Serial.print(nodeId);
  Serial.print(F("|"));
  Serial.print(nodeAddress, OCT);
  Serial.print(F("|0x"));
  Serial.print(muxAddress, HEX);
  Serial.print(F("|"));
  Serial.print(muxChannel);
  Serial.print(F("|0x"));
  Serial.print(deviceAddress, HEX);
  Serial.print(F("|"));
  Serial.print(pin);
  Serial.print(F("|"));
  Serial.println(state);
}
