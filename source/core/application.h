#ifndef URATOOL_APPLICATION_H
#define URATOOL_APPLICATION_H
#include <pthread.h>

#define CREATE_ISH_SCOPELOCK(ish_ptr) MutexScopeLock \
	__ish_scope_lock(ish_ptr->retrieve_full_lock())

class MutexScopeLock
{
	public:
		 MutexScopeLock(pthread_mutex_t* mutex);
		~MutexScopeLock();

	private:
		pthread_mutex_t* 	_mutex;
};

class InternalStateHandler
{

	public:
		 InternalStateHandler();
		~InternalStateHandler();

		void request_full_lock();
		void release_full_lock();

		pthread_mutex_t* retrieve_full_lock();

	private:
		pthread_mutex_t 	_full_lock;
		bool 				_gui_runtime;

};

class Application
{
	public:
					Application();
		virtual    ~Application();

		bool 		init();
		bool 		runtime();
		int 		shutdown();

	protected:
		InternalStateHandler _ish;

};

#endif
