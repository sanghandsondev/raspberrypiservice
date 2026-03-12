#!/bin/bash

set -e

echo ">>> Starting the setup process for cross-compilation environment for Raspberry Pi..."

PI_USER="pi"
PI_IP_ADDRESS="192.168.1.50"
SYSROOT_PATH="$HOME/raspberrypi/sysroot"

# Step 1: Install Cross-Compiler Toolchain & Build Dependencies (Host PC)
echo ">>> [1/5] Installing cross-compilation toolchain and build dependencies on host PC..."
sudo apt update
sudo apt install -y \
    g++-aarch64-linux-gnu \
    gcc-aarch64-linux-gnu \
    cmake \
    pkg-config \
    wget \
    python3 \
    rsync \
    ssh

# Step 2: Install required packages on Raspberry Pi (so sysroot has all headers/libs)
echo ">>> [2/5] Checking connection to Raspberry Pi at ${PI_IP_ADDRESS}..."
if ! ping -c 1 -W 2 "${PI_IP_ADDRESS}" &> /dev/null; then
    echo ">>> Error: Cannot connect to Raspberry Pi at ${PI_IP_ADDRESS}."
    echo ">>> Please check the IP address and network connection."
    exit 1
fi
echo ">>> Connection successful."

echo ">>> Installing required development packages on Pi (for sysroot)..."
ssh "${PI_USER}@${PI_IP_ADDRESS}" "
    set -e
    sudo apt update
    sudo apt install -y \
        libdbus-1-dev \
        libsystemd-dev \
        libboost-system-dev \
        libboost-filesystem-dev \
        libsqlite3-dev \
        libasound2-dev
    echo '>>> Pi development packages installed.'
"

# Step 3: Create Sysroot from Raspberry Pi
echo ">>> [3/5] Copying sysroot from Pi into ${SYSROOT_PATH}..."
mkdir -p "${SYSROOT_PATH}"
echo ">>> Copying folders /lib and /usr from Pi..."
rsync -avz --rsync-path="sudo rsync" "${PI_USER}@${PI_IP_ADDRESS}:/lib" "${SYSROOT_PATH}/"
rsync -avz --rsync-path="sudo rsync" "${PI_USER}@${PI_IP_ADDRESS}:/usr" "${SYSROOT_PATH}/"

# Step 4: Fix symlinks in Sysroot
echo ">>> [4/5] Fixing symlinks in sysroot..."
wget -q -O sysroot-relativelinks.py https://raw.githubusercontent.com/riscv/riscv-poky/master/scripts/sysroot-relativelinks.py
python3 sysroot-relativelinks.py "${SYSROOT_PATH}"
rm -f sysroot-relativelinks.py

# Step 5: Install runtime packages on Pi
echo ">>> [5/5] Installing runtime packages on Pi..."
ssh "${PI_USER}@${PI_IP_ADDRESS}" "
    set -e
    sudo apt install -y \
        dbus \
        libdbus-1-3 \
        libsystemd0 \
        libboost-system-dev \
        libboost-filesystem-dev \
        libsqlite3-0 \
        libasound2 \
        bluez \
        bluez-obex \
        ofono \
        dbus-user-session
    echo '>>> Pi runtime packages installed.'
"

echo ""
echo "============================================="
echo ">>> Done! Cross-compilation environment ready."
echo ">>> Sysroot: ${SYSROOT_PATH}"
echo ">>> Usage:"
echo ">>>   make deploy              # Build all & deploy to Pi"
echo ">>>   make deploy module=core  # Build & deploy CoreManager only"
echo "============================================="