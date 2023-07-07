#ifndef URATOOL_THREADING_H
#define URATOOL_THREADING_H
#include <pthread.h>
#include <vector>

typedef void* (*thread_func)(void*);

class Thread
{

	public:
		Thread(thread_func func_ptr);
		
		void launch();
		void join();

	private:
		thread_func 	_main;
		pthread_t 		_handle;

};

class ThreadingManager
{
	public:
		
		template <class T> inline T* create_thread();

	protected:
		std::vector<Thread*> 	_active_threads;
		
};


template <class T> inline T* ThreadingManager::
create_thread()
{
	T* instance = new T(T::Main);
	this->_active_threads.push_back(instance);
	return instance;
}

#endif
