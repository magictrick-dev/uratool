# Initialize the CMake version
cmake_minimum_required(VERSION 3.21)

# Set the project name. Change this to the appropriate name as desired.
project(uratool)

set(CMAKE_C_STANDARD 11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(${PROJECT_NAME} 

"./source/main.cpp"
"./source/gui_thread.cpp"
"./source/device.cpp"
"./source/udev_thread.cpp"
"./source/core/threading.cpp"

)

# Allow absolute referencing for project files located in ./src
target_include_directories(${PROJECT_NAME} PUBLIC ./source)

# Include OpenGL libraries.
target_link_libraries(${PROJECT_NAME} udev blkid ncurses pthread)
