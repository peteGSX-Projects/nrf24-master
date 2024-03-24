#ifndef NODEI2CDEVICE_H
#define NODEI2CDEVICE_H

#include <Arduino.h>

/// @brief Class for each I2C object detected, with all objects stored in a linked list
class NodeI2CDevice {
public:
  /// @brief Constructor
  /// @param address I2C address of the device
  /// @param muxAddress I2C address of a MUX the device is behind (0 = main bus)
  /// @param muxChannel Channel of a MUX the devices is on (255 = main bus)
  NodeI2CDevice(uint8_t nodeId, uint8_t address, uint8_t muxAddress, uint8_t muxChannel);

  /// @brief Get the node ID this device is associated with
  /// @return Node ID
  uint8_t getNodeId();
  
  /// @brief Get the I2C address of this device
  /// @return I2C address
  uint8_t getAddress();

  /// @brief Get the I2C address of a MUX this device is behind
  /// @return 0 for no MUX, or I2C address of the MUX
  uint8_t getMUXAddress();

  /// @brief Get the channel number of the MUX this device is on
  /// @return 255 for no MUX, or channel number
  uint8_t getMUXChannel();

  /// @brief Get the first I2C device in the linked list
  /// @return I2C device object
  static NodeI2CDevice *getFirst();

  /// @brief Get the next I2C device in the linked list
  /// @return I2C device object
  NodeI2CDevice *getNext();

  /// @brief Check if the specified I2C address already exists on the main I2C bus
  /// @param address I2C address to check
  /// @return True if a device on the main I2C bus uses this address, false if not
  static bool isAddressTaken(uint8_t address);

  /// @brief Get the number of detected devices
  /// @return Number of detected devices
  static uint8_t getDeviceCount();

private:
  uint8_t _nodeId;
  uint8_t _address;
  uint8_t _muxAddress;
  uint8_t _muxChannel;
  static uint8_t _deviceCount;
  static NodeI2CDevice *_first;
  NodeI2CDevice *_next;

};

#endif
