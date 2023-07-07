#include <core/application.h>
#include <core/threading.h>
#include <string>
#include <iostream>

class GUIThread : public Thread
{

	public:
		GUIThread();
		~GUIThread();

		THREAD_MAIN(GUIThread);
		virtual void exit();
};

GUIThread::
GUIThread()
{
	std::cout << "GUI Thread construct." << std::endl;
}

GUIThread::
~GUIThread()
{
	std::cout << "GUI Thread destruct." << std::endl;
}

void GUIThread::
main()
{
	std::cout << "GUI Thread main." << std::endl;
}

void GUIThread::
exit()
{

}


// -----------------------------------------------------------------------------
// Mutex Scope Lock
// -----------------------------------------------------------------------------
// A mutex scope lock is a small facility that performs locking/unlocking of
// provided mutex for the duration that the object is in scope.

MutexScopeLock::
MutexScopeLock(pthread_mutex_t* mutex)
	: _mutex(mutex)
{
	pthread_mutex_lock(this->_mutex);
}

MutexScopeLock::
~MutexScopeLock()
{
	pthread_mutex_unlock(this->_mutex);
}

// -----------------------------------------------------------------------------
// Internal State Handler Definition
// -----------------------------------------------------------------------------
// The internal state handler is the interface which acts between the main
// and GUI thread. Since the data is shared between two threads, the internal
// state handler contains mutexs for safe handling of thread-specific data manip.
//

InternalStateHandler::
InternalStateHandler()
	: _full_lock(PTHREAD_MUTEX_INITIALIZER)
{ }

InternalStateHandler::
~InternalStateHandler()
{ }

void InternalStateHandler::
request_full_lock()
{
	pthread_mutex_lock(&this->_full_lock);
}

void InternalStateHandler::
release_full_lock()
{
	pthread_mutex_unlock(&this->_full_lock);
}

pthread_mutex_t* InternalStateHandler::
retrieve_full_lock()
{
	return &this->_full_lock;
}

// -----------------------------------------------------------------------------
// Application Definition
// -----------------------------------------------------------------------------
// The application drives the internal application and its internal functionality.

Application::
Application()
{ }

Application::
~Application()
{ }

bool Application::
init()
{

	ThreadingManager t_man;
	GUIThread* gui_thread = t_man.create_thread<GUIThread>();
	gui_thread->launch();
	gui_thread->join();

	std::cout << "Performing initialization routine..." << std::endl;
	return true;
}

bool Application::
runtime()
{
	std::cout << "Runtime, exit." << std::endl;
	return false;
}

int Application::
shutdown()
{
	std::cout << "Shutdown procedure." << std::endl;
	return 0;
}
