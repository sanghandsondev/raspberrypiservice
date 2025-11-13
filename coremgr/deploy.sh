#!/bin/bash

PI_USER="pi"
PI_HOST="192.168.1.50"
PROJECT_NAME="coremanager"
SERVICE_FILE="coremanager.service"

LOCAL_BINARY="build/${PROJECT_NAME}"
LOCAL_SERVICE_FILE="${SERVICE_FILE}"

REMOTE_TMP_PATH="/tmp"
REMOTE_INSTALL_PATH="/usr/local/bin"
REMOTE_SERVICE_PATH="/etc/systemd/system"

set -e

# 1. Copy binary and service file to Pi
echo ">>> [1/3] Copying binary and service file to Pi..."
scp "${LOCAL_BINARY}" "${LOCAL_SERVICE_FILE}" "${PI_USER}@${PI_HOST}:${REMOTE_TMP_PATH}"

# 2. Install files on Pi
echo ">>> [2/3] Installing files on Pi..."
ssh "${PI_USER}@${PI_HOST}" "
    set -e
    sudo systemctl stop ${SERVICE_FILE} || true

    sudo mv \"${REMOTE_TMP_PATH}/${PROJECT_NAME}\" \"${REMOTE_INSTALL_PATH}/${PROJECT_NAME}\"
    sudo chmod +x \"${REMOTE_INSTALL_PATH}/${PROJECT_NAME}\"

    sudo mv \"${REMOTE_TMP_PATH}/${SERVICE_FILE}\" \"${REMOTE_SERVICE_PATH}/${SERVICE_FILE}\"
"

# 3. Reload and restart service on Pi
echo ">>> [3/3] Reloading and restarting service on Pi..."
ssh "${PI_USER}@${PI_HOST}" "
    set -e
    sudo systemctl daemon-reload

    sudo systemctl enable ${SERVICE_FILE}

    sudo systemctl restart ${SERVICE_FILE}
    
    sleep 2
    sudo systemctl status ${SERVICE_FILE} --no-pager
"

echo ">>> Done! Deployment completed successfully."