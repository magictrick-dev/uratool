#include <udev_thread.h>
#include <gui_thread.h>

#include <cstring>
#include <unistd.h>
#include <fcntl.h>

#if 0
/**
 * We will need to enumerate the devices on udev startup to get the currently
 * active devices. We won't know what's in, what's out, without first doing this.
 * Otherwise, we will need to force the user to unplug and replug to make it work.
 */

void
uratool_enumerate_devices(uratool_udev_instance* instance)
{

	udev_enumerate* enum_inst = udev_enumerate_new(instance->context);
	
	udev_enumerate_add_match_subsystem(enum_inst, "block");
	udev_enumerate_add_match_property(enum_inst, "DEVTYPE", "partition");
	
	udev_enumerate_scan_devices(enum_inst);
	udev_list_entry* devices = udev_enumerate_get_list_entry(enum_inst);
	
	udev_list_entry* device_entry;
	udev_list_entry_foreach(device_entry, devices)
	{
		const char* path = udev_list_entry_get_name(device_entry);
	
		if (path)
		{
			udev_device* block_device = udev_device_new_from_syspath(instance->context, path);
			udev_device* usb_device = udev_device_get_parent_with_subsystem_devtype(block_device, "usb", "usb_device");
			
			if (block_device && usb_device)
			{
				printf("---------------------------\n");
				printf("DEVICE: %s\n", udev_device_get_sysname(block_device));
				printf("DEVNAME: %s\n", udev_device_get_property_value(block_device, "DEVNAME"));
				printf("DEVTYPE: %s\n", udev_device_get_property_value(block_device, "DEVTYPE"));
				printf("VENDOR: %s\n", udev_device_get_sysattr_value(usb_device, "idVendor"));
				printf("SERIAL: %s\n", udev_device_get_property_value(block_device, "ID_SERIAL_SHORT"));
				printf("\n");
			}

			URATOOL_UDEV_UNREF(block_device, udev_device_unref);
		}

	}


	URATOOL_UDEV_UNREF(enum_inst, udev_enumerate_unref);

}
#endif

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

udev* UDEVThread::
get_udev_context()
{
    return this->_udev_context;
}

void UDEVThread::
exit()
{ }

void UDEVThread::
main()
{

	// -------------------------------------------------------------------------
	// Initialize UDEV
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
	// Main runtime
	// -------------------------------------------------------------------------
	while (this->_gui_thread->get_runtime_state())
	{

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
			std::string event_name;
			event_name += "USB Action Caught: ";
			event_name += action_message;
			this->_gui_thread->print(event_name);

			// If it is an add, then we should inspect it.
			if (!strcmp(action_message, "add") || !strcmp(action_message, "change") || !strcmp(action_message, "remove"))
			{

				// First, turn it into a USB device.
				udev_device* usb_device = udev_device_get_parent_with_subsystem_devtype(event_device, "usb", "usb_device");

#if 0
				printf("    Type: %s\n", udev_device_get_devtype(event_device));
				printf("    Device Name: %s\n", udev_device_get_property_value(event_device, "DEVNAME"));
				printf("    Device UUID: %s\n", udev_device_get_property_value(event_device, "SYNTH_UUID"));
				printf("    Vendor: %s\n", udev_device_get_property_value(usb_device, "ID_VENDOR"));
				printf("    Short Serial: %s\n", udev_device_get_property_value(usb_device, "ID_USB_SERIAL_SHORT"));
#endif

				std::string device_name;
				device_name += "    Type: ";
				device_name += udev_device_get_devtype(event_device);
				this->_gui_thread->print(device_name);


                const char* device_uuid = udev_device_get_property_value(event_device, "ID_FS_UUID");
                if (device_uuid != NULL)
                {
                    std::string uuid_out = "    UUID: ";
                    uuid_out += device_uuid;
                    this->_gui_thread->print(uuid_out);
                }

			}

			// Clear up the udev device from memory.
			udev_device_unref(event_device);
		}
	}

	printf("Fell out of main loop.\n");

}
