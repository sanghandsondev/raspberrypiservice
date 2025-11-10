# sudo apt update
# sudo apt install -y g++-aarch64-linux-gnu gcc-aarch64-linux-gnu

# mkdir -p ~/raspberrypi/sysroot

# rsync -avz --rsync-path="sudo rsync" pi@PI_IP_ADDRESS:/{lib,usr,etc} ~/raspberrypi/sysroot/

# wget https://raw.githubusercontent.com/riscv/riscv-poky/master/scripts/sysroot-relativelinks.py
# python3 sysroot-relativelinks.py ~/raspberrypi/sysroot

set (CMAKE_SYSTEM_NAME Linux)
set (CMAKE_SYSTEM_PROCESSOR aarch64)

set (CMAKE_SYSROOT $ENV{HOME}/raspberrypi/sysroot)

set (CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
set (CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)

set (CMAKE_FIND_ROOT_PATH ${CMAKE_SYSROOT})
set (CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set (CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set (CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set (CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)