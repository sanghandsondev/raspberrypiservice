cmake_minimum_required(VERSION 3.10)

project(CommonLib)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(DBUS-1 REQUIRED dbus-1)
pkg_check_modules(SYSTEMD REQUIRED libsystemd)

set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB_RECURSE COMMON_SOURCES CONFIGURE_DEPENDS
    ${ROOT_DIR}/src/*.cpp
)

# Create static library
add_library(common STATIC ${COMMON_SOURCES}) 

# Include directories for users of the library
target_include_directories(common PUBLIC 
    ${ROOT_DIR}/include
    ${ROOT_DIR}/include/Configure
    ${ROOT_DIR}/include/DBus
    ${ROOT_DIR}/include/Event
    ${ROOT_DIR}/include/Log
    ${ROOT_DIR}/include/Thread
    ${DBUS-1_INCLUDE_DIRS}
    ${SYSTEMD_INCLUDE_DIRS}
)

# Link libraries to the common library (dynamically linked)
target_link_libraries(common PUBLIC
    pthread
    ${DBUS-1_LIBRARIES}
    ${SYSTEMD_LIBRARIES}
)
