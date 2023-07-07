#include <core/threading.h>

Thread::
Thread()
{ }

Thread::
~Thread()
{ }

void Thread::
launch()
{
	pthread_create(&this->_handle, NULL, this->_main, (void*)this);
}

void Thread::
join()
{
	pthread_join(this->_handle, NULL);
}

ThreadingManager::
ThreadingManager()
{ }


ThreadingManager::
~ThreadingManager()
{

	while (!this->_active_threads.empty())
	{
		Thread* current_thread = this->_active_threads.back();
		current_thread->exit();
		delete current_thread;
		this->_active_threads.pop_back();
	}

}
