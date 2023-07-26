#ifndef URATOOL_APPLICATION_H
#define URATOOL_APPLICATION_H
#include <routines.h>
#include <core/threading.h>
#include <gui_thread.h>
#include <udev_thread.h>

#include <string>

// Short hand of scoping in the application struct.
#define SCOPE_APPLICATION(name) Application& name = Application::get()

/**
 * Our thread APIs must be exposed in order to interop. This will eventually need
 * to change to decouple the dependencies and leave the application to decide how
 * this will happen. For now, this will work.
 *
 * The application is responsible for mitigating control between the GUI thread,
 * UDEV monitoring thread, and the cron scheduling for the attached USB devices.
 */
class Application
{
    public:
        static      Application&   get();
        static      Application&   print(const char*);
        static      Application&   print(std::string);
        static      Application&   exit_runtime();
        static      bool           is_running() const;

        virtual    ~Application();
    protected:
        Application();

    private:
        Routines            routine_handler;       
        ThreadingManager    threading_manager;

        GUIThread*          gui_thread;
        UDEVThread*         udev_thread;

        void*               udev_context;
};

#endif

