#!/bin/sh

# Detects if we install from a USB dongle.
# Note: here we assume that 'there is a USB partition' => 'we install from a USB dongle'.
USB_DEV=$(list-devices usb-partition | sed 's/[0-9]*$//' | sort -u | head -1)

# Sets the target disk accordingly.
if [ -z "${USB_DEV}" ]; then
  DISK_DEV=$(list-devices disk | head -1);
else
  DISK_DEV=$(list-devices disk | grep -v "${USB_DEV}" | head -1);
fi

# Configures partman and GRUB.
debconf-set partman-auto/disk "${DISK_DEV}"
debconf-set grub-installer/bootdev "${DISK_DEV}"
