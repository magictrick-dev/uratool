#ifndef URATOOL_APPLICATION_H
#define URATOOL_APPLICATION_H
#include <pthread.h>

struct IState
{

	pthread_mutex_t state_mutex;

	bool gui_runtime;

};

class Application
{

	public:
					Application();
		virtual    ~Application()

	protected:
		pthread_t 	_gui_thread;
		IState 		_app_state;
};

#endif
