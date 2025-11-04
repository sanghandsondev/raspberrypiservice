cmake_minimum_required(VERSION 3.10)

set(PROJECT_NAME hardwaremanager)
project(${PROJECT_NAME})

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# Add source files
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    ${ROOT_DIR}/src/*.cpp
)

# Add executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE 
    ${ROOT_DIR}/include
    ${ROOT_DIR}/include/Event
    ${ROOT_DIR}/include/Configure
    ${ROOT_DIR}/include/Thread
    ${ROOT_DIR}/include/Util
)

# Link libraries (if any)
# find_package(sdbus-c++ REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE
    pthread
)