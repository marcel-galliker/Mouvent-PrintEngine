#!/bin/bash

################################################################################
# This script:
# * Downloads the ISO of Ubuntu Server 18.04.4 and
# * Customizes it to suit our needs.
#
# Notes:
# * Ubuntu 18.04 is 'based' on Debian 'Buster',
# * Creating a preseed file is extremely complicated because:
#   - There are different installers (console or graphical, with live session or not)
#     having different customization possibilities and incompatible syntaxes.
#     For example:
#     * The last version of Ubuntu Server using Debian-Installer seems to be 18.04.4,
#     * Newer versions of Ubuntu Server (18.04.5, 20.04, 20.10, ...) use Subiquity,
#       which we do not support,
#   - Documentation is scarce and outdated,
#   - Examples often do not work (anymore?) for obscur reasons:
#     * The installer has evolved,
#     * Commands that should work in the preseed file only work in an external script,
#     * There are variations between Debian and Ubuntu,
#     * ...
#   - Partioning is even more difficult because of a cryptic syntax,
# * The Debian documentation advises to run the following commands to generate
#   the list of all existing questions/answers supported by the installer:
#     sudo apt-get install debconf-utils
#     sudo debconf-get-selections --installer > debconf.txt
#     debconf-get-selections >> debconf.txt
#   This proved helpful to confirm the existence of some options, but it is
#   globally unreadable and mostly useless for understanding,
# * Network and mirror configuration has been disabled here (but left in comments
#   for reference) because we do it in the post install script.
#
# Useful links:
# * Tutorials:
#   - Preseeding:
#     * https://www.frakkingsweet.com/debian-preseed-and-docker
#     * https://www.pugetsystems.com/labs/hpc/Note-Auto-Install-Ubuntu-with-Custom-Preseed-ISO-1654
#     * https://www.vm.ibm.com/education/lvc/LVC0803.pdf
#   - Partman:
#     * https://wikitech.wikimedia.org/wiki/PartMan
#     * https://wikitech.wikimedia.org/wiki/PartMan/Auto
#   - Debconf:
#     * http://www.fifi.org/doc/debconf-doc/tutorial.html
# * Documentation:
#   - Debian:
#     * https://wiki.debian.org/DebianInstaller/Preseed
#     * https://www.debian.org/releases/buster/amd64
#     * https://www.debian.org/releases/buster/amd64/apb.en.html
#     * https://www.debian.org/releases/buster/amd64/apbs01.en.html#preseed-methods
#     * https://www.debian.org/releases/buster/amd64/apbs02.en.html
#     * https://www.debian.org/releases/buster/amd64/apbs04.en.html
#     * https://www.debian.org/releases/buster/amd64/apbs05.en.html
#   - Ubuntu:
#     * https://help.ubuntu.com/community/InstallCDCustomization
#     * https://help.ubuntu.com/community/LiveCDCustomization
#     * https://help.ubuntu.com/lts/installation-guide/amd64
#     * https://help.ubuntu.com/lts/installation-guide/amd64/apb.html
#     * https://help.ubuntu.com/lts/installation-guide/amd64/apbs01.html#preseed-methods
#     * https://help.ubuntu.com/lts/installation-guide/amd64/apbs02.html
#     * https://help.ubuntu.com/lts/installation-guide/amd64/apbs04.html
#     * https://help.ubuntu.com/lts/installation-guide/amd64/apbs05.html
#     * https://odm.ubuntu.com
#     * https://wiki.ubuntu.com/Kernel/LTSEnablementStack
#     * https://wiki.ubuntu.com/Kernel/RollingLTSEnablementStack
#   - Chroot:
#     * https://en.wikipedia.org/wiki/Chroot
#   - Debconf:
#     * https://www.debian.org/doc/packaging-manuals/debconf_specification.html
#   - SYSLINUX:
#     * https://wiki.syslinux.org
#     * https://wiki.syslinux.org/wiki/index.php?title=SYSLINUX
#   - Autoinstall (new Ubuntu server installer, not used):
#     * https://ubuntu.com/server/docs/install/autoinstall
#     * https://ubuntu.com/server/docs/install/autoinstall-quickstart
#     * https://ubuntu.com/server/docs/install/autoinstall-reference
#   - Ubiquity (!= Subiquity):
#     * https://wiki.ubuntu.com/Ubiquity
#     * https://wiki.ubuntu.com/UbiquityAutomation
#     * https://wiki.ubuntu.com/DebuggingUbiquity
#     * https://wiki.ubuntu.com/DebuggingUbiquity/AttachingLogs
#   - Other:
#     * https://www.pendrivelinux.com/vga-boot-modes-to-set-screen-resolution
# * Preseed file examples:
#   - https://preseed.debian.net/debian-preseed
#   - https://preseed.debian.net/debian-preseed/buster
#   - https://www.debian.org/releases/buster/example-preseed.txt
#   - https://help.ubuntu.com/lts/installation-guide/example-preseed.txt
#   - https://gist.github.com/diasjorge/9ac754ad9a0fdc1a62b0
#   - https://gist.github.com/moutend/cd60aca079864d1933bdac80d8049b69
#   - https://github.com/yuryu/preseed/blob/master/debian-atomic.cfg
#   - https://gitlab.rayark.com/open/ubuntu-preseed/blob/master/ubuntu.seed
#   - https://lists.debian.org/debian-user/2009/10/msg00149.html
# * FAQs:
#   - https://askubuntu.com/questions/1219749/how-to-install-software-using-preseed-cfg-ubuntu-18-04-4
#   - https://askubuntu.com/questions/1233454/how-to-preseed-ubuntu-20-04-desktop
#   - https://ma.ttias.be/how-to-generate-a-passwd-password-hash-via-the-command-line-on-linux
#   - https://matelakat.blogspot.com/2014/05/ubuntu-installer-unmount-partitions.html
#   - https://serverfault.com/questions/550072/preseeding-ubuntu-partman-recipe-using-lvm-and-raid
# * Downloads:
#   - https://releases.ubuntu.com
# * Tools:
#   - https://fai-project.org
#   - https://launchpad.net/cubic
################################################################################

