#include "RF24.h"
#include "RF24Network.h"
#include "RF24Mesh.h"
#include <SPI.h>
#include "config.h"

/**** Configure the nrf24l01 CE and CS pins ****/
RF24 radio(CE_PIN, CS_PIN);
RF24Network network(radio);
RF24Mesh mesh(radio, network);

/*
Theory to test for dynamic node IDs:
- Each node starts with reserved node ID 255
- At start up, joins network with this ID
- Master allocates next node ID
- Restart with allocated node ID
- Rejoin network to obtain new address

Extra step so user node's only can join the network:
- Defined passphrase

Process:
- Master starts with mesh DHCP active
- Master has nextNodeID defined as 1 to allocate to the first node request
- Node starts with node ID 255
- Node attempts to join mesh and obtain mesh network address
- Payload should have a type that is a node ID request
- Master receives node ID request, assigns nextNodeID and increments
- Node receives nextNodeID
- Node sets itself to nextNodeID and rejoins network with this ID
- Node must heartbeat every x seconds to retain node ID

Questions:
- What happens if a node doesn't heartbeat, but comes back online with a duplicate ID?
- Is a node ID processed in one go, or is a queue/delay needed for other nodes requesting an ID?
*/

unsigned long masterDisplayDelay = 5000;
unsigned long lastDisplay = 0;
unsigned long nodeLastSeenDelay = 5000;

struct payload_t {
  unsigned long ms;
  unsigned long counter;
};

/// @brief Struct to maintain a list of valid mesh nodes including their mesh address and time last seen in ms
struct validNode_t {
  uint16_t address;
  unsigned long lastSeen;
};

/**
 * @brief Type of the packet. 0 - 127 are user-defined types, 128 - 255 are reserved for system.

User message types 1 through 64 will NOT be acknowledged by the network, while message types 65 through 127 will receive a network ACK.
System message types 192 through 255 will NOT be acknowledged by the network. Message types 128 through 192 will receive a network ACK.
 
 */
enum PacketType : unsigned char {
  NodeIDRequest = 0,
  NoAvailableNodeID = 1,
  InvalidNodeID = 2,
  NodeHeartbeat = 3,
  TimerData = 65,
};

// Static array to map valid node IDs to addresses
validNode_t nodeList[255];

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
  validateNodeList();
  displayAddressList();
}

/**
 * @brief Process incoming network traffic according to type.
 * 
 */
void processNetwork() {
  RF24NetworkHeader header;
  network.peek(header);
  uint32_t data = 0;
  switch (header.type) {
    case PacketType::TimerData:
      processTimerData(header, data);
      break;
    
    case PacketType::NodeIDRequest:
      allocateNodeID(header);
      break;

    case PacketType::NodeHeartbeat:
      processHeartbeat(header);
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
void processTimerData(RF24NetworkHeader header, uint32_t data) {
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
 * @brief Allocate the next available node ID
 * Logic:
 * - Default node ID is 255 indicating new ID required
 * - Any nodeList entry with an address of 0 can be allocated
 * - If all entries are allocated, send PacketType::NoAvailableNodeID
 * 
 * @param header RF24NetworkHeader
 */
void allocateNodeID(RF24NetworkHeader header) {
  uint8_t newNodeID = 255;
  for (uint8_t i = 1; i < 255; i++) {
    if (nodeList[i].address == 0) {
      newNodeID = i;
      break;
    }
  }
  if (newNodeID == 255) {
    // PacketType::NoAvailableNodeID
  } else {
    // PacketType::NodeIDRequest;
    uint16_t nodeAddress = header.from_node;
    nodeList[newNodeID].address = nodeAddress;
    nodeList[newNodeID].lastSeen = millis();
    // uint16_t nodeAddress = header.from_node;
    // nodeList[newNodeID] = nodeAddress;
    // RF24NetworkHeader writeHeader = (nodeAddress, PacketType::NodeIDRequest);
    // mesh.write(&writeHeader, newNodeID, sizeof(newNodeID));
  }
}

/**
 * @brief Function to validate nodes are still online or clean up stale ones
 * 
 */
void validateNodeList() {
  for (uint8_t i = 1; i < 255; i++) {
    if (millis() - nodeList[i].lastSeen > nodeLastSeenDelay) {
      nodeList[i].address = 0;
    }
  }
}

/**
 * @brief Process a received heartbeat packet from a node
 * 
 * @param header RF24NetworkHeader object
 */
void processHeartbeat(RF24NetworkHeader header) {
  uint16_t nodeAddress = header.from_node;
  uint8_t nodeID = mesh.getNodeID(nodeAddress);
  if (nodeList[nodeID].address != nodeAddress) return;
  nodeList[nodeID].lastSeen = millis();
}
