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
#include <iomanip>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cstring>
#include <ctime>

#include <unistd.h>
#include <signal.h>
#include <libudev.h>
#include <libcron/Cron.h>
#include <vendor/jsoncpp/json.hpp>

#include <core/primitives.h>
#include <resourceconfiguration.h>
#include <state.h>

// -----------------------------------------------------------------------------
// Application State Definition & Application Utilities
// -----------------------------------------------------------------------------
// TODO(Chris): As far as I know, the signal handler API doesn't allow for a way
// to pass user data over without using some global. The application state is
// fetched using a function fetcher/singleton instance such that we can access
// it when a SIGINT event is caught.

void exit_runtime(); // Forward dec.

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
	udev_unref((udev*)state->udev_context);
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

    state->udev_context = (void*)udev_new();

	state->udev_thread = state->thread_manager.create_thread<UDEVThread>();
	state->udev_thread->set_udev_context((udev*)state->udev_context); // Set a udev context for the thread.
	state->udev_thread->launch();

	// -------------------------------------------------------------------------
	// Continue running while the GUI is open.
	// -------------------------------------------------------------------------

#if 0
    libcron::Cron cron;

    static time_t now;
    now = time(0);
    bool added_schedule = cron.add_schedule("Hello from Cron", "*/3 * * * * ?", [=](auto&) {
        time_t last = time(0);
        time_t diff = last - now;
        now = last;
        std::stringstream oss;
        oss << "Hell from libcron!    ";
        oss << "Time difference: " << diff;
        state->gui_thread->print(oss.str());
    });
#endif

    RoutineConfiguration configuration;
    if (!configuration.load_profile("profile.json"))
    {
        std::cout << "Unable to load profile." << std::endl;
    }

    while (state->gui_thread->get_runtime_state())
    {
        //cron.tick();
        usleep(16000);
    }

	// -------------------------------------------------------------------------
    // Exit runtime.
	// -------------------------------------------------------------------------
    exit_runtime();

}
