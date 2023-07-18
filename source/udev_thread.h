#ifndef URATOOL_UDEV_THREAD_H
#define URATOOL_UDEV_THREAD_H
#include <core/threading.h>
#include <device.h>

#include <libudev.h>

#include <string>
#include <vector>

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
        
        void            device_update(std::string event_message, udev_device* event_device);

        std::string     get_udev_property(udev_device* device, const char* name);

        StorageDevice*  find_device_by_uuid(std::string uuid);
        inline std::vector<StorageDevice>& get_device_list() { return this->_storage_devices; }

        inline void     lock_storage_devices() { pthread_mutex_lock(&this->_m_storage_devices); }
        inline void     unlock_storage_devices() { pthread_mutex_unlock(&this->_m_storage_devices); }

    protected:
		udev*           _udev_context;
		GUIThread*      _gui_thread;
        
        pthread_mutex_t             _m_storage_devices;
        std::vector<StorageDevice>  _storage_devices;
};


#endif
