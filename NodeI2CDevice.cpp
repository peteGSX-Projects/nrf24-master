#include "NodeI2CDevice.h"
#include <Arduino.h>

MeshNode *MeshNode::_first = nullptr;

MeshNode::MeshNode(uint8_t nodeId) :
  _nodeId(nodeId), _deviceCount(0), _lastDeviceListRequest(0), _receivedDeviceList(false), _next(nullptr), _firstDevice(nullptr) {
    if (!_first) {
      _first = this;
    } else {
      MeshNode *current = _first;
      while (current->_next != nullptr) {
        current = current->_next;
      }
      current->_next = this;
    }
  }

uint8_t MeshNode::getNodeId() {
  return _nodeId;
}

void MeshNode::setExpectedDeviceCount(uint8_t deviceCount) {
  _expectedDeviceCount = deviceCount;
}

uint8_t MeshNode::getExpectedDeviceCount() {
  return _expectedDeviceCount;
}

uint8_t MeshNode::getDeviceCount() {
  return _deviceCount;
}

unsigned long MeshNode::getLastDeviceListRequest() {
  return _lastDeviceListRequest;
}

void MeshNode::setLastDeviceListRequest(unsigned long ms) {
  _lastDeviceListRequest = ms;
}

bool MeshNode::receivedDeviceList() {
  return _receivedDeviceList;
}

void MeshNode::setReceivedDeviceList() {
  _receivedDeviceList = true;
}

MeshNode *MeshNode::getFirst() {
  return _first;
}

MeshNode *MeshNode::getNext() {
  return _next;
}

void MeshNode::addDevice(uint8_t address, uint8_t muxAddress, uint8_t muxChannel) {
  NodeI2CDevice *newDevice = new NodeI2CDevice(address, muxAddress, muxChannel);
  if (!_firstDevice) {
    _firstDevice = newDevice;
  } else {
    NodeI2CDevice *current = _firstDevice;
    while (current->getNext() != nullptr) {
      current = current->getNext();
    }
    current->setNext(newDevice);
  }
  _deviceCount++;
  Serial.print(F("New device address|MUX|channel|count: 0x"));
  Serial.print(newDevice->getAddress(), HEX);
  Serial.print(F("|0x"));
  Serial.print(newDevice->getMUXAddress(), HEX);
  Serial.print(F("|"));
  Serial.print(newDevice->getMUXChannel());
  Serial.print(F("|"));
  Serial.println(_deviceCount);
}

NodeI2CDevice *MeshNode::getFirstDevice() {
  return _firstDevice;
}

void MeshNode::deleteNode() {
  Serial.print(F("Deleting mesh node ID "));
  Serial.println(this->getNodeId());
  if (this == _first) {
    MeshNode *temp = _first;
    _first = _first->getNext();
    delete temp;
    return;
  }
  MeshNode *prev = nullptr;
  MeshNode *current = _first;
  while (current != nullptr && current != this) {
    prev = current;
    current = current->getNext();
  }
  // If the node is not found
  if (current == nullptr) {
    return;
  }
  // Remove the node from the linked list
  _next = current->getNext();
  delete current;
}

MeshNode::~MeshNode() {
  while (_firstDevice != nullptr) {
        Serial.print(F("Deleting device at address "));
        Serial.println(_firstDevice->getAddress());
        NodeI2CDevice *temp = _firstDevice;
        _firstDevice = _firstDevice->getNext();
        delete temp;
    }
}

void MeshNode::setDeviceList(byte *deviceBuffer, size_t bufferSize) {
  size_t offset = 0;
  // If we have an existing device list, delete it first
  while (_firstDevice != nullptr) {
    NodeI2CDevice *temp = _firstDevice;
    _firstDevice = _firstDevice->getNext();
    delete temp;
  }
  _firstDevice = nullptr;
  _deviceCount = 0;
  while (offset < bufferSize) {
    uint8_t address = deviceBuffer[offset++];
    uint8_t muxAddress = deviceBuffer[offset++];
    uint8_t muxChannel = deviceBuffer[offset++];
    addDevice(address, muxAddress, muxChannel);
  }
}

NodeI2CDevice::NodeI2CDevice(uint8_t address, uint8_t muxAddress, uint8_t muxChannel) :
  _address(address), _muxAddress(muxAddress), _muxChannel(muxChannel), _next(nullptr) {}

uint8_t NodeI2CDevice::getAddress() {
  return _address;
}

uint8_t NodeI2CDevice::getMUXAddress() {
  return _muxAddress;
}

uint8_t NodeI2CDevice::getMUXChannel() {
  return _muxChannel;
}

void NodeI2CDevice::setNext(NodeI2CDevice *nextDevice) {
  _next = nextDevice;
}

NodeI2CDevice *NodeI2CDevice::getNext() {
  return _next;
}
