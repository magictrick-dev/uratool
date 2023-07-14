#ifndef URATOOL_DEVICE_H
#define URATOOL_DEVICE_H
#include <string>

/**
 * Storage devices represent the various USB devices that are currently available
 * on the system. Devices themselves are then designed to be mounted and interacted
 * with by the system. For now this is a filler class with zero functionality.
 */
class StorageDevice
{

    public:
        StorageDevice(std::string identifier);

    protected:
        std::string     _uuid;
        std::string     _mount_point;
        std::string     _device_name;

};


#endif
