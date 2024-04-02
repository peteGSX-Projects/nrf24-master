#include "RF24.h"
#include "RF24Network.h"
#include "RF24Mesh.h"
#include <SPI.h>
#include "config.h"
#include "NodeI2CDevice.h"
#include "SerialFunctions.h"

/**** Configure the nrf24l01 CE and CS pins ****/
RF24 radio(CE_PIN, CS_PIN);
RF24Network network(radio);
RF24Mesh mesh(radio, network);

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

Other things to do:
- How to flag if an I2C device is offline?
*/

unsigned long masterDisplayDelay = 5000;
unsigned long lastDisplay = 0;

struct payload_t {
  unsigned long ms;
  unsigned long counter;
};

/**
 * @brief Type of the packet. 0 - 127 are user-defined types, 128 - 255 are reserved for system.

User message types 1 through 64 will NOT be acknowledged by the network, while message types 65 through 127 will receive a network ACK.
System message types 192 through 255 will NOT be acknowledged by the network. Message types 128 through 192 will receive a network ACK.
 
 */
enum PacketType : unsigned char {
  ReadAnalogueVPin = 65,
  ReadDigitalVPin = 66,
  SetDigitalPin = 67,
  SetServoPWM = 68,
  SetDigitalVpin = 69,
};

void setup() {
  Serial.begin(115200);
  while (!Serial) {}  // some boards need this because of native USB capability
  delay(2000);
  Serial.print(F("nRF24L01 mesh testing - "));
  Serial.print(F(" Master, node ID: "));
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

void loop() {
  mesh.update();
  mesh.DHCP();
  if (network.available()) {
    processNetwork();
  }
  processSerialInput(&mesh);
  // displayAddressList();
  // processNodes();
}

/**
 * @brief Process incoming network traffic according to type.
 * 
 */
void processNetwork() {
  RF24NetworkHeader header;
  network.peek(header);
  unsigned long data;
  Serial.print(F("Packet type "));
  Serial.print(header.type);
  Serial.print(F(" from "));
  Serial.println(header.from_node, OCT);
  switch (header.type) {
    case PacketType::NodeDeviceCount:
      processNodeDeviceCount(header, data);
      break;
    
    case PacketType::NodeDeviceList:
      processNodeDevices(header, data);
      break;
    
    default:
      network.read(header, 0, 0);
      Serial.print(F("Unknown header received: "));
      Serial.println(header.type);
      break;
  }
}

/**
 * @brief Display the list of node IDs and assigned addresses at regular intervals
 * 
 */
void displayAddressList() {
  if (millis() - lastDisplay >= masterDisplayDelay) {
    lastDisplay = millis();
    Serial.println(" ");
    Serial.println(F("------Assigned Addresses------"));
    for (int i = 0; i < mesh.addrListTop; i++) {
      Serial.print(F("NodeID|RF24Network Address|Devices: "));
      Serial.print(mesh.addrList[i].nodeID);
      Serial.print(F("|0"));
      Serial.print(mesh.addrList[i].address, OCT);
      uint8_t deviceCount = 0;
      for (MeshNode *meshNode = MeshNode::getFirst(); meshNode; meshNode = meshNode->getNext()) {
        if (meshNode->getNodeId() == mesh.addrList[i].nodeID) {
          deviceCount = meshNode->getExpectedDeviceCount();
          continue;
        }
      }
      Serial.print(F("|"));
      Serial.println(deviceCount);
    }
    Serial.println(F("------------------------------"));
  }
}

/**
 * @brief Process the device count received from a node
 * 
 * @param header RF24NetworkHeader
 * @param data Device count
 */
void processNodeDeviceCount(RF24NetworkHeader header, unsigned long data) {
  int16_t nodeId = mesh.getNodeID(header.from_node);
  network.read(header, &data, sizeof(data));
  Serial.print(F("Node ID|Device count: "));
  Serial.print(nodeId);
  Serial.print(F("|"));
  Serial.println(data);
  for (MeshNode *meshNode = MeshNode::getFirst(); meshNode; meshNode = meshNode->getNext()) {
    if (meshNode->getNodeId() == nodeId) {
      meshNode->setExpectedDeviceCount(data);
      requestNodeDeviceList(nodeId);
      break;
    }
  }
}

/**
 * @brief Process the list of devices received from a node
 * 
 * @param header RF24NetworkHeader
 * @param data Device list
 */
void processNodeDevices(RF24NetworkHeader header, unsigned long data) {
  int16_t nodeId = mesh.getNodeID(header.from_node);
  network.read(header, &data, sizeof(data));
  byte buffer[sizeof(data)];
  for (size_t i = 0; i < sizeof(data); i++) {
    buffer[i] = (data >> (i * 8)) & 0xFF;
  }
  size_t bufferSize = sizeof(data);
  Serial.print(F("Received data: from_node|to_node|id|type|next_id|data|array: "));
  Serial.print(header.from_node, OCT);
  Serial.print(F("|"));
  Serial.print(header.to_node, OCT);
  Serial.print(F("|"));
  Serial.print(header.id);
  Serial.print(F("|"));
  Serial.print(header.type);
  Serial.print(F("|"));
  Serial.print(header.next_id);
  Serial.print(F("|"));
  Serial.print(data);
  Serial.print(F("|"));
  for (uint8_t i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i]);
  }
  Serial.println(F(""));
  for (MeshNode *meshNode = MeshNode::getFirst(); meshNode; meshNode = meshNode->getNext()) {
    if (meshNode->getNodeId() == nodeId) {
      meshNode->setDeviceList(buffer, bufferSize);
      if (meshNode->getExpectedDeviceCount() == meshNode->getDeviceCount()) {
        meshNode->setReceivedDeviceList();
      } else {
        Serial.print(F("Expected count|Actual count: "));
        Serial.print(meshNode->getExpectedDeviceCount());
        Serial.print(F("|"));
        Serial.println(meshNode->getDeviceCount());
      }
      break;
    }
  }
}

