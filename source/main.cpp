#include <iostream>
#include <string>
#include <cstring>

#include <pthread.h>
#include <ncurses.h>
#include <unistd.h>
#include <fcntl.h>
#include <libudev.h>

#include <core/primitives.h>
#include <core/threading.h>
#include <gui_thread.h>

/**
 * The UDEV thread works in tandem with the main thread--since UDEV itself is
 * blocking, regardless if no device is being actively monitored, then it will
 * hold until the process is killed or a device is polled. Unfortunately, this
 * doesn't make for a good application, therefore it is easier to sublet this
 * functionality behind another thread which we can kill when the GUI front end
 * goes dark. This unfortunately couples the interactivity into another thread,
 * however we can simply give the UDEVThread the GUIThread and have it function
 * with "main"-like operability.
 */
class UDEVThread : public Thread
{

	public:
		THREAD_MAIN(UDEVThread);
		virtual void 	exit();

	public:
		GUIThread* 		gui_thread;
		udev* 			udev_context;

};

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
	udev_monitor* monitor = udev_monitor_new_from_netlink(udev_context, "udev");
    udev_monitor_filter_add_match_subsystem_devtype(monitor, "block", "partition");
    udev_monitor_enable_receiving(monitor);

	// Fix the file descriptor to be blocking.
    int fd = udev_monitor_get_fd(monitor);
	int udev_monitor_fd_flags = fcntl(fd, F_GETFL);
	fcntl(fd, F_SETFL, udev_monitor_fd_flags & ~O_NONBLOCK);

	// -------------------------------------------------------------------------
	// Main runtime
	// -------------------------------------------------------------------------
	while (gui_thread->get_runtime_state())
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
			gui_thread->print(event_name);

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
				gui_thread->print(device_name);

			}

			// Clear up the udev device from memory.
			udev_device_unref(event_device);
		}
	}

	printf("Fell out of main loop.\n");

}

int
main(int argc, char** argv)
{

	// -------------------------------------------------------------------------
	// Initialize the GUI thread.
	// -------------------------------------------------------------------------

	ThreadingManager thread_manager;
	GUIThread* gui_thread = thread_manager.create_thread<GUIThread>();
	gui_thread->set_runtime_state(true);
	gui_thread->launch();

	// -------------------------------------------------------------------------
	// Initialize the UDEV thread.
	// -------------------------------------------------------------------------

	UDEVThread* udev_thread = thread_manager.create_thread<UDEVThread>();
	udev_thread->gui_thread = gui_thread; // Pass the GUI thread to the UDEV thread.
	udev_thread->udev_context = udev_new(); // Create a udev context for the thread.
	udev_thread->launch();

	// -------------------------------------------------------------------------
	// Continue running while the GUI is open.
	// -------------------------------------------------------------------------
	
	while (gui_thread->get_runtime_state())
	{
		// Sleep, we don't actually need to do anything.
		// We could perhaps give main the GUI thread responsibility and let
		// UDEV hinge on runtime state of GUI thread.
		usleep(32000); // 32ms
	}

	// -------------------------------------------------------------------------
	// If the GUI thread has closed, we can force-kill the UDEV thread.
	// You normally would not want this, because this potentially leaks
	// resources. In our cases, this shouldn't cause issues since we will
	// release these resources back to the OS as soon as the application closes.
	// -------------------------------------------------------------------------
	pthread_cancel(udev_thread->get_handle());
	udev_unref(udev_thread->udev_context);

}
