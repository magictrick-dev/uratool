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
        StorageDevice();
        StorageDevice(std::string uuid,
                std::string dev_path,
                std::string device_name);

        inline std::string      get_uuid() const     { return this->_uuid; }
        inline std::string      get_dev_path() const { return this->_dev_path; }
        inline std::string      get_dev_name() const { return this->_dev_name; }

        inline void     set_uuid(std::string uuid)      { this->_uuid = uuid; }
        inline void     set_dev_path(std::string path)  { this->_dev_path = path; }
        inline void     set_dev_name(std::string name)  { this->_dev_name = name; }

    protected:
        std::string     _uuid;
        std::string     _dev_path;
        std::string     _dev_name;


};


#endif
