cmake_minimum_required( VERSION 2.8 )
project( BlobDetection )

find_package(OpenCV REQUIRED)

include_directories( ${PROJECT_SOURCE_DIR} )

set( CMAKE_CXX_FLAGS_DEBUG ON )
set( CMAKE_CXX_FLAGS_RELEASE OFF )

add_executable( ${PROJECT_NAME} main.cpp )

target_link_libraries( ${PROJECT_NAME} ${OpenCV_LIBS} -lcvblob -lrand_gen -lcircdet -lgsl -lgslcblas -ltbb )

enable_testing()

add_test( TestRun ${PROJECT_NAME} ../frame0000.jpg )