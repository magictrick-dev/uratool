#include <core/application.h>

Application::
Application()
{

	// Prepare the internal state structure.
	this->_app_state.state_mutex = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_lock(this->_app_state.state_mutex);

	// Autostart the GUI front end.
	this->_app_state.gui_runtime = true;

	// Free the lock.
	pthread_mutex_unlock(this->_app_state.state_mutex);

}

Application::
~Application()
{

	pthread_mutex_lock(this->_app_state.state_mutex);

	// Autostart the GUI front end.
	this->_app_state.gui_runtime = false;

	// Free the lock.
	pthread_mutex_unlock(this->_app_state.state_mutex);

	

}
