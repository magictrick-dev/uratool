
# USB Replication Automation Tool

A utility for managing backups of physical USB storage devices.

## Documentation

In order to use URATool, you will need a unix environment which supports udev and
cifs (for mounting network shares). Any external vendor libraries are included in
source, so you won't need to perform any additional dependency installations to
compile. URATool is primarily meant to run as a front-end TUI application on a
low-power device such as a Raspberry Pi.

URATool currently only supports USB partitioned block devices. You may need to
format and partition new USB devices in order for URATool to pick them up as a 
useable device.

### Compiling from Source

In order to build URATool, you will need CMake to build the project. Then, you
can simply use CMake to create a build of URATool or run `build.sh`. It's that
easy. Then, you can simply run URATool as desired.
