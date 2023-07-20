# Initialize the CMake version
cmake_minimum_required(VERSION 3.21)

# Set the project name. Change this to the appropriate name as desired.
project(uratool)

set(CMAKE_C_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(${PROJECT_NAME} 

"./source/main.cpp"
"./source/gui_thread.cpp"
"./source/device.cpp"
"./source/udev_thread.cpp"
"./source/core/threading.cpp"

)

# External libraries.
add_subdirectory("source/vendor/date")
add_subdirectory("source/vendor/libcron")

# Allow absolute referencing for project files located in ./src
target_include_directories(${PROJECT_NAME} PUBLIC ./source)

# Include OpenGL libraries.
target_link_libraries(${PROJECT_NAME} udev blkid ncurses pthread libcron date)

get_target_property(LIBCRON_INC_PATH libcron INTERFACE_INCLUDE_DIRECTORIES)
get_target_property(DATE_INC_PATH date INTERFACE_INCLUDE_DIRECTORIES)
include_directories(${LIBCRON_INC_PATH} ${DATE_INC_PATH})

