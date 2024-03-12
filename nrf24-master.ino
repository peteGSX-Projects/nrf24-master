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
*/

uint8_t nodeId = 0;
uint8_t nextNodeId = 1;

unsigned long masterDisplayDelay = 5000;
unsigned long lastDisplay = 0;

struct payload_t {
  unsigned long ms;
  unsigned long counter;
};

void setup() {
  Serial.begin(115200);
  while (!Serial) {}  // some boards need this because of native USB capability
  delay(2000);
  Serial.print(F("nRF24L01 mesh testing - "));
  Serial.print(F(" Master, node ID: "));
  mesh.setNodeID(nodeId);
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
    RF24NetworkHeader header;
    network.peek(header);
    uint32_t data = 0;
    switch (header.type) {
      case 'M':
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
        break;

      default:
        network.read(header, 0, 0);
        Serial.println(header.type);
        break;
    }
  }
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
