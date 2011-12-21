cmake_minimum_required( VERSION 2.8 )
project( BlobDetection )

find_package(OpenCV REQUIRED)

set( CMAKE_CXX_FLAGS_DEBUG OFF )
set( CMAKE_CXX_FLAGS_RELEASE ON )
#set( CMAKE_CXX_FLAGS "-Wall -g -pg" )

include_directories( ${PROJECT_SOURCE_DIR} )

include_directories( ${PROJECT_SOURCE_DIR}/blob_detection )
add_subdirectory( blob_detection )

add_executable( ${PROJECT_NAME} main.cpp )

target_link_libraries( ${PROJECT_NAME} blob_detection )

enable_testing()

add_test( TestRun ${PROJECT_NAME} ../frame0000.jpg )