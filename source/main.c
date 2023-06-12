#include <stdlib.h>
#include <stdio.h>
#include <core/helpers.h>
#include <core/primitives.h>

#include <libudev.h>

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
				printf("PARTUUID: %s\n", udev_device_get_property_value(block_device, "ID_PART_ENTRY_UUID"));
				printf("DEVTYPE: %s\n", udev_device_get_property_value(block_device, "DEVTYPE"));
				printf("VENDOR: %s\n", udev_device_get_sysattr_value(usb_device, "idVendor"));
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
	uratool_enumerate_devices(&instance);

	// Close the enumeration.
	uratool_close_udev_instance(&instance);
}




