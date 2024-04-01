#ifndef NODEI2CDEVICE_H
#define NODEI2CDEVICE_H

#include <Arduino.h>

class NodeI2CDevice;

class MeshNode {

public:
  /// @brief Constructor
  /// @param nodeId Node ID - valid is 1 to 255, 0 defines invalid (master can only be 0)
  MeshNode(uint8_t nodeId);

  /// @brief Get the node ID
  /// @return Node ID
  uint8_t getNodeId();

  /// @brief Set the number of devices attached to the node
  /// @param deviceCount Number of devices received from the node
  void setExpectedDeviceCount(uint8_t deviceCount);

  /// @brief Get the expected number of devices attached to this node
  /// @return Expected number of devices reported by the node
  uint8_t getExpectedDeviceCount();

  /// @brief Get the counted number of devices attached to this node
  /// @return Count of devices attached to this node
  uint8_t getDeviceCount();

  /// @brief Get the last timestamp (in ms) the device list was requested for this node
  /// @return Timestamp in ms
  unsigned long getLastDeviceListRequest();

  /// @brief Set the timestamp the device list for this node was requested
  /// @param ms Timestamp in ms
  void setLastDeviceListRequest(unsigned long ms);

  /// @brief Query if the device list for this node has been received
  /// @return True if list received, false otherwise
  bool receivedDeviceList();

  /// @brief Set flag that the device list for this node has been received
  void setReceivedDeviceList();

  /// @brief Get the first node in the list
  /// @return Pointer to the first MeshNode object
  static MeshNode *getFirst();

  /// @brief Get the next node from the list
  /// @return Pointer to the next MeshNode object
  MeshNode *getNext();

  /// @brief Add a device associated with this node
  /// @param address I2C address of the device
  /// @param muxAddress I2C address of a MUX if it is behind one, otherwise leave blank for no MUX
  /// @param muxChannel Channel number of a MUX if it is behind one, otherwise leave blank for no MUX/channel
  void addDevice(uint8_t address, uint8_t muxAddress = 0, uint8_t muxChannel = 255);

  /// @brief Get the first device associated with this node
  /// @return Pointer to the first device
  NodeI2CDevice *getFirstDevice();

  /// @brief Delete this mesh node object, and all associated devices
  void deleteNode();

  /// @brief Destructor, which also deletes all associated devices
  ~MeshNode();

  /// @brief Set this node's device list using the buffer containing the serialised devices
  /// @param deviceBuffer Pointer to the buffer containing the serialised devices
  /// @param bufferSize Size of the buffer
  void setDeviceList(byte *deviceBuffer, size_t bufferSize);

private:
  uint8_t _nodeId;
  uint8_t _expectedDeviceCount;
  uint8_t _deviceCount;
  unsigned long _lastDeviceListRequest;
  bool _receivedDeviceList;
  static MeshNode *_first;
  MeshNode *_next;
  NodeI2CDevice *_firstDevice;

};

/// @brief Class for each I2C object detected, with all objects stored in a linked list
class NodeI2CDevice {
public:
  /// @brief Constructor
  /// @param address I2C address of the device
  /// @param muxAddress I2C address of a MUX the device is behind (0 = main bus)
  /// @param muxChannel Channel of a MUX the devices is on (255 = main bus)
  NodeI2CDevice(uint8_t address, uint8_t muxAddress, uint8_t muxChannel);

  /// @brief Get the I2C address of this device
  /// @return I2C address
  uint8_t getAddress();

  /// @brief Get the I2C address of a MUX this device is behind
  /// @return 0 for no MUX, or I2C address of the MUX
  uint8_t getMUXAddress();

  /// @brief Get the channel number of the MUX this device is on
  /// @return 255 for no MUX, or channel number
  uint8_t getMUXChannel();

  /// @brief Set the pointer to the next device in the list
  /// @param nextDevice Pointer to the next device in the list
  void setNext(NodeI2CDevice *nextDevice);

  /// @brief Get the next I2C device in the linked list
  /// @return I2C device object
  NodeI2CDevice *getNext();

private:
  uint8_t _address;
  uint8_t _muxAddress;
  uint8_t _muxChannel;
  NodeI2CDevice *_next;

};

#endif
