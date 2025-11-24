cmake_minimum_required(VERSION 3.10)

set(PROJECT_NAME recordmanager)
project(${PROJECT_NAME})

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# Add common library
add_subdirectory(${ROOT_DIR}/../common ${CMAKE_CURRENT_BINARY_DIR}/common)

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
    # -Werror
)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE 
    ${ROOT_DIR}/include
    ${ROOT_DIR}/include/Configure
    ${ROOT_DIR}/include/Thread
    ${ROOT_DIR}/include/DBus
    ${ROOT_DIR}/include/Log
    ${ROOT_DIR}/include/Util
)

# Link libraries (if any)
target_link_libraries(${PROJECT_NAME} PRIVATE
    common
    asound
)

# Install target: cd build && sudo make install
# install(TARGETS ${PROJECT_NAME} DESTINATION /usr/local/bin)