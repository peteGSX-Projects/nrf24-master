#include "RF24.h"
#include "RF24Network.h"
#include "RF24Mesh.h"
#include <SPI.h>
#include "config.h"
#include "NodeI2CDevice.h"

/**** Configure the nrf24l01 CE and CS pins ****/
RF24 radio(CE_PIN, CS_PIN);
RF24Network network(radio);
RF24Mesh mesh(radio, network);

/*
Protocol to send list of I2C devices to the master:
- At node startup, I2C scan first
- Start networking when scan complete
- Network joining completed next
- Master requests the device list - PacketType::RequestI2CList
- Node responds and sends the device list
- Master acknowledges successful receipt - PacketType::I2CListReceived
- Master stores in another linked list, this one associated with the node ID

To request device lists, probably need a flag to indicate they're received for each node
Need two linked lists for this, one for the nodes, and one for the devices
- Use mesh.addrList to loop through each known node
- If in linked list, check flag
- If flag true, move on
- If flag false, request list
- Probably need a delay between requests and a retry count perhaps?
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
  NodeHeartbeat = 1,
  RequestDeviceList = 65,
  NodeDeviceList = 66,
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
  displayAddressList();
  processNodes();
}

/**
 * @brief Process incoming network traffic according to type.
 * 
 */
void processNetwork() {
  RF24NetworkHeader header;
  network.peek(header);
  uint32_t data = 0;
  Serial.print(F("Packet type "));
  Serial.print(header.type);
  Serial.print(F(" from "));
  Serial.println(header.from_node, OCT);
  switch (header.type) {
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
      Serial.print(F("NodeID|RF24Network Address: "));
      Serial.print(mesh.addrList[i].nodeID);
      Serial.print(F("|0"));
      Serial.println(mesh.addrList[i].address, OCT);
    }
    Serial.println(F("------------------------------"));
  }
}

/**
 * @brief Process incoming timer data from a node
 * 
 * @param header RF24NetworkHeader
 * @param data Timer data in ms
 */
void processNodeDevices(RF24NetworkHeader header, uint32_t data) {
  Serial.print(F("Received data: from_node|to_node|id|type|next_id: "));
  Serial.print(header.from_node, OCT);
  Serial.print(F("|"));
  Serial.print(header.to_node, OCT);
  Serial.print(F("|"));
  Serial.print(header.id);
  Serial.print(F("|"));
  Serial.print(header.type);
  Serial.print(F("|"));
  Serial.println(header.next_id);
  network.read(header, &data, sizeof(data));
  Serial.println(data);
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
            requestNodeDevices(nodeId);
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
  // for (MeshNode *meshNode = MeshNode::getFirst(); meshNode; meshNode = meshNode->getNext()) {
  //   bool deleteNode = true;
  //   for (uint8_t i = 0; i < mesh.addrListTop; i++) {
  //     uint8_t nodeId = mesh.addrList[i].nodeID;
  //     if (meshNode->getNodeId() == nodeId) {
  //       deleteNode = false;
  //       continue;
  //     }
  //   }
  //   if (deleteNode) {
  //     meshNode->deleteNode();
  //   }
  // }
}

/**
 * @brief Request the list of I2C devices from the specified node ID
 * 
 * @param nodeId ID of the node to request the device list from
 */
void requestNodeDevices(uint8_t nodeId) {
  uint8_t data = 0;
  mesh.write(&data, PacketType::RequestDeviceList, sizeof(data), nodeId);
}
