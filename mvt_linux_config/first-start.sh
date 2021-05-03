#!/bin/bash

# Change eth names
python3 /home/mouvent/update-eth-names.py

# Configure user root
echo root:radex | chpasswd
mkdir -p "/home/root/.ssh"
chmod -R 700 "/home/root"

# Create user radex
useradd -m radex
echo radex:radex | chpasswd
mkdir -p "/home/radex/source-data"
mkdir -p "/home/radex/print-data"
chmod -R 777 "/home/radex"

# Copy Print Engine
mkdir "/opt/radex"
unzip /home/mouvent/PrintEngine.zip -d /opt/radex
chmod -R 777 "/opt/radex"

# Create Mouvent service
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

# Remove startup script
rm /etc/rc.local

# Set SSH allowed users
echo -e '\nAllowUsers root radex mouvent' >> /etc/ssh/sshd_config
echo -e '\nPermitRootLogin yes' >> /etc/ssh/sshd_config

# The end
reboot
