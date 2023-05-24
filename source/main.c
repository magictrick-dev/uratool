#include <stdlib.h>
#include <stdio.h>
#include <core/helpers.h>
#include <core/primitives.h>
#include <libusb-1.0/libusb.h>

int
main(int argc, char** argv)
{

	// Initialize the context.
	libusb_context* libusb_context_ptr = NULL;
	errno lib_usb_ret = libusb_init(&libusb_context_ptr);
	FAST_EXIT_ON_ERROR(!lib_usb_ret, "Unable to initialize libusb.\n", -1);

	libusb_device **list;
	ssize_t cnt = libusb_get_device_list(NULL, &list);
	if (cnt < 0) FAST_EXIT_ON_ERROR(false, "Devices less than 0.\n", -2);
	
	for (ssize_t index = 0; index < cnt; ++index)
	{

		libusb_device* current_device = list[index];
		libusb_device_handle* current_handle = NULL;
		errno handle_ret = libusb_open(current_device, &current_handle);
		
		if (handle_ret != 0)
		{
			if (handle_ret == LIBUSB_ERROR_ACCESS)
				printf("Access error.\n");
			printf("Unable to open USB device: %ld\n", index);
			continue;
		}

		printf("Found USB device: %ld\n", index);
		libusb_close(current_handle);

	}

	libusb_free_device_list(list, 1);
	
	// Deinitialize the context.
	libusb_exit(libusb_context_ptr);

	return 0;
}

