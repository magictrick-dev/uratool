#!/bin/bash

# Ensure Configuration is Enabled
if [ ! -d "./build" ];
then
	cmake -B./build
fi

# Build
if [ -d "./build" ];
then

	# If the existing binary exists, delete it.
	if [ -d "./build/bin" ] && [ -f "./build/bin/uratool" ];
	then
		rm ./build/bin/uratool
	fi

	# Attempt to build.
	cmake --build ./build

	# If the build succeeded, then the binary was created in the
	# build/bin directory. Move that to the root directory.
	if [ -d "./build/bin" ] && [ -f "./build/bin/uratool" ];
	then
		cp ./build/bin/uratool ./
	fi

fi

