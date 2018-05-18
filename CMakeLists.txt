# cmake needs this line
cmake_minimum_required(VERSION 2.8)

# Define project name
project(kcf_project)

# Find OpenCV, you may need to set OpenCV_DIR variable
# to the absolute path to the directory containing OpenCVConfig.cmake file
# via the command line or GUI
find_package(OpenCV REQUIRED)

# If the package has been found, several variables will
# be set, you can find the full list with descriptions
# in the OpenCVConfig.cmake file.
# Print some message showing some of them
message(STATUS "OpenCV library status:")
message(STATUS "    version: ${OpenCV_VERSION}")
message(STATUS "    libraries: ${OpenCV_LIBS}")
message(STATUS "    include path: ${OpenCV_INCLUDE_DIRS}")

find_package(Threads REQUIRED)  
if(THREADS_HAVE_PTHREAD_ARG)  
	set_property(TARGET kcf PROPERTY COMPILE_OPTIONS "-pthread")  
	set_property(TARGET kcf PROPERTY INTERFACE_COMPILE_OPTIONS "-pthread")  
endif() 

if(CMAKE_VERSION VERSION_LESS "2.8.11")
  # Add OpenCV headers location to your include paths
  include_directories(${OpenCV_INCLUDE_DIRS})
endif()

# Declare the executable target built from your sources
FILE(GLOB_RECURSE sourcefiles "src/*.cpp")
add_executable(kcf ${sourcefiles})

# Link your application with OpenCV libraries
if(CMAKE_THREAD_LIBS_INIT)  
	target_link_libraries(kcf ${OpenCV_LIBS} "${CMAKE_THREAD_LIBS_INIT}") 
endif() 