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
}

NodeI2CDevice *MeshNode::getFirstDevice() {
  return _firstDevice;
}

void MeshNode::deleteNode() {
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
        NodeI2CDevice *temp = _firstDevice;
        _firstDevice = _firstDevice->getNext();
        delete temp;
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
