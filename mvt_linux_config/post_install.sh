#!/bin/sh

# Notes:
# * The source media is mounted in '/cdrom' (be it a CD/DVD or a USB dongle),
# * The target system is mounted in '/target'.
#   So '/target' will be '/' of the installed system.
SOURCE_ROOT_DIR='/cdrom'
TARGET_ROOT_DIR='/target'
TARGET_USER_NAME='user_name_placeholder'
TARGET_USER_PASSWORD='user_password_placeholder'
TARGET_USER_DIR="${TARGET_ROOT_DIR}/home/${TARGET_USER_NAME}"

# Copies configuration files.
cp "${SOURCE_ROOT_DIR}/preseed/first-start.sh" "${TARGET_ROOT_DIR}/etc/rc.local"
cp "${SOURCE_ROOT_DIR}/preseed/update-eth-names.py" "${TARGET_USER_DIR}"
cp "${SOURCE_ROOT_DIR}/preseed/01-eth-config.yaml" "${TARGET_ROOT_DIR}/etc/netplan/"
cp "${SOURCE_ROOT_DIR}/preseed/smb.conf" "${TARGET_ROOT_DIR}/etc/samba/"
cp "${SOURCE_ROOT_DIR}/preseed"/*.deb "${TARGET_USER_DIR}"
cp "${SOURCE_ROOT_DIR}/preseed/PrintEngine.zip" "${TARGET_USER_DIR}"

chmod 777 "${TARGET_ROOT_DIR}/etc/rc.local"
