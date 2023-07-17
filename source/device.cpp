#include <device.h>

StorageDevice::
StorageDevice(std::string identifier, std::string dev_path, std::string dev_name)
    : _uuid(identifier), _dev_path(dev_path), _dev_name(dev_name)
{}

StorageDevice::
StorageDevice()
{}
