#!/bin/bash

# filepath: /home/sang/sangank/raspberrypiservice/coremgr/deploy.sh

# --- Cấu hình ---
PI_USER="pi"
PI_HOST="<PI_IP_ADDRESS>" # Ví dụ: 192.168.1.100
PROJECT_NAME="coremanager"
SERVICE_FILE="coremanager.service"

# Đường dẫn trên máy tính phát triển
LOCAL_BINARY="build/${PROJECT_NAME}"
LOCAL_SERVICE_FILE="${SERVICE_FILE}"

# Đường dẫn trên Pi
REMOTE_TMP_PATH="/tmp"
REMOTE_INSTALL_PATH="/usr/local/bin"
REMOTE_SERVICE_PATH="/etc/systemd/system"

# Dừng script nếu có lỗi
set -e

echo ">>> Bắt đầu quá trình triển khai tới ${PI_HOST}..."

# 1. Biên dịch chéo
echo ">>> 1/4: Biên dịch chéo dự án..."
make cross

# 2. Sao chép các tệp cần thiết sang Pi
echo ">>> 2/4: Sao chép tệp binary và service tới Pi..."
scp "${LOCAL_BINARY}" "${LOCAL_SERVICE_FILE}" "${PI_USER}@${PI_HOST}:${REMOTE_TMP_PATH}"

# 3. Cài đặt các tệp trên Pi
echo ">>> 3/4: Cài đặt các tệp trên Pi..."
ssh "${PI_USER}@${PI_HOST}" "
    set -e
    echo '    [Pi] Dừng dịch vụ hiện tại...'
    sudo systemctl stop ${SERVICE_FILE} || true

    echo '    [Pi] Cài đặt tệp binary mới...'
    sudo mv \"${REMOTE_TMP_PATH}/${PROJECT_NAME}\" \"${REMOTE_INSTALL_PATH}/${PROJECT_NAME}\"
    sudo chmod +x \"${REMOTE_INSTALL_PATH}/${PROJECT_NAME}\"

    echo '    [Pi] Cài đặt tệp service mới...'
    sudo mv \"${REMOTE_TMP_PATH}/${SERVICE_FILE}\" \"${REMOTE_SERVICE_PATH}/${SERVICE_FILE}\"
"

# 4. Tải lại và khởi động dịch vụ
echo ">>> 4/4: Tải lại và khởi động dịch vụ trên Pi..."
ssh "${PI_USER}@${PI_HOST}" "
    set -e
    echo '    [Pi] Tải lại systemd daemon...'
    sudo systemctl daemon-reload

    echo '    [Pi] Kích hoạt dịch vụ (để tự khởi động cùng hệ thống)...'
    sudo systemctl enable ${SERVICE_FILE}

    echo '    [Pi] Khởi động lại dịch vụ...'
    sudo systemctl restart ${SERVICE_FILE}
    
    echo '    [Pi] Kiểm tra trạng thái dịch vụ...'
    sleep 2 # Đợi một chút để dịch vụ có thời gian khởi động
    sudo systemctl status ${SERVICE_FILE} --no-pager
"

echo ">>> Hoàn tất! Dịch vụ đã được cập nhật trên Pi."