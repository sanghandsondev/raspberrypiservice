#!/bin/bash

# filepath: /home/sang/sangank/raspberrypiservice/setup.sh

# Dừng script nếu có lỗi
set -e

echo ">>> Bắt đầu quá trình thiết lập môi trường cross-compile cho Raspberry Pi..."

# --- Cấu hình ---
PI_IP_ADDRESS="<PI_IP_ADDRESS>" # Thay bằng IP của Pi
SYSROOT_PATH="$HOME/rpi/sysroot"

# Bước 1: Cài đặt Cross-Compiler Toolchain
echo ">>> 1/3: Cài đặt toolchain aarch64-linux-gnu..."
sudo apt update
sudo apt install -y g++-aarch64-linux-gnu gcc-aarch64-linux-gnu wget python3

# Bước 2: Chuẩn bị Sysroot từ Raspberry Pi
echo ">>> 2/3: Tạo sysroot từ Pi tại ${SYSROOT_PATH}..."
mkdir -p "${SYSROOT_PATH}"
echo "    Đang sao chép /lib và /usr từ Pi. Quá trình này có thể mất vài phút..."
rsync -avz --rsync-path="sudo rsync" "pi@${PI_IP_ADDRESS}:/{lib,usr}" "${SYSROOT_PATH}/"

# Bước 3: Sửa các liên kết tượng trưng (symlinks)
echo ">>> 3/3: Sửa các symlink trong sysroot..."
wget -O sysroot-relativelinks.py https://raw.githubusercontent.com/riscv/riscv-poky/master/scripts/sysroot-relativelinks.py
python3 sysroot-relativelinks.py "${SYSROOT_PATH}"
rm sysroot-relativelinks.py

echo ">>> Hoàn tất! Môi trường đã sẵn sàng để biên dịch chéo."
echo "    Hãy chắc chắn rằng tệp rpi4_toolchain.cmake trỏ đến ${SYSROOT_PATH}."