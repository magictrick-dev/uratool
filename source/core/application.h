#ifndef URATOOL_APPLICATION_H
#define URATOOL_APPLICATION_H
#include <pthread.h>

typedef void* (*gui_thread_handle)(void*);

struct IState
{

	pthread_mutex_t state_mutex;
	bool gui_runtime;

};

class Application
{

	public:
					Application();
		virtual    ~Application();

		void 		set_gui_thread(gui_thread_handle gui_func);
		void 		create_gui_thread();
		void 		delete_gui_thread();

	protected:
		gui_thread_handle 	_thread_func;
		pthread_t 			_gui_thread;
		IState 				_app_state;
};

#endif
