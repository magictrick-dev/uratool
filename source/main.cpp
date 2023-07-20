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

#include <core/primitives.h>
#include <core/threading.h>
#include <gui_thread.h>
#include <udev_thread.h>

// -----------------------------------------------------------------------------
// Timing Utilities
// -----------------------------------------------------------------------------

struct current_time
{
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
};

inline void
update_time(current_time* ct)
{
    time_t ttime = time(0);
    tm *local_time = localtime(&ttime);

    ct->year = 1900 + local_time->tm_year;
    ct->month = 1 + local_time->tm_mon;
    ct->day = local_time->tm_mday;
    ct->hour = 1 + local_time->tm_hour;
    ct->minute = 1 + local_time->tm_min;
    ct->second = 1 + local_time->tm_sec;
}

// Granularity type determines the interval of which certain operations take place.
// The higher the granularity, the larger the check is.
//
// Regular granularity operates in a fixed, immediate by-seconds format.
//      No next-operation rounding occurs and will happen at fixed intervals.
// Minute graularity determines what minute of the hour this takes place.
// Hour granularity determines what hour of the this takes place.
// Day granularity determines what day of the week this takes place.
// Week grandularity determines what week of the month this takes place.
// Month granularity determines what month of the year takes place.
//
// For example, a minute granularity of 28 dictates that at every hour, minute 28,
// the operation is performed. It will then set the next interval to occur one hour
// from that point, at minute 28.
//
// An hour granularity of 17 dictates that operations happen 5PM or (hour 17) every
// 24 hours. An hour granularity of 17.5 is 5:30PM, etc.
//
// A day granularity determines what day of the week the operation takes place. A
// day granularity of 3 determines that the operation takes place on Tuesday.
//
enum class GranularityType
{
    Regular,
    Minute,
    Hour,
    Day,
    Week,
    Month,
};

class Scheduler;
class Operation
{
    public:
        virtual void execute();
    private:      
};

class Scheduler
{

    public:
        Scheduler();
        inline void update_operations();
};

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

    current_time last_time_interval = {};
    update_time(&last_time_interval);

    time_t current_time = time(NULL);
    time_t next_interval = current_time + 60;

    while (state->gui_thread->get_runtime_state())
    {
        time_t now = time(NULL);

        update_time(&last_time_interval);
        if (current_time >= next_interval)
        {
            std::stringstream oss;
            oss << "Updated at: " << now << " | " << last_time_interval.hour << ":"
                << last_time_interval.minute << ":"
                << last_time_interval.second;
            state->gui_thread->print(oss.str());
            next_interval = now + 60;
        }

        update_time(&last_time_interval);
        current_time = now; 
        usleep(16000);
    }

	// -------------------------------------------------------------------------
    // Exit runtime.
	// -------------------------------------------------------------------------
    exit_runtime();

}
