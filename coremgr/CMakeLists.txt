cmake_minimum_required(VERSION 3.10)

set(PROJECT_NAME coremanager)
project(${PROJECT_NAME})

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages and libraries
find_package(PkgConfig REQUIRED)
pkg_check_modules(DBUS-1 REQUIRED dbus-1)
pkg_check_modules(SYSTEMD REQUIRED libsystemd)

find_package(Boost REQUIRED COMPONENTS system filesystem)

set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# Add source files
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    ${ROOT_DIR}/src/*.cpp
)

# Add executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Set compiler flags
target_compile_options(${PROJECT_NAME} PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Werror
)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE 
    ${ROOT_DIR}/include
    ${ROOT_DIR}/include/Event
    ${ROOT_DIR}/include/Configure
    ${ROOT_DIR}/include/Thread
    ${ROOT_DIR}/include/DBus
    ${ROOT_DIR}/include/Log
    ${ROOT_DIR}/include/Util
    ${ROOT_DIR}/../common/include
    ${DBUS-1_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
    ${SYSTEMD_INCLUDE_DIRS}
)

# Link libraries (if any)
# find_package(sdbus-c++ REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE
    pthread
    ${DBUS-1_LIBRARIES}
    ${Boost_LIBRARIES}
    ${SYSTEMD_LIBRARIES}
)

# Install target: cd build && sudo make install
install(TARGETS ${PROJECT_NAME} DESTINATION /usr/local/bin)