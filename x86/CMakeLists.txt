cmake_minimum_required(VERSION 2.8)
project( lane )

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11" )
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")

aux_source_directory(./src/ SRC_LIST)
add_executable(${PROJECT_NAME} ${SRC_LIST})

include_directories( include/common )
include_directories( include/)
include_directories( 3rdparty/tim-vx/include/VX )
include_directories( 3rdparty/tim-vx/include/CL )
find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

target_link_libraries( lane ${OpenCV_LIBS} )
target_link_libraries( lane /home/soc123/tengine-ncnn-tracking-kalman-x86/lib/libtengine-lite.so)

