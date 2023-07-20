// -----------------------------------------------------------------------------
// USB Replication Automation Tool  : URATool
// Developed by Chris DeJong        : Github @ magictrick-dev
// -----------------------------------------------------------------------------
// URATool is a USB replication tool designed to run as a standalone linux service
// to periodically replicate and backup USB devices on a schedule. As a TUI
// application, it's designed to be ran both as a long-term application or as a
// service. The original idea behind this application is to allow this to run on
// a low power device such as a R.Pi mounted to the wall with a monitor and a USB
// hub to attach USB drives to. These drives run on a backup after-hours which
// doubles as a physical backup system but also to prevent people from removing
// the drives from the backup routine.

#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <cstring>
#include <ctime>

#include <unistd.h>
#include <signal.h>
#include <libudev.h>
#include <libcron/Cron.h>

#include <core/primitives.h>
#include <core/threading.h>
#include <gui_thread.h>
#include <udev_thread.h>
#include <state.h>

// -----------------------------------------------------------------------------
// Application State Definition & Application Utilities
// -----------------------------------------------------------------------------
// TODO(Chris): As far as I know, the signal handler API doesn't allow for a way
// to pass user data over without using some global. The application state is
// fetched using a function fetcher/singleton instance such that we can access
// it when a SIGINT event is caught.

void exit_runtime(); // Forward dec.

struct application_state
{
    ThreadingManager    thread_manager;
    GUIThread*          gui_thread;
    UDEVThread*         udev_thread;

    udev*               udev_context;
};

inline static application_state*
get_state()
{
    static application_state _instance = {};
    return &_instance;
}

void
signal_handler(int s)
{
    application_state* state = get_state();
	state->gui_thread->set_runtime_state(false);
    state->gui_thread->join();
    exit_runtime();
}

void
exit_runtime()
{
    application_state* state = get_state();
	pthread_cancel(state->udev_thread->get_handle());
	udev_unref(state->udev_context);
    exit(0);
}

// -----------------------------------------------------------------------------
// Main Definition
// -----------------------------------------------------------------------------

int
main(int argc, char** argv)
{

    // -------------------------------------------------------------------------
    // Initialize the application state.
    // -------------------------------------------------------------------------
    application_state* state = get_state();

    // -------------------------------------------------------------------------
    // Capture terminal signals.
    // -------------------------------------------------------------------------
    struct sigaction sigIntHandler;

    sigIntHandler.sa_handler = signal_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, NULL);

	// -------------------------------------------------------------------------
	// Initialize the GUI thread.
	// -------------------------------------------------------------------------

	state->gui_thread = state->thread_manager.create_thread<GUIThread>();
	state->gui_thread->set_runtime_state(true);
	state->gui_thread->launch();

	// -------------------------------------------------------------------------
	// Initialize the UDEV thread.
	// -------------------------------------------------------------------------

    state->udev_context = udev_new();

	state->udev_thread = state->thread_manager.create_thread<UDEVThread>();
	state->udev_thread->set_gui_thread(state->gui_thread); // Pass the GUI thread to the UDEV thread.
	state->udev_thread->set_udev_context(state->udev_context); // Set a udev context for the thread.
	state->udev_thread->launch();

    state->gui_thread->set_udev_thread(state->udev_thread);

	// -------------------------------------------------------------------------
	// Continue running while the GUI is open.
	// -------------------------------------------------------------------------

    libcron::Cron cron;

    cron.add_schedule("Hello from Cron", "*/3 * * * * ?", [=](auto&) {
        state->gui_thread->print("Hello from libcron!");
    });

    while (state->gui_thread->get_runtime_state())
    {
        cron.tick();
        usleep(16000);
    }

	// -------------------------------------------------------------------------
    // Exit runtime.
	// -------------------------------------------------------------------------
    exit_runtime();

}
