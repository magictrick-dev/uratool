#ifndef URATOOL_STATE_H
#define URATOOL_STATE_H
#include <core/threading.h>

#include <gui_thread.h>
#include <udev_thread.h>

struct application_state
{
    ThreadingManager    thread_manager;
    GUIThread*          gui_thread;
    UDEVThread*         udev_thread;

    void*               udev_context;
};

inline application_state*
get_state()
{
    static application_state _state = {};
    return &_state;
}

#endif
