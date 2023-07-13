#ifndef URATOOL_UDEV_THREAD_H
#define URATOOL_UDEV_THREAD_H
#include <core/threading.h>
#include <libudev.h>

// Forward declare GUIThread.
class GUIThread;

#if 0
/**
 * Storage devices represent the various USB devices that are currently available
 * on the system. Devices themselves are then designed to be mounted and interacted
 * with by the system. For now this is a filler class with zero functionality.
 */
class StorageDevice
{

    public:
        StorageDevice(std::string identifier);

    protected:
        std::string     _uuid;
        std::string     _mount_point;
        std::string     _device_name;

};
#endif

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
        udev*           get_udev_context();

        void            set_gui_thread(GUIThread* gui_thread);

    protected:
		udev*           _udev_context;
		GUIThread*      _gui_thread;
};


#endif
