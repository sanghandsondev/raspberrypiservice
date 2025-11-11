#!/bin/bash

set -e

echo ">>> Starting the setup process for cross-compilation environment for Raspberry Pi..."

PI_IP_ADDRESS="192.168.1.50"
SYSROOT_PATH="$HOME/raspberrypi/sysroot"

# Step 1: Install Cross-Compiler Toolchain
echo ">>> [1/4] Install package aarch64-linux-gnu..."
sudo apt update
sudo apt install -y g++-aarch64-linux-gnu gcc-aarch64-linux-gnu wget python3

# Step 2: Check connection to Raspberry Pi
echo ">>> [2/4] Checking connection to Raspberry Pi at ${PI_IP_ADDRESS}..."
if ! ping -c 1 -W 2 "${PI_IP_ADDRESS}" &> /dev/null; then
    echo ">>> Error: Cannot connect to Raspberry Pi at ${PI_IP_ADDRESS}."
    echo ">>> Please check the IP address and network connection."
    exit 1
fi
echo ">>> Connection successful."

# Step 2: Create Sysroot from Raspberry Pi
echo ">>> [3/4] Copy sysroot from Pi into ${SYSROOT_PATH}..."
mkdir -p "${SYSROOT_PATH}"
echo ">>> Copying folders /lib and /usr from Pi..."
rsync -avz --rsync-path="sudo rsync" "pi@${PI_IP_ADDRESS}:/lib" "${SYSROOT_PATH}/"
rsync -avz --rsync-path="sudo rsync" "pi@${PI_IP_ADDRESS}:/usr" "${SYSROOT_PATH}/"

# Step 3: Fix symlinks in Sysroot
echo ">>> [4/4] Fix symlinks in sysroot..."
wget -O sysroot-relativelinks.py https://raw.githubusercontent.com/riscv/riscv-poky/master/scripts/sysroot-relativelinks.py
python3 sysroot-relativelinks.py "${SYSROOT_PATH}"
rm sysroot-relativelinks.py

echo ">>> Done! Environment is set up for cross-compilation."
echo ">>> Note: When cross-compiling, remember to set the CMake toolchain file accordingly."