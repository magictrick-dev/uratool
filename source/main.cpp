#include <iostream>

#include <pthread.h>

#include <core/primitives.h>

void*
udev_watcher(void* args)
{
    std::cout << "Hello, world\n" << std::endl;
	pthread_exit(NULL);
}

int
main(int argc, char** argv)
{
    pthread_t watcher;
    i32 watcher_retv;

    watcher_retv = pthread_create(&watcher, NULL, udev_watcher, NULL);

	pthread_join(watcher, NULL);

	std::cout << "Watcher thread returns " << watcher_retv << std::endl;

    return 0;
}
