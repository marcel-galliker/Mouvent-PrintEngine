#! /bin/sh
### BEGIN INIT INFO
# Provides:          RxBoot
# Required-Start:    
# Required-Stop:     
# Default-Start:    98 
# Default-Stop:     98
# Short-Description: starts radex main executable
# Description:       
#
# Install:
#	ON SOC SYSTEMS:		"rx_boot" = rx_boot_soc
#	ON UNUNTU SYSTEMS:	"rx_boot" = rx_boot_lx
#	> copy "rx_boot" into /opt/radex (add execution rights!)
#	> su
#	> place this file into /etc/init.d	(add execution rights!)
#	> cd /etc/rcS.d	(soc)
#	> cd /etc/rc2.d (ubuntu)
#	> ln –s /etc/init.d/radex.sh S98radex.sh
#	> sudo update-rc.d radex.sh defaults
#
#	Creating public keys: http://serverfault.com/questions/241588/how-to-automate-ssh-login-with-password
#
#   To start a program in Linux in background ad a '&' at the end of the command line !
### END INIT INFO
#
# Author: Marcel Galliker <marcel@radex-net.com>

# echo Startig rx_boot
/opt/radex/rx_boot_soc
# /opt/radex/rx_boot_lx

# case "$1" in
#    start)
##        /opt/radex/rx_boot start
#        /opt/radex/rx_boot
#        ;;
#    stop)
##        /opt/radex/rx_boot stop
#        ;;
#    restart)
##        /opt/radex/rx_boot restart
#        ;;
#esac
#
exit 0
