#include "I2CDevice.h"
#include <Arduino.h>

I2CDevice *I2CDevice::_first = nullptr;
uint8_t I2CDevice::_deviceCount = 0;

I2CDevice::I2CDevice(uint8_t nodeId, uint8_t address, uint8_t muxAddress, uint8_t muxChannel) :
  _nodeId(nodeId), _address(address), _muxAddress(muxAddress), _muxChannel(muxChannel), _next(nullptr) {
    if (!_first) {
      _first = this;
    } else {
      I2CDevice *current = _first;
      while (current->_next != nullptr) {
        current = current->_next;
      }
      current->_next = this;
    }
    _deviceCount++;
  }

uint8_t I2CDevice::getNodeId() {
  return _nodeId;
}

uint8_t I2CDevice::getAddress() {
  return _address;
}

uint8_t I2CDevice::getMUXAddress() {
  return _muxAddress;
}

uint8_t I2CDevice::getMUXChannel() {
  return _muxChannel;
}

I2CDevice *I2CDevice::getFirst() {
  return _first;
}

I2CDevice *I2CDevice::getNext() {
  return _next;
}

bool I2CDevice::isAddressTaken(uint8_t address) {
  for (I2CDevice *device = getFirst(); device; device = device->getNext()) {
    if (device->getMUXAddress() == 0 && device->getMUXChannel() == 255 && device->getAddress() == address) {
      return true;
    }
  }
  return false;
}

uint8_t I2CDevice::getDeviceCount() {
  return _deviceCount;
}
