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

    // Load our routines from disk.
    Application::load_routines("profile.json");

    // Run the main loop.
    while (Application::is_running())
    {
        Application::update_routine(); // Tick the cron jobs.
        usleep(16000);
    }

    // If we break out of the main loop, make sure to properly exit runtime.
    Application::exit_runtime();

}
