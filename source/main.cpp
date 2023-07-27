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
#include <routines.h>
#include <state.h>
#include <application.h>

// -----------------------------------------------------------------------------
// Application State Definition & Application Utilities
// -----------------------------------------------------------------------------
// TODO(Chris): As far as I know, the signal handler API doesn't allow for a way
// to pass user data over without using some global. The application state is
// fetched using a function fetcher/singleton instance such that we can access
// it when a SIGINT event is caught.

void
signal_handler(int s)
{
    Application::exit_runtime();
}

// -----------------------------------------------------------------------------
// Main Definition
// -----------------------------------------------------------------------------

int
main(int argc, char** argv)
{

    // -------------------------------------------------------------------------
    // Fetch our application. This will cause it to initialize.
    // -------------------------------------------------------------------------
    Application& application_instance = Application::get();

    // -------------------------------------------------------------------------
    // Capture terminal signals.
    // -------------------------------------------------------------------------
    struct sigaction sigIntHandler;

    sigIntHandler.sa_handler = signal_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, NULL);

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

    Routines configuration;
    if (!configuration.load_profile("profile.json"))
    {
        std::cout << "Unable to load profile." << std::endl;
    }

    while (Application::is_running())
    {
        //cron.tick();
        usleep(16000);
    }

    // If we break out of the main loop, make sure to properly exit runtime.
    Application::exit_runtime();

}
