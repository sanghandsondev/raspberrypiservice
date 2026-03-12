#!/bin/bash

PI_USER="pi"
PI_HOST="192.168.1.50"
PROJECT_NAME="recordmanager"
SERVICE_FILE="recordmanager.service"
DBUS_CONF_FILE="com.example.recordmanager.conf"

LOCAL_BINARY="build/${PROJECT_NAME}"
LOCAL_SERVICE_FILE="${SERVICE_FILE}"
LOCAL_DBUS_CONF="${DBUS_CONF_FILE}"

REMOTE_TMP_PATH="/tmp"
REMOTE_INSTALL_PATH="/usr/local/bin"
REMOTE_SERVICE_PATH="/etc/systemd/system"
REMOTE_DBUS_CONF_PATH="/etc/dbus-1/system.d"

set -e

# 1. Install required runtime packages on Pi (idempotent)
echo ">>> [1/5] Ensuring runtime packages are installed on Pi..."
ssh "${PI_USER}@${PI_HOST}" "
    set -e

    # Core runtime + ALSA for audio recording
    PACKAGES='dbus libdbus-1-3 libsystemd0 libasound2 alsa-utils'
    MISSING=''
    for pkg in \$PACKAGES; do
        if ! dpkg -s \"\$pkg\" &>/dev/null 2>&1; then
            MISSING=\"\$MISSING \$pkg\"
        fi
    done

    if [ -n \"\$MISSING\" ]; then
        echo \">>> Installing missing packages:\$MISSING\"
        sudo apt update
        sudo apt install -y \$MISSING
    else
        echo '>>> All runtime packages already installed.'
    fi

    # Ensure pi user is in audio group for ALSA access
    sudo usermod -aG audio ${PI_USER} 2>/dev/null || true
"

# 2. Copy binary, service file, and D-Bus config to Pi
echo ">>> [2/5] Copying binary, service file, and D-Bus config to Pi..."
scp "${LOCAL_BINARY}" "${LOCAL_SERVICE_FILE}" "${LOCAL_DBUS_CONF}" "${PI_USER}@${PI_HOST}:${REMOTE_TMP_PATH}"

# 3. Install files on Pi
echo ">>> [3/5] Installing files on Pi..."
ssh "${PI_USER}@${PI_HOST}" "
    set -e
    sudo systemctl stop ${SERVICE_FILE} 2>/dev/null || true

    # Install binary
    sudo mv \"${REMOTE_TMP_PATH}/${PROJECT_NAME}\" \"${REMOTE_INSTALL_PATH}/${PROJECT_NAME}\"
    sudo chmod +x \"${REMOTE_INSTALL_PATH}/${PROJECT_NAME}\"

    # Install systemd service file
    sudo mv \"${REMOTE_TMP_PATH}/${SERVICE_FILE}\" \"${REMOTE_SERVICE_PATH}/${SERVICE_FILE}\"

    # Install D-Bus security policy config
    sudo mv \"${REMOTE_TMP_PATH}/${DBUS_CONF_FILE}\" \"${REMOTE_DBUS_CONF_PATH}/${DBUS_CONF_FILE}\"
    sudo chmod 644 \"${REMOTE_DBUS_CONF_PATH}/${DBUS_CONF_FILE}\"
"

# 4. Reload D-Bus to pick up new config
echo ">>> [4/5] Reloading D-Bus configuration..."
ssh "${PI_USER}@${PI_HOST}" "
    set -e
    sudo systemctl reload dbus 2>/dev/null || sudo systemctl restart dbus
"

# 5. Reload and restart service on Pi
echo ">>> [5/5] Reloading and restarting service on Pi..."
ssh "${PI_USER}@${PI_HOST}" "
    set -e
    sudo systemctl daemon-reload
    sudo systemctl enable ${SERVICE_FILE}
    sudo systemctl restart ${SERVICE_FILE}
    
    sleep 2
    sudo systemctl status ${SERVICE_FILE} --no-pager
"

echo ">>> Done! ${PROJECT_NAME} deployment completed successfully."