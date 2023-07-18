#include <filesystem>
#include <sstream>

#include <device.h>
#include <sys/mount.h>

StorageDevice::
StorageDevice(std::string identifier, std::string dev_path, std::string dev_name)
    : _uuid(identifier), _dev_path(dev_path), _dev_name(dev_name)
{}

StorageDevice::
StorageDevice()
{}

void StorageDevice::
mount_device()
{
    std::string mount_path = "/backup/volumes/";
    mount_path += this->_uuid;
    int mount_status = mount(this->_dev_path.c_str(), mount_path.c_str(),
            "devtmpfs", 0, NULL);

    if (mount_status == -1)
    {
        std::stringstream oss;
        oss << "Unable to mount " << this->_dev_path << " ( " << this->_uuid
            << " ) to " << mount_path;
        this->_gui_thread->print(oss.str());
    }
    else
    {
        this->_mount_path = mount_path;
    }
}
