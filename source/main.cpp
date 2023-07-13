#include <iostream>
#include <string>
#include <cstring>

#include <unistd.h>
#include <libudev.h>

#include <core/primitives.h>
#include <core/threading.h>
#include <gui_thread.h>
#include <udev_thread.h>

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
	udev_thread->set_gui_thread(gui_thread); // Pass the GUI thread to the UDEV thread.
	udev_thread->set_udev_context(udev_new()); // Create a udev context for the thread.
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
	udev_unref(udev_thread->get_udev_context());

}
