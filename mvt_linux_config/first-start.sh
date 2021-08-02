#!/bin/bash

# Changes eth names.
# https://wiki.debian.org/NetworkInterfaceNames
# https://askubuntu.com/questions/1245054/netplan-set-name-option-not-working-on-freshly-installed-ubuntu-18-04-lts-deskto
python3 /home/mouvent/update-eth-names.py
rm /home/mouvent/update-eth-names.py
sed -i 's/GRUB_CMDLINE_LINUX=""/GRUB_CMDLINE_LINUX="net.ifnames=0 biosdevname=0"/' /etc/default/grub
netplan apply
cp /run/systemd/network/*.link /etc/systemd/network/
update-initramfs -u

# Configures user root.
echo root:radex | chpasswd
mkdir -p "/home/root/.ssh"
chmod -R 700 "/home/root"

# Creates user radex.
useradd -m radex
echo radex:radex | chpasswd
mkdir -p "/home/radex/source-data"
mkdir -p "/home/radex/print-data"
chmod -R 777 "/home/radex"

# Installs additional packages.
dpkg --install /home/mouvent/*.deb
rm /home/mouvent/*.deb

# Copies Print Engine.
mkdir "/opt/radex"
unzip /home/mouvent/PrintEngine.zip -d /opt/radex
chmod -R 777 "/opt/radex"
rm /home/mouvent/PrintEngine.zip

# Creates Mouvent service.
SERVICE_FILE=/etc/systemd/system/radex.service
STARTUP_FILE=/opt/radex/mouvent.sh

echo 'Creating service file'
rm $SERVICE_FILE
touch $SERVICE_FILE
echo $'[Unit]\nDescription=rx_main_ctrl start\n\nWants=network-online.target\n\nAfter=network-online.target\n\n[Service]\nType=forking\nUser=root\nExecStart=/opt/radex/mouvent.sh\n\n[Install]\nWantedBy=multi-user.target' >> $SERVICE_FILE

rm $STARTUP_FILE
echo 'Creating new mouvent.sh script'
echo $'#!/bin/bash\n\nnetplan apply\n\nkillall rx_main_ctrl\nkillall rx_dhcp_server\nkillall rx_spooler_ctrl\n\nsleep 5\n\n/opt/radex/bin/main/rx_dhcp_server &\n/opt/radex/bin/main/rx_main_ctrl' >> $STARTUP_FILE
chmod -R 777 $STARTUP_FILE

echo 'Enable Service'
systemctl enable radex.service

#echo 'Start service'
systemctl start radex.service

# Removes startup script.
rm /etc/rc.local

# Sets SSH allowed users.
echo -e '\nAllowUsers root radex mouvent' >> /etc/ssh/sshd_config
echo -e '\nPermitRootLogin yes' >> /etc/ssh/sshd_config

# The end.
reboot
