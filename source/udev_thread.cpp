#include <udev_thread.h>
#include <gui_thread.h>

#include <cstring>
#include <device.h>

#include <unistd.h>
#include <fcntl.h>

#include <sstream>

void UDEVThread::
set_gui_thread(GUIThread* gui_thread)
{
    this->_gui_thread = gui_thread;
}

void UDEVThread::
set_udev_context(udev* context)
{
    this->_udev_context = context;
}

inline std::string UDEVThread::
get_udev_property(udev_device* device, const char* prop_name)
{
    const char* result = udev_device_get_property_value(device, prop_name);
    if (result != NULL)
        return result;
    return "";
}

void UDEVThread::
exit()
{ }

StorageDevice* UDEVThread::
find_device_by_uuid(std::string uuid)
{
    // Lock the set of storage devices and search.
    StorageDevice* found = nullptr;
    for (StorageDevice& current_device : this->_storage_devices)
    {
        if (current_device.get_uuid() == uuid)
        {
            found = &current_device;
            break;
        }
    }
    return found;
}

void UDEVThread::
device_update(std::string event_message, udev_device* event_device)
{

    // First, turn it into a USB device.
    udev_device* usb_device = udev_device_get_parent_with_subsystem_devtype(event_device, "usb", "usb_device");
    if (usb_device == NULL)
        return; // Nothing to do, not a USB device.

    // Get the device properties.
    std::string device_uuid = this->get_udev_property(event_device, "ID_FS_UUID");
    std::string device_path = this->get_udev_property(event_device, "DEVNAME");
    std::string device_vendor = this->get_udev_property(usb_device, "ID_VENDOR");
    std::string device_serial = this->get_udev_property(usb_device, "ID_USB_SERIAL_SHORT");

    // Ensure that we have a UUID.
    if (device_uuid.empty())
        return;

    // The device was added.
    if (event_message == "add")
    {
        pthread_mutex_lock(&this->_m_storage_devices);
        if (this->find_device_by_uuid(device_uuid) == NULL)
        {
            // Insert the device.
            this->_storage_devices.emplace_back(StorageDevice(device_uuid,
                        device_path, device_vendor));

            // Set the GUI thread onto the devices should they need to print out.
            StorageDevice& current_device = this->_storage_devices.back();
            current_device.set_gui_thread(this->_gui_thread);

            // Insert into event log.
            std::stringstream add_message;
            add_message << "Device " << device_vendor << " [ " << device_uuid
                << " ] was added to the device list.";
            this->_gui_thread->print(add_message.str());
        }
        else
        {
            this->_gui_thread->print("Unable to add device... device exists already!");
        }
        pthread_mutex_unlock(&this->_m_storage_devices);
    }

    // The device was changed.
    if (event_message == "change")
    {
        pthread_mutex_lock(&this->_m_storage_devices);
        StorageDevice* device_found = this->find_device_by_uuid(device_uuid);
        if (device_found != NULL)
        {
            device_found->set_dev_path(device_path);
            device_found->set_dev_name(device_vendor);

            // Insert into event log.
            std::stringstream update_message;
            update_message << "Device " << device_vendor << " [ " << device_uuid
                << " ] was updated in the device list.";
            this->_gui_thread->print(update_message.str());

        }
        else
        {
            this->_gui_thread->print("Unable to update device... device not found!");
        }
        pthread_mutex_unlock(&this->_m_storage_devices);

    }

    // The device was removed.
    if (event_message == "remove")
    {
        pthread_mutex_lock(&this->_m_storage_devices);

        bool found = false;
        size_t index;
        for (index = 0; index < this->_storage_devices.size(); ++index)
        {
            if (this->_storage_devices[index].get_uuid() == device_uuid)
            {
                found = true;
                break;
            }
        }

        if (found == true)
        {
            this->_storage_devices.erase(this->_storage_devices.begin() + index);
            std::stringstream remove_message;
            remove_message << "Device " << device_vendor << " [ " << device_uuid
                << " ] was remove from the device list.";
            this->_gui_thread->print(remove_message.str());
        }
        else
        {
            this->_gui_thread->print("Unable to remove device... device not found!");
        }

        pthread_mutex_unlock(&this->_m_storage_devices);
    }
}

