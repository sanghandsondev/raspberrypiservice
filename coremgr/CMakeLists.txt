cmake_minimum_required(VERSION 3.10)

set(PROJECT_NAME coremanager)
project(${PROJECT_NAME})

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages and libraries
find_package(PkgConfig REQUIRED)
pkg_check_modules(DBUS-1 REQUIRED dbus-1)

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
    ${DBUS-1_INCLUDE_DIRS}
    ${ROOT_DIR}/../common/include
)

# Link libraries (if any)
# find_package(sdbus-c++ REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE
    pthread
    ${DBUS-1_LIBRARIES}
)