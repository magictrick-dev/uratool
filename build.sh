#!/bin/bash

# Ensure Configuration is Enabled
if [ ! -d "./comp" ];
then
	cmake -B./comp
fi

# Build
if [ -d "./comp" ];
then

	# If the existing binary exists, delete it.
	if [ -d "./comp/bin" ] && [ -f "./comp/bin/uratool" ];
	then
		rm ./comp/bin/uratool
	fi

	# Attempt to build.
	cmake --build ./comp

	# If the build succeeded, then the binary was created in the
	# build/bin directory. Move that to the root directory.
	if [ -d "./comp/bin" ] && [ -f "./comp/bin/uratool" ];
	then
		cp ./comp/bin/uratool ./
	fi

fi

