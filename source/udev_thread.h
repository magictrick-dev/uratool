#ifndef URATOOL_UDEV_THREAD_H
#define URATOOL_UDEV_THREAD_H
#include <core/threading.h>

#include <libudev.h>

#include <string>

// Forward declare GUIThread.
class GUIThread;

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
		virtual void    exit();

        void            set_udev_context(udev* context);
        void            set_gui_thread(GUIThread* gui_thread);

        // May return false if the property wasn't found.
        std::string     get_udev_property(udev_device* device, const char* name);

    protected:
		udev*           _udev_context;
		GUIThread*      _gui_thread;
};


#endif
