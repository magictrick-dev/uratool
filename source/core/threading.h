#ifndef URATOOL_THREADING_H
#define URATOOL_THREADING_H
#include <pthread.h>
#include <vector>

typedef void* (*thread_func)(void*);
#define THREAD_MAIN(thread_type) static void* Main(void* arg) \
	{ ((thread_type*)arg)->main(); pthread_exit(NULL); } \
	virtual void main()

// Forward Decs.
class ThreadingManager; 
class Thread;

class Thread
{

	public:
		Thread();
		virtual ~Thread();
		
		void launch();
		void join();

		virtual void main() = 0;
		virtual void exit() = 0;

		pthread_t 		get_handle() const;

	private:
		thread_func 	_main;
		pthread_t 		_handle;

		friend class ThreadingManager;
};

class ThreadingManager
{
	public:
		ThreadingManager();
		virtual ~ThreadingManager();

		template <class T> inline T* create_thread();
	protected:
		std::vector<Thread*> 	_active_threads;
		
};

template <class T> inline T* ThreadingManager::
create_thread()
{
	T* instance = new T();
	instance->_main = T::Main;
	this->_active_threads.push_back(instance);
	return instance;
}

#endif