void UDEVThread::
main()
{

	// -------------------------------------------------------------------------
    // Enumerate existing devices
	// -------------------------------------------------------------------------

    // Create our enumeration struct.
	udev_enumerate* enum_inst = udev_enumerate_new(this->_udev_context);
	
    // Only block types with property "partition" in this enum.
	udev_enumerate_add_match_subsystem(enum_inst, "block");
	udev_enumerate_add_match_property(enum_inst, "DEVTYPE", "partition");
	
    // Scan the devices.
	udev_enumerate_scan_devices(enum_inst);
	udev_list_entry* devices = udev_enumerate_get_list_entry(enum_inst);
	
    // Loop through the devices.
	udev_list_entry* device_entry;
	udev_list_entry_foreach(device_entry, devices)
	{
        // Get the path of the device.
		const char* path = udev_list_entry_get_name(device_entry);

        // If the path exists, then we can inspect.
		if (path)
		{
            // Determine if it is a block and USB device.
			udev_device* block_device = udev_device_new_from_syspath(this->_udev_context, path);
			udev_device* usb_device = udev_device_get_parent_with_subsystem_devtype(block_device, "usb", "usb_device");
			
            // Must be a block and USB device.
			if (block_device && usb_device)
                this->device_update("add", block_device);

            // Unref the device afterwards.
            udev_device_unref(block_device);
		}
    }

    // Unref the enumeration instance.
    udev_enumerate_unref(enum_inst);

	// -------------------------------------------------------------------------
	// Initialize UDEV monitor.
	// -------------------------------------------------------------------------

	// Create the monitor.
	udev_monitor* monitor = udev_monitor_new_from_netlink(this->_udev_context, "udev");
    udev_monitor_filter_add_match_subsystem_devtype(monitor, "block", "partition");
    udev_monitor_enable_receiving(monitor);

	// Fix the file descriptor to be blocking.
    int fd = udev_monitor_get_fd(monitor);
	int udev_monitor_fd_flags = fcntl(fd, F_GETFL);
	fcntl(fd, F_SETFL, udev_monitor_fd_flags & ~O_NONBLOCK);

	// -------------------------------------------------------------------------
	// Main runtime.
	// -------------------------------------------------------------------------
    // The UDEV monitor doesn't busy-spin when waiting on devices; it will block
    // until a device is caught and is ready to be processed. Because of this,
    // the UDEV thread doesn't close in the typical behavior we'd expect. We aren't
    // relying on the GUI thread's runtime state flag to be true every cycle since
    // the loop doesn't cycle very often.
    //
    // The UDEV thread is force-closed by the main thread when the GUI thread
    // exits using pthread_cancel(). Normally, you wouldn't want to do this since
    // resources may not be freed, however there are mechanisms in place to clean
    // up should the thread be forced to close.
    //
    // Essentially, main will lock the loop in case an iteration is in process.
    // Then it will run the exit() routine which cleans up dynamically allocated
    // resources and then exit.

	while (this->_gui_thread->get_runtime_state())
	{

        // TODO(Chris): We should use a mutex lock to check if the thread is about
        // to close. The main thread will lock that mutex during this process,
        // preventing the runtime from continuing. It will then free resources
        // and then close.

		// We will poll the devices from our monitor.
		struct udev_device* event_device = udev_monitor_receive_device(monitor);
		if (event_device == NULL)
		{
			// We will then sleep for 16ms here.
			// This will prevent the processor from being hit with too many loop iterations.
			usleep(16000);
		}
		else
		{
			// Get the action type.
			const char* action_message = udev_device_get_action(event_device);
            if (action_message == NULL)
                continue;
            this->device_update(action_message, event_device);
            udev_device_unref(event_device);
		}
	}

}
