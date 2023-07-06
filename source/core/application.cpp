#include <core/application.h>

Application::
Application()
{

	// Prepare the internal state structure.
	this->_app_state.state_mutex = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_lock(&this->_app_state.state_mutex);

	// Autostart the GUI front end.
	this->_app_state.gui_runtime = true;

	// Free the lock.
	pthread_mutex_unlock(&this->_app_state.state_mutex);

}

Application::
~Application()
{
	this->delete_gui_thread();
}

void Application::
set_gui_thread(gui_thread_handle gui_func)
{
	this->_thread_func = gui_func;
}

void Application::
create_gui_thread()
{

	if (this->_thread_func == NULL)
		return;

	//&thread1, NULL, print_message_function, (void*) message1
	int thread_return_value = pthread_create(&this->_gui_thread, NULL,
			this->_thread_func, (void*)&this->_app_state);

}

void Application::
delete_gui_thread()
{
	pthread_mutex_lock(&this->_app_state.state_mutex);
	this->_app_state.gui_runtime = false;
	pthread_mutex_unlock(&this->_app_state.state_mutex);
	pthread_join(this->_gui_thread, NULL);
}
