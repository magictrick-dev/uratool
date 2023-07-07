#include <iostream>
#include <pthread.h>
#include <core/primitives.h>
#include <core/application.h>

/**
void
set_runtime_state(IState* a_state, bool new_state)
{
	pthread_mutex_lock(&a_state->state_mutex);
	a_state->gui_runtime = new_state;
	pthread_mutex_unlock(&a_state->state_mutex);
}

bool
get_runtime_state(IState* a_state)
{
	bool runtime_state;
	pthread_mutex_lock(&a_state->state_mutex);
	runtime_state = a_state->gui_runtime;
	pthread_mutex_unlock(&a_state->state_mutex);
	return runtime_state;
}

void*
gui_thread(void* args)
{
	
	IState* a_state = (IState*)args;

	bool runtime_loop = get_runtime_state(a_state);
	while (runtime_loop)
	{
		printf("Entered runtime.\n");
		printf("Trying to exit runtime.\n");
		set_runtime_state(a_state, false);
		runtime_loop = get_runtime_state(a_state);
	}

	printf("Exitting GUI thread.\n");
	pthread_exit(NULL);
}
*/

#include <vector>

typedef void* (*thread_func)(void*);

class Thread
{
	public:
		Thread(thread_func func) : _main(func) {}

		inline void launch()
		{
			pthread_create(&this->_handle, NULL, this->_main, (void*)this);
		}

		inline void join()
		{
			pthread_join(this->_handle, NULL);
		}

	private:
		thread_func 	_main;
		pthread_t 		_handle;
};

class TestThread : public Thread
{

	public:
		TestThread(thread_func func) : Thread(func)
		{
		}

		static void* Main(void* args)
		{
			std::cout << "TestThread static Main()" << std::endl;
			TestThread* self = (TestThread*)args;
			self->main();
			pthread_exit(NULL);
		}

		void main()
		{

			std::cout << "TestThread main()" << std::endl;
			this->number_c = number_a + number_b;
			std::cout << "The magic number is " << number_c << " " << std::endl;

		}

	private:
		int number_a = 69;
		int number_b = 420;
		int number_c = 0;
	
};

class ThreadingManager
{
	public:

		template <class T>
		inline T* create_thread()
		{
			T* instance = new T(T::Main);
			this->_active_thread = instance;
			return instance;
		}
	protected:
		Thread* 	_active_thread;
};

int
main(int argc, char** argv)
{

	ThreadingManager t_man;
	TestThread* t_thread = t_man.create_thread<TestThread>();
	t_thread->launch();

	Application application;
	if (!application.init()) return 1;
	while (application.runtime());
	t_thread->join();
    return application.shutdown();
}
