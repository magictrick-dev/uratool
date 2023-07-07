#include <core/threading.h>

Thread::
Thread(thread_func func_ptr) : _main(func_ptr)
{ };

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
