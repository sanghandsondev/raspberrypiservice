# C++ Service for Raspberry Pi 4

> 3 Linux daemon services giao tiếp qua D-Bus IPC, cross-compiled từ Ubuntu host sang Raspberry Pi 4 (aarch64).

---

## 📑 Mục Lục

1. [Tổng Quan Kiến Trúc](#1-tổng-quan-kiến-trúc)
2. [Yêu Cầu Hệ Thống](#2-yêu-cầu-hệ-thống)
3. [Cài Đặt Lần Đầu (First-Time Setup)](#3-cài-đặt-lần-đầu-first-time-setup)
4. [Build & Deploy](#4-build--deploy)
5. [Cấu Trúc Thư Mục](#5-cấu-trúc-thư-mục)
6. [Chi Tiết Từng Service](#6-chi-tiết-từng-service)
7. [D-Bus IPC Communication](#7-d-bus-ipc-communication)
8. [Kiểm Tra Trên Pi](#8-kiểm-tra-trên-pi)
9. [Troubleshooting](#9-troubleshooting)

---

## 1. Tổng Quan Kiến Trúc

```
┌─────────────────────┐    ┌────────────────────────┐    ┌─────────────────────┐
│   HardwareManager   │    │     CoreManager         │    │   RecordManager     │
│                     │    │                          │    │                     │
│ • Bluetooth (BlueZ) │    │ • WebSocket Server      │    │ • ALSA Audio Record │
│ • HFP Calls (oFono) │    │ • SQLite Database       │    │ • WAV File Output   │
│ • PBAP Phonebook    │    │ • Business Logic        │    │ • Audio Filtering   │
│ • Temp Sensor (GPIO)│    │ • Timer                 │    │                     │
└────────┬────────────┘    └─────────┬────────────────┘    └────────┬────────────┘
         │                           │                               │
         └───── D-Bus System Bus ────┴───────────────────────────────┘
                    (IPC: Signal + Method Call)
```

**Tech Stack:**
- **Language:** C++17
- **Build:** CMake + aarch64-linux-gnu cross-compiler
- **IPC:** D-Bus (libdbus-1)
- **Target:** Raspberry Pi 4 — Raspberry Pi OS Lite 64-bit (Debian Bookworm)
- **Host:** Ubuntu 22.04+ (x86_64)

---

## 2. Yêu Cầu Hệ Thống

### Host PC (Ubuntu)
- Ubuntu 22.04 trở lên (x86_64)
- Kết nối mạng đến Raspberry Pi (SSH)

### Raspberry Pi 4
- **OS:** Raspberry Pi OS Lite 64-bit (khuyến nghị bản 2024-04 trở lên)
- **User:** `pi` (default)
- **IP:** `192.168.1.50` (có thể đổi trong `setenv.sh` và các `deploy.sh`)
- **SSH:** Đã bật SSH service
- Kết nối mạng cùng LAN với Host PC

---

## 3. Cài Đặt Lần Đầu (First-Time Setup)

### Bước 1: Chuẩn bị Raspberry Pi

Flash Raspberry Pi OS Lite 64-bit lên SD card bằng **Raspberry Pi Imager**:
- Chọn **Raspberry Pi OS Lite (64-bit)**
- Trong Settings: bật SSH, đặt username `pi`, password, WiFi/Ethernet
- Boot Pi, đảm bảo SSH hoạt động:
  ```bash
  ssh pi@192.168.1.50
  ```

### Bước 2: Clone project

```bash
git clone https://github.com/sanghandsondev/raspberrypiservice.git
cd raspberrypiservice
```

### Bước 3: Chạy setup cross-compilation

> ⚠️ Chỉ cần chạy **1 lần duy nhất** (hoặc khi đổi Pi / cài thêm thư viện).

```bash
make setenv
```

Script `setenv.sh` sẽ tự động thực hiện 5 bước:

| Bước | Nội dung | Chạy trên |
|------|----------|-----------|
| 1/5 | Cài cross-compiler toolchain (`aarch64-linux-gnu-g++`, `cmake`, `rsync`, ...) | Host PC |
| 2/5 | SSH vào Pi cài development packages (`libdbus-1-dev`, `libsystemd-dev`, `libboost-*-dev`, `libsqlite3-dev`, `libasound2-dev`) | Pi |
| 3/5 | Rsync sysroot từ Pi (`/lib`, `/usr`) về `~/raspberrypi/sysroot/` | Host PC |
| 4/5 | Fix symlinks trong sysroot | Host PC |
| 5/5 | Cài runtime packages trên Pi (`bluez`, `bluez-obex`, `ofono`, `dbus-user-session`, ...) | Pi |

> **Lưu ý IP:** Nếu Pi không phải `192.168.1.50`, sửa biến `PI_IP_ADDRESS` trong `setenv.sh` và `PI_HOST` trong các file `deploy.sh` trước khi chạy.

---

## 4. Build & Deploy

### Deploy tất cả 3 services

```bash
make deploy
```

### Deploy 1 service cụ thể

```bash
make deploy module=core       # CoreManager only
make deploy module=hardware   # HardwareManager only
make deploy module=record     # RecordManager only
```

### Quy trình `make deploy` tự động thực hiện

Mỗi module sẽ:

```
cmake (cross-compile) → make → scp binary + .service + .conf → Pi
    → apt install packages (nếu thiếu)
    → install D-Bus config → /etc/dbus-1/system.d/
    → reload D-Bus
    → daemon-reload → enable + restart service
    → hiển thị service status
```

### Build local (không deploy)

Build trên host PC để kiểm tra compile (không cross-compile):

```bash
make install                  # Build all
make install module=core      # Build CoreManager only
```

### Clean

```bash
make clean                    # Clean all
make clean module=hardware    # Clean HardwareManager only
```

---

## 5. Cấu Trúc Thư Mục

```
raspberrypiservice/
├── Makefile                    # Root Makefile — điều phối build/deploy
├── setenv.sh                   # Setup cross-compilation (chạy 1 lần)
├── rpi4_toolchain.cmake        # CMake toolchain file cho aarch64
├── README.md                   # File này
│
├── common/                     # Thư viện dùng chung (libcommon.a)
│   ├── CMakeLists.txt
│   ├── include/
│   │   ├── Configure/IConfig.hpp
│   │   ├── DBus/               # D-Bus client, sender, receiver base
│   │   ├── Event/              # EventQueue, Event, Payload
│   │   ├── Log/Logger.hpp
│   │   └── Thread/ThreadBase.hpp
│   └── src/
│
├── coremgr/                    # CoreManager service
│   ├── CMakeLists.txt
│   ├── Makefile
│   ├── deploy.sh
│   ├── coremanager.service     # systemd unit file
│   ├── com.example.coremanager.conf  # D-Bus security policy
│   ├── include/
│   └── src/main.cpp
│
├── hardwaremgr/                # HardwareManager service
│   ├── CMakeLists.txt
│   ├── Makefile
│   ├── deploy.sh
│   ├── hardwaremanager.service
│   ├── com.example.hardwaremanager.conf
│   ├── include/
│   │   └── BT/                 # BluezDBus, OfonoDBus, ObexPbapClient, BluetoothAgent
│   └── src/main.cpp
│
└── recordmgr/                  # RecordManager service
    ├── CMakeLists.txt
    ├── Makefile
    ├── deploy.sh
    ├── recordmanager.service
    ├── com.example.recordmanager.conf
    ├── include/
    └── src/main.cpp
```

---

## 6. Chi Tiết Từng Service

### CoreManager (`coremgr`)

| Thuộc tính | Giá trị |
|------------|---------|
| Binary | `/usr/local/bin/coremanager` |
| D-Bus Name | `com.example.coremanager` |
| Dependencies | libdbus-1, libsystemd, libboost-system, libboost-filesystem, libsqlite3 |
| Chức năng | WebSocket server (port 9000), SQLite DB, business logic, timer, state management |

### HardwareManager (`hardwaremgr`)

| Thuộc tính | Giá trị |
|------------|---------|
| Binary | `/usr/local/bin/hardwaremanager` |
| D-Bus Name | `com.example.hardwaremanager` |
| Dependencies | libdbus-1, libsystemd, bluez, bluez-obex, ofono, dbus-user-session |
| Chức năng | Bluetooth scan/pair/connect (BlueZ), HFP voice calls (oFono), PBAP phonebook (obexd), temperature sensor (GPIO/1-Wire) |

### RecordManager (`recordmgr`)

| Thuộc tính | Giá trị |
|------------|---------|
| Binary | `/usr/local/bin/recordmanager` |
| D-Bus Name | `com.example.recordmanager` |
| Dependencies | libdbus-1, libsystemd, libasound2, alsa-utils |
| Chức năng | Audio recording (ALSA), WAV file output, audio noise filtering (sox) |

---

## 7. D-Bus IPC Communication

3 services giao tiếp qua **D-Bus System Bus** bằng signals:

```
CoreManager  ──signal──→  HardwareManager   (e.g., START_SCAN_BTDEVICE)
HardwareManager ──signal──→  CoreManager     (e.g., SCAN_BTDEVICE_FOUND_NOTI)
CoreManager  ──signal──→  RecordManager      (e.g., START_RECORD)
RecordManager ──signal──→  CoreManager       (e.g., RECORD_COMPLETE_NOTI)
```

Mỗi service có file `.conf` (D-Bus security policy) được cài vào `/etc/dbus-1/system.d/` để cho phép user `pi` own và gửi message.

---

## 8. Kiểm Tra Trên Pi

### Xem trạng thái service

```bash
sudo systemctl status coremanager
sudo systemctl status hardwaremanager
sudo systemctl status recordmanager
```

### Xem log realtime

```bash
# Tất cả 3 services
journalctl -f -u coremanager -u hardwaremanager -u recordmanager

# 1 service cụ thể
journalctl -f -u hardwaremanager
```

### Restart service

```bash
sudo systemctl restart coremanager
sudo systemctl restart hardwaremanager
sudo systemctl restart recordmanager
```

### Kiểm tra D-Bus

```bash
# Xem service đã đăng ký trên system bus
busctl list | grep example

# Monitor D-Bus signals
dbus-monitor --system "sender='com.example.hardwaremanager'"
```

### Kiểm tra Bluetooth

```bash
bluetoothctl
  > power on
  > scan on
  > devices
```

---

## 9. Troubleshooting

### Service không start

```bash
# Xem log chi tiết
journalctl -u hardwaremanager -n 50 --no-pager

# Kiểm tra binary tồn tại
ls -la /usr/local/bin/hardwaremanager

# Kiểm tra D-Bus config
ls -la /etc/dbus-1/system.d/com.example.*.conf
```

### D-Bus permission denied

```bash
# Kiểm tra D-Bus config đã được cài
cat /etc/dbus-1/system.d/com.example.hardwaremanager.conf

# Reload D-Bus
sudo systemctl reload dbus
```

### PBAP (phonebook) không hoạt động

```bash
# Kiểm tra obexd đang chạy
ps aux | grep obexd

# Kiểm tra session bus khả dụng
echo $DBUS_SESSION_BUS_ADDRESS
# Phải thấy: unix:path=/run/user/1000/bus

# Kiểm tra lingering enabled
loginctl show-user pi | grep Linger
# Phải thấy: Linger=yes
```

### Không kết nối được SSH

```bash
# Kiểm tra Pi có online không
ping 192.168.1.50

# Nếu IP khác, sửa trong setenv.sh và các deploy.sh:
# PI_IP_ADDRESS="<ip-mới>"   (setenv.sh)
# PI_HOST="<ip-mới>"          (*/deploy.sh)
```

### Cross-compile lỗi "cannot find -lxxx"

```bash
# Sysroot thiếu thư viện → cài lại trên Pi rồi rsync lại
ssh pi@192.168.1.50 "sudo apt install -y libXXX-dev"

# Rsync lại sysroot
make setenv
# Hoặc chỉ rsync thủ công:
rsync -avz --rsync-path="sudo rsync" pi@192.168.1.50:/usr ~/raspberrypi/sysroot/
```

---

## Quick Start (TL;DR)

```bash
# 1. Clone
git clone https://github.com/sanghandsondev/raspberrypiservice.git
cd raspberrypiservice

# 2. Sửa IP nếu Pi không phải 192.168.1.50
#    → setenv.sh: PI_IP_ADDRESS="..."
#    → coremgr/deploy.sh, hardwaremgr/deploy.sh, recordmgr/deploy.sh: PI_HOST="..."

# 3. Setup (1 lần duy nhất)
make setenv

# 4. Build cross + deploy lên Pi
make deploy

# 5. Kiểm tra trên Pi
ssh pi@192.168.1.50
sudo systemctl status coremanager hardwaremanager recordmanager
journalctl -f -u coremanager -u hardwaremanager -u recordmanager
```