/**
 * @brief Function to ensure all nodes on the mesh have been queried for a device list
 * and deletes nodes no longer in the mesh
 * 
 */
void processNodes() {
  for (uint8_t i = 0; i < mesh.addrListTop; i++) {
    uint8_t nodeId = mesh.addrList[i].nodeID;
    bool createNode = true;
    for (MeshNode *meshNode = MeshNode::getFirst(); meshNode; meshNode = meshNode->getNext()) {
      if (meshNode->getNodeId() == nodeId) {
        createNode = false;
        if (!meshNode->receivedDeviceList()) {
          unsigned long timeNow = millis();
          if (timeNow - meshNode->getLastDeviceListRequest() > 1000) {
            Serial.print(F("Request device list for node ID "));
            Serial.println(nodeId);
            requestNodeDeviceCount(nodeId);
            meshNode->setLastDeviceListRequest(timeNow);
          }
        }
        continue;
      }
    }
    if (createNode || !MeshNode::getFirst()) {
      new MeshNode(nodeId);
    }
  }
  for (MeshNode *meshNode = MeshNode::getFirst(); meshNode; meshNode = meshNode->getNext()) {
    bool deleteNode = true;
    for (uint8_t i = 0; i < mesh.addrListTop; i++) {
      uint8_t nodeId = mesh.addrList[i].nodeID;
      if (meshNode->getNodeId() == nodeId) {
        deleteNode = false;
        continue;
      }
    }
    if (deleteNode) {
      meshNode->deleteNode();
    }
  }
}

/**
 * @brief Request the number of devices the specified node ID has
 * 
 * @param nodeId ID of the node to request the device count from
 */
void requestNodeDeviceCount(uint8_t nodeId) {
  uint8_t data = 0;
  mesh.write(&data, PacketType::RequestDeviceCount, sizeof(data), nodeId);
}

/**
 * @brief Request the list of I2C devices from the specified node ID
 * 
 * @param nodeId ID of the node to request the device list from
 */
void requestNodeDeviceList(uint8_t nodeId) {
  uint8_t data = 0;
  mesh.write(&data, PacketType::RequestDeviceList, sizeof(data), nodeId);
}
