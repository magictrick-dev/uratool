#include <application.h>
#include <libudev.h>

#include <string>
#include <sstream>

/**
 * The constructor is marked protected to prevent the user from directly constructing
 * the application class. Since the application itself is a singleton, it is loaded
 * on the first invocation of get or any accompanying static method interfaces.
 */
Application::
Application()
{

    // Initialize the GUI thread.
    this->gui_thread = this->thread_manager.create_thread<GUIThread>();
    this->gui_thread->set_runtime_state(true);
    this->gui_thread->launch();

    // Initialize the UDEV thread.
    this->udev_context = (void*)udev_new();
    this->udev_thread = this->thread_manager.create_thread<UDEVThread>();
    this->udev_thread->set_udev_context((udev*)this->udev_context);
    this->udev_thread->launch();

}

Application::
~Application()
{
    
}

// -----------------------------------------------------------------------------
// Static Interaction Methods
// -----------------------------------------------------------------------------

/**
 * A lazy singletin fetcher fro the Application class. This will load and init
 * the class when it is first invoked.
 */
Application& Application::
get()
{
    static Application _application_instance;
    return _application_instance;
}

bool Application::
mount(std::string uuid)
{
    SCOPE_APPLICATION(self);
    bool return_status = false;

    // Lock the current device list.
    self.udev_thread->lock_storage_devices();

    // Find the device first and then if it is found, mount it.
    StorageDevice* device = self.udev_thread->find_device_by_uuid(uuid);
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
    self.udev_thread->unlock_storage_devices();

    // Return the status.
    return return_status;
}

bool Application::
unmount(std::string uuid)
{
    SCOPE_APPLICATION(self);
    bool return_status = false;

    // Lock the current device list.
    self.udev_thread->lock_storage_devices();

    // Find the device first and then if it is found, mount it.
    StorageDevice* device = self.udev_thread->find_device_by_uuid(uuid);
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
    self.udev_thread->unlock_storage_devices();

    // Return the status.
    return return_status;
}

std::vector<std::string> Application::
get_all_devices_info()
{

    SCOPE_APPLICATION(self);

    // Retrieves the information of each device.
    std::vector<std::string> device_info;

    // Loop through the devices and stream them into the oss.
    self.udev_thread->lock_storage_devices();
    std::vector<StorageDevice>* device_list = self.udev_thread->get_device_list();
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

    self.udev_thread->unlock_storage_devices();

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
    self.gui_thread->print(message);
    return self;
}

bool Application::
is_running()
{
    SCOPE_APPLICATION(self);
    return self.gui_thread->get_runtime_state();
}

Application& Application::
exit_runtime()
{
    SCOPE_APPLICATION(self);

    // Close the GUI thread by setting the runtime state to false.
    // The GUI thread will finish its iterative procedure and then break from its
    // main loop.
    self.gui_thread->set_runtime_state(false);
    self.gui_thread->join();

    // The UDEV thread will need to be force closed since it is a blocking runtime.
    pthread_cancel(self.udev_thread->get_handle());
    udev_unref((udev*)self.udev_context);

    return self;

}
