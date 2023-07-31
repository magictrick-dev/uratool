#include <application.h>
#include <gui_thread.h>
#include <udev_thread.h>

#include <libudev.h>

#include <string>
#include <sstream>

// -----------------------------------------------------------------------------
// Helpers & Macros
// -----------------------------------------------------------------------------

// These macros are designed to C-cast the various threads to their proper type.
// Since the Application class is being actively passed around, we don't want to
// expose these APIs to the front-end needlessly. These threads are included in
// this translation unit and we need to cast them up to their derived types.
#define THREAD_CAST_AND_SCOPE(type, thread, name) type* name = (type*)thread
#define SCOPE_GUI_THREAD(name) THREAD_CAST_AND_SCOPE(GUIThread, \
        Application::get().gui_thread, name)
#define SCOPE_UDEV_THREAD(name) THREAD_CAST_AND_SCOPE(UDEVThread, \
        Application::get().udev_thread, name)

// -----------------------------------------------------------------------------
// Internal Functionality.
// -----------------------------------------------------------------------------

/**
 * The constructor is marked protected to prevent the user from directly constructing
 * the application class. Since the application itself is a singleton, it is loaded
 * on the first invocation of get or any accompanying static method interfaces.
 */
Application::
Application()
{
    // Initialize the GUI thread.
    GUIThread* gui_thread_ptr = this->thread_manager.create_thread<GUIThread>();
    this->gui_thread = gui_thread_ptr;
    gui_thread_ptr->set_runtime_state(true);
    gui_thread_ptr->launch();

    // Initialize the UDEV thread.
    this->udev_context = (void*)udev_new();
    UDEVThread* udev_thread_ptr = this->thread_manager.create_thread<UDEVThread>();
    this->udev_thread = udev_thread_ptr;
    udev_thread_ptr->set_udev_context((udev*)this->udev_context);
    udev_thread_ptr->launch();

}

Application::
~Application()
{
    
}

// -----------------------------------------------------------------------------
// Static Interaction Methods
// -----------------------------------------------------------------------------

Application& Application::
get()
{
    static Application _application_instance;
    return _application_instance;
}

Application& Application::
load_routines(std::string file_path)
{

    SCOPE_APPLICATION(self);
    if (!self.routine_handler.load_profile(file_path))
    {
        std::stringstream oss;
        oss << "Unable to load configuration file " << file_path << ":" << std::endl
            << "    File not found";
        self.print(oss.str());
    }

    return self;

}

Application& Application::
update_routine()
{
    SCOPE_APPLICATION(self);
    self.routine_handler.update();
    return self;
}

Routines& Application::
get_routines()
{
    SCOPE_APPLICATION(self);
    return self.routine_handler;
}

Application& Application::
create_routine(Configuration& config)
{
    SCOPE_APPLICATION(self);


    return self;
}

Application& Application::
delete_routine(std::string routine_name)
{
    SCOPE_APPLICATION(self);


    return self;
}

bool Application::
mount(std::string uuid)
{
    SCOPE_APPLICATION(self);
    SCOPE_UDEV_THREAD(udev_thread_ptr);
    bool return_status = false;

    // Lock the current device list.
    udev_thread_ptr->lock_storage_devices();

    // Find the device first and then if it is found, mount it.
    StorageDevice* device = udev_thread_ptr->find_device_by_uuid(uuid);
    if (device != NULL)
    {
        device->mount_device();
        return_status = true; 
    }
    else
    {
        std::stringstream oss;
        oss << "Unable to find device with UUID: " << uuid;
        self.print(oss.str());
    }

    // Unlock the current device list.
    udev_thread_ptr->unlock_storage_devices();

    // Return the status.
    return return_status;
}

bool Application::
unmount(std::string uuid)
{
    SCOPE_APPLICATION(self);
    SCOPE_UDEV_THREAD(udev_thread_ptr);
    bool return_status = false;

    // Lock the current device list.
    udev_thread_ptr->lock_storage_devices();

    // Find the device first and then if it is found, mount it.
    StorageDevice* device = udev_thread_ptr->find_device_by_uuid(uuid);
    if (device != NULL)
    {
        device->unmount_device();
        return_status = true; 
    }
    else
    {
        std::stringstream oss;
        oss << "Unable to find device with UUID: " << uuid;
        self.print(oss.str());
    }

    // Unlock the current device list.
    udev_thread_ptr->unlock_storage_devices();

    // Return the status.
    return return_status;
}

std::vector<std::string> Application::
get_all_routines_info()
{
    std::vector<std::string> routine_info;


    return routine_info;
}

std::vector<std::string> Application::
get_all_devices_info()
{
    SCOPE_UDEV_THREAD(udev_thread_ptr);
    SCOPE_APPLICATION(self);

    // Retrieves the information of each device.
    std::vector<std::string> device_info;

    // Loop through the devices and stream them into the oss.
    udev_thread_ptr->lock_storage_devices();
    std::vector<StorageDevice>* device_list = udev_thread_ptr->get_device_list();
    if (device_list != NULL)
    {

        // Create an OSS for displaying the output.
        std::stringstream oss;
   
        // Loop through the devices.
        for (size_t i = 0; i < device_list->size(); ++i)
        {
            // The method that we device to print may change, so this could be
            // subject to change in the future.
            StorageDevice& current_device = device_list->at(i);
            oss << "    " << i + 1 << " " << current_device.get_dev_name() << " w/ UUID: "
                << current_device.get_uuid() << " with dev-path: "
                << current_device.get_dev_path();
            device_info.push_back(oss.str());
            oss.str("");
        }
    }

    udev_thread_ptr->unlock_storage_devices();

    // Return the list. An empty list may mean there are no devices to print.
    return device_info;
}

Application& Application::
print(const char* message)
{
    SCOPE_APPLICATION(self);
    return self.print(std::move(std::string(message)));
}

Application& Application::
print(std::string message)
{
    SCOPE_APPLICATION(self);
    SCOPE_GUI_THREAD(gui_thread_ptr);
    gui_thread_ptr->print(message);
    return self;
}

bool Application::
is_running()
{
    SCOPE_GUI_THREAD(gui_thread_ptr);
    return gui_thread_ptr->get_runtime_state();
}

Application& Application::
exit_runtime()
{
    SCOPE_GUI_THREAD(gui_thread_ptr);
    SCOPE_UDEV_THREAD(udev_thread_ptr);
    SCOPE_APPLICATION(self);

    // Close the GUI thread by setting the runtime state to false.
    // The GUI thread will finish its iterative procedure and then break from its
    // main loop.
    gui_thread_ptr->set_runtime_state(false);
    gui_thread_ptr->join();

    // The UDEV thread will need to be force closed since it is a blocking runtime.
    pthread_cancel(udev_thread_ptr->get_handle());
    udev_unref((udev*)self.udev_context);

    return self;

}
