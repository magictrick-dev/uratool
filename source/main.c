#include <stdlib.h>
#include <stdio.h>
#include <core/helpers.h>
#include <core/primitives.h>

#include <libudev.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
// -----------------------------------------------------------------------------
// 
// -----------------------------------------------------------------------------


// -----------------------------------------------------------------------------
// udev_instance
// -----------------------------------------------------------------------------
// Contains information regarding a udev, and used for maintaining lifetimes of
// the udev constructs (reference counting, and what not).

typedef struct
{
	udev* context;
} uratool_udev_instance;

// -----------------------------------------------------------------------------
// udev_create & udev_close
// -----------------------------------------------------------------------------
// Helper functions which construct and deconstruct the udev lifetimes.

uratool_udev_instance
uratool_create_udev_instance()
{

	uratool_udev_instance instance = {};
	instance.context = udev_new();


	return instance;
}

void
uratool_close_udev_instance(uratool_udev_instance* instance)
{

	URATOOL_UDEV_UNREF(instance->context, udev_unref);

}

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

// -----------------------------------------------------------------------------
// Entry Point
// -----------------------------------------------------------------------------

int main(int argc, char *argv[]) {


	// Create a udev instance that we can use to enumerate devices.
	uratool_udev_instance instance = uratool_create_udev_instance();	

	//uratool_enumerate_devices(&instance);

    struct udev_monitor* mon = udev_monitor_new_from_netlink(instance.context, "udev");

    udev_monitor_filter_add_match_subsystem_devtype(mon, "block", "partition");
    udev_monitor_enable_receiving(mon);

    int fd = udev_monitor_get_fd(mon);
	int udev_monitor_fd_flags = fcntl(fd, F_GETFL);
	fcntl(fd, F_SETFL, udev_monitor_fd_flags & ~O_NONBLOCK);

	while (1) {

		// Get the event device.
		struct udev_device* event_device = udev_monitor_receive_device(mon);
		if (event_device == NULL)
		{
			sleep(1);
			continue;
		}

		// Get the action type.
		const char* action_message = udev_device_get_action(event_device);
		printf("Event: %s\n", action_message);

		// If it is an add, then we should inspect it.
		if (!strcmp(action_message, "add"))
		{
			printf("    Type: %s\n", udev_device_get_devtype(event_device));
			printf("    Device Name: %s\n", udev_device_get_property_value(event_device, "DEVNAME"));
		}

		// Clear up the udev device from memory.
		udev_device_unref(event_device);

		// Snooze, save on system resources on long-polling.
		sleep(1);
	}

	// Close the enumeration.
	uratool_close_udev_instance(&instance);
}




