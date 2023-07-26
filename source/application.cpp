#include <application.h>
#include <libudev.h>

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

/**
 * The const char variant will cast to a string and then invoke the GUI print method. 
 */
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
    udev_unref(self.udev_context);

    return self;

}
