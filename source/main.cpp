#include <iostream>
#include <pthread.h>
#include <core/primitives.h>
#include <core/application.h>

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

int
main(int argc, char** argv)
{

	Application instance;
	instance.set_gui_thread(gui_thread);
	instance.create_gui_thread();

	while (1)
	{
		
	}

    return 0;
}
