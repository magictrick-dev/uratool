#include <filesystem>
#include <sstream>

#include <application.h>
#include <device.h>
#include <string.h>
#include <errno.h>
#include <sys/mount.h>

StorageDevice::
StorageDevice(std::string identifier, std::string dev_path, std::string dev_name)
    :   _uuid(identifier), _dev_path(dev_path), _dev_name(dev_name),
        _is_mounted(false), _mount_path("")
{}

StorageDevice::
StorageDevice()
{}

StorageDevice::
~StorageDevice()
{
    // Attempt to unmount the device before destructing.
    this->unmount_device();
}

void StorageDevice::
unmount_device()
{
    if (this->is_mounted())
    {
        int unmount_status = umount(this->_mount_path.c_str());
        if (unmount_status == -1)
        {
            std::stringstream oss;
            oss << "Unable to unmount " << this->_dev_path << " ( " << this->_uuid
                << " ) at " << this->_mount_path;
            Application::print(oss.str());

            const char* error_string = strerror(errno);
            if (error_string != NULL)
            {
                std::string error_message = error_string;
                Application::print(error_message);
            }
        }
        else
        {
            std::stringstream oss;
            oss << "The device with UUID " << this->_uuid
                << " was succesfully unmounted from "
                << this->_mount_path;
            Application::print(oss.str());
            this->_is_mounted = false;
            this->_mount_path = "";
        }
    }
}

void StorageDevice::
mount_device()
{
    std::string mount_path = "/backup/volumes/";
    mount_path += this->_uuid;

    // Determine if the path exists.
    if (!std::filesystem::exists(mount_path))
    {
        std::error_code e_code;
        if (!std::filesystem::create_directory(mount_path, e_code))
        {
            std::stringstream oss;
            oss << "Unable to create directory for volume at "
                << mount_path << ", reason: " << std::endl;
            oss << "    " << e_code.message();
            Application::print(oss.str());
        }
    }

    // Attempt to mount.
    int mount_status = mount(this->_dev_path.c_str(), mount_path.c_str(),
            "vfat", 0, NULL);

    // If the mount wasn't successful, then describe why.
    if (mount_status == -1)
    {
        std::stringstream oss;
        oss << "Unable to mount " << this->_dev_path << " ( " << this->_uuid
            << " ) to " << mount_path << ":" << std::endl;

        const char* error_string = strerror(errno);
        if (error_string != NULL)
        {
            std::string error_message = error_string;
            oss << "    " << error_message;
        }
        else
        {
            oss << "    Unable to get reason";
        }

        Application::print(oss.str());

    }
    else
    {
        this->_mount_path = mount_path;
        this->_is_mounted = true;
        std::stringstream oss;
        oss << "The device with UUID " << this->_uuid << " was succesfully mounted to "
            << this->_mount_path;
        Application::print(oss.str());
    }
}
