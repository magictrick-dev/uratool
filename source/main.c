#include <stdlib.h>
#include <stdio.h>
#include <core/helpers.h>
#include <core/primitives.h>

#include <libudev.h>

int
main(/*int argc, char** argv*/)
{

	// Initiate libudev.
	udev* udev_context = udev_new();

	// Get the enumerables.
	udev_enumerate* enumerables = udev_enumerate_new(udev_context);

	return 0;
}

