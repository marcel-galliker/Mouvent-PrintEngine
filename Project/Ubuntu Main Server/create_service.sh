#!/bin/bash

# Old radex file
ORF=/etc/init.d/radex.sh

# New service file
NSF=/etc/systemd/system/radex.service

# New radex file
NRF=/opt/radex/radex.sh

echo 'Deleting old radex.sh startup script'
rm $ORF

echo 'Creating service file'
touch $NSF
echo $'[Unit]\nDescription=rx_main_ctrl start\n\nWants=network-online.target\n\nAfter=network-online.target\n\n[Service]\nType=forking\nUser=root\nExecStart=/opt/radex/radex.sh\n\n[Install]\nWantedBy=multi-user.target' >> $NSF


echo 'Creating new radex.sh script'
echo $'#!/bin/bash\n\nkillall rx_main_ctrl\nkillall rx_dhcp_server\nkillall rx_spooler_ctrl\n\nsleep 5\n\n/opt/radex/bin/main/rx_dhcp_server &\n/opt/radex/bin/main/rx_main_ctrl' >> $NRF
chmod -R 777 $NRF

echo 'Enable Service'
systemctl enable radex.service

echo 'Start service'
systemctl start radex.service

echo 'DONE'