################################################################################
# Gets the Ubuntu Server ISO.
################################################################################

UBUNTU_REPO='https://old-releases.ubuntu.com/releases/18.04.4'
UBUNTU_ISO='ubuntu-18.04.4-server-amd64.iso'

if [ ! -f "${UBUNTU_ISO}" ]; then
  echo "Please wait, downloading ISO image ${UBUNTU_ISO} ..."
  wget "${UBUNTU_REPO}/${UBUNTU_ISO}"
fi

################################################################################
# Mounts the ISO.
################################################################################

UBUNTU_DIR='UbuntuServer'

if [ ! -d "${UBUNTU_DIR}" ]; then
  echo "Creating directory ${UBUNTU_DIR} ..."
  mkdir "${UBUNTU_DIR}"
fi
if ! mountpoint -q "${UBUNTU_DIR}"; then
  echo "Mounting ISO image in ${UBUNTU_DIR} ..."
  sudo mount -o loop,ro "${UBUNTU_ISO}" "${UBUNTU_DIR}"
fi

################################################################################
# Copies the directory for modifications.
################################################################################

MOUVENT_DIR='MouventServer'

if [ -d "${MOUVENT_DIR}" ]; then
  echo "Removing directory ${MOUVENT_DIR} ..."
  chmod -R u+w "${MOUVENT_DIR}"
  rm -rf "${MOUVENT_DIR}"
fi
echo "Copying directory ${UBUNTU_DIR} to ${MOUVENT_DIR} ..."
cp -r "${UBUNTU_DIR}" "${MOUVENT_DIR}"

################################################################################
# Customizes the preseed.
################################################################################

CONFIG_SEED_FILE='mouvent-server.seed'
CONFIG_LOCALE='en_US'                                   # Format: languagecode_COUNTRYCODE.
CONFIG_KEYBOARD='us'
#CONFIG_DOMAIN='unassigned-domain'                       # No domain.
#CONFIG_HOST_NAME='MouventPrinter'
CONFIG_USER_FULL_NAME='Mouvent'
CONFIG_USER_NAME='mouvent'
CONFIG_USER_PASSWORD='Gutenberg1991'
CONFIG_TIME_ZONE='Europe/Zurich'                        # See /usr/share/zoneinfo/ for valid values.
CONFIG_SET_PARTITION_DISK_FILE='set_partition_disk.sh'
CONFIG_PARTITION_METHOD='regular'                       # Usual partition scheme: no LVM or LVM+encryption.
CONFIG_PARTITION_RECIPE='atomic'                        # One single partition for /, /home, /usr, ...
CONFIG_INSTALLED_KERNEL='hwe-18.04'
CONFIG_UPDATE_POLICY='none'                             # No automatic updates.
#CONFIG_SERVER_TYPE='server'                             # Simple server without additional options.
CONFIG_POST_INSTALL_FILE='post_install.sh'

chmod u+w "${MOUVENT_DIR}/preseed"
rm -f "${MOUVENT_DIR}/preseed"/*
cat << EOF > "${MOUVENT_DIR}/preseed/${CONFIG_SEED_FILE}"
### Unmount '/media' to avoid complications with 'partman'.
d-i preseed/early_command string umount /media

### Automatic installation.
d-i auto-install/enable boolean true
d-i debconf/priority select critical

### Localization.
d-i debian-installer/locale string ${CONFIG_LOCALE}.UTF-8
d-i localechooser/supported-locales multiselect ${CONFIG_LOCALE}.UTF-8

### Keyboard.
d-i console-setup/ask_detect boolean false
d-i keyboard-configuration/xkb-keymap select ${CONFIG_KEYBOARD}

### Network.
d-i netcfg/enable boolean false
#d-i netcfg/choose_interface select auto
#d-i netcfg/get_domain string ${CONFIG_DOMAIN}
#d-i netcfg/get_hostname string ${CONFIG_HOST_NAME}
#d-i hw-detect/load_firmware boolean true

### Mirror.
d-i mirror/enable boolean false
#d-i mirror/country string manual
#d-i mirror/http/hostname string archive.ubuntu.com
#d-i mirror/http/directory string /ubuntu
#d-i mirror/http/proxy string

### User.
d-i passwd/user-fullname string ${CONFIG_USER_FULL_NAME}
d-i passwd/username string ${CONFIG_USER_NAME}
d-i passwd/user-password password ${CONFIG_USER_PASSWORD}
d-i passwd/user-password-again password ${CONFIG_USER_PASSWORD}
d-i user-setup/allow-password-weak boolean true

### Clock and time zone:
### * Sets the clock using NTP (if available),
### * Sets the system clock to UTC
###   (both 'utc' and 'utc-auto' exist, without a clear distinction).
d-i clock-setup/ntp boolean true
d-i clock-setup/utc boolean true
d-i clock-setup/utc-auto boolean true
d-i time/zone string ${CONFIG_TIME_ZONE}

### Disk partition.
d-i partman/early_command string sh /cdrom/preseed/${CONFIG_SET_PARTITION_DISK_FILE}
d-i partman/unmount_active boolean true
d-i partman-auto/method string ${CONFIG_PARTITION_METHOD}
d-i partman-auto/choose_recipe select ${CONFIG_PARTITION_RECIPE}
d-i partman-partitioning/confirm_write_new_label boolean true
d-i partman/choose_partition select finish
d-i partman/confirm boolean true
d-i partman/confirm_nooverwrite boolean true

### Installed kernel.
d-i base-installer/kernel/altmeta string ${CONFIG_INSTALLED_KERNEL}

### Updates.
d-i pkgsel/update-policy select ${CONFIG_UPDATE_POLICY}

### Server base.
#tasksel tasksel/force-tasks string ${CONFIG_SERVER_TYPE}
tasksel tasksel/first multiselect openssh-server
d-i pkgsel/include string net-tools samba ssh unzip

### Verbose output and no boot splash screen.
d-i debian-installer/quiet boolean false
d-i debian-installer/splash boolean false

### Custom command that will be run at the end of the process.
d-i preseed/late_command string sh /cdrom/preseed/${CONFIG_POST_INSTALL_FILE}

### Reboot.
d-i finish-install/reboot_in_progress note
EOF
chmod a=r "${MOUVENT_DIR}/preseed/${CONFIG_SEED_FILE}"
cp "${CONFIG_POST_INSTALL_FILE}" \
   "${CONFIG_SET_PARTITION_DISK_FILE}" \
   01-eth-config.yaml \
   first-start.sh \
   PrintEngine.zip \
   smb.conf \
   update-eth-names.py \
   "${MOUVENT_DIR}/preseed"
sed --in-place \
    --expression "s/user_name_placeholder/${CONFIG_USER_NAME}/" \
    --expression "s/user_password_placeholder/${CONFIG_USER_PASSWORD}/" \
    "${MOUVENT_DIR}/preseed/${CONFIG_POST_INSTALL_FILE}"
chmod a=rx "${MOUVENT_DIR}/preseed/${CONFIG_POST_INSTALL_FILE}" \
           "${MOUVENT_DIR}/preseed/${CONFIG_SET_PARTITION_DISK_FILE}" \
           "${MOUVENT_DIR}/preseed/first-start.sh" \
           "${MOUVENT_DIR}/preseed/update-eth-names.py"
chmod u-w "${MOUVENT_DIR}/preseed"

################################################################################
# Customizes the GRUB menu.
################################################################################

chmod u+w "${MOUVENT_DIR}/boot/grub/grub.cfg"
cat << EOF > "${MOUVENT_DIR}/boot/grub/grub.cfg"

if loadfont /boot/grub/font.pf2 ; then
  set gfxmode=auto
  insmod efi_gop
  insmod efi_uga
  insmod gfxterm
  terminal_output gfxterm
fi

set menu_color_normal=white/black
set menu_color_highlight=black/light-gray

set timeout=5
menuentry "Install Mouvent Server with the HWE kernel" {
  set gfxpayload=keep
  linux /install/hwe-vmlinuz file=/cdrom/preseed/${CONFIG_SEED_FILE} auto=true locale=${CONFIG_LOCALE} priority=critical quiet ---
  initrd /install/hwe-initrd.gz
}
EOF
chmod u-w "${MOUVENT_DIR}/boot/grub/grub.cfg"

################################################################################
# Customizes the ISOLINUX menu.
################################################################################

chmod u+w "${MOUVENT_DIR}/isolinux/isolinux.cfg"
sed --in-place 's/timeout 300/timeout 50/' "${MOUVENT_DIR}/isolinux/isolinux.cfg"
chmod u-w "${MOUVENT_DIR}/isolinux/isolinux.cfg"

chmod u+w "${MOUVENT_DIR}/isolinux/txt.cfg"
cat << EOF > "${MOUVENT_DIR}/isolinux/txt.cfg"
default mouvent-server
label mouvent-server
  menu label ^Install Mouvent Server with the HWE kernel
  kernel /install/hwe-vmlinuz
  append file=/cdrom/preseed/${CONFIG_SEED_FILE} vga=788 initrd=/install/hwe-initrd.gz auto=true locale=${CONFIG_LOCALE} priority=critical quiet ---
EOF
chmod u-w "${MOUVENT_DIR}/isolinux/txt.cfg"

################################################################################
# Creates the new ISO image.
################################################################################

UBUNTU_ISO_MBR_FILE='ubuntu_isohybrid_mbr.img'

if [ -f "${UBUNTU_ISO_MBR_FILE}" ]; then
  rm "${UBUNTU_ISO_MBR_FILE}"
fi
dd if="${UBUNTU_ISO}" bs=1 count=446 of="${UBUNTU_ISO_MBR_FILE}"

MOUVENT_ISO='mouvent-server-amd64.iso'

if [ -f "${MOUVENT_ISO}" ]; then
  rm "${MOUVENT_ISO}"
fi
xorriso -as mkisofs -r -V "Mouvent Install CD" \
            -cache-inodes -J -l \
            -isohybrid-mbr "${UBUNTU_ISO_MBR_FILE}" \
            -c isolinux/boot.cat \
            -b isolinux/isolinux.bin \
               -no-emul-boot -boot-load-size 4 -boot-info-table \
            -eltorito-alt-boot \
            -e boot/grub/efi.img \
               -no-emul-boot -isohybrid-gpt-basdat \
            -o "${MOUVENT_ISO}" \
            "${MOUVENT_DIR}"

################################################################################
# Does some cleanup.
################################################################################

echo "Cleaning temporary files ..."
if [ -d "${MOUVENT_DIR}" ]; then
  chmod -R u+w "${MOUVENT_DIR}"
  rm -rf "${MOUVENT_DIR}"
fi
if [ -f "${UBUNTU_ISO_MBR_FILE}" ]; then
  rm "${UBUNTU_ISO_MBR_FILE}"
fi
