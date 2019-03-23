#!/bin/sh

#ifconfig eth0 down
#ifconfig eth0 hw ether 00:84:14:80:00:03
#ifconfig eth0 up

if [ ! -f /mnt/mtd/if.cfg ]; then
	echo "no if.cfg file, auto to make"
	echo "ip 192.168.1.233" > /mnt/mtd/if.cfg
	echo "netmask 255.255.0.0" >> /mnt/mtd/if.cfg
#	echo "mac 00:84:14:80:00:03" >> /mnt/mtd/if.cfg
else

	ip=`grep -e '^ip ' /mnt/mtd/if.cfg | sed -e 's/^ip //g'`
	netmask=`grep -e '^netmask ' /mnt/mtd/if.cfg | sed -e 's/^netmask //g'`
#	mac=`grep -e '^mac ' /mnt/mtd/if.cfg | sed -e 's/^mac //g'`

	#if [ -z $ip ] || [ -z $mac ]; then
	if [ -z $ip ]; then
		echo "if.cfg file is empty, auto to make"
		echo "ip 192.168.1.233" >> /mnt/mtd/if.cfg
	fi
	
	if [ -z $netmask ]; then
		echo "if.cfg file is empty, auto to make"
		echo "netmask 255.255.0.0" >> /mnt/mtd/if.cfg
	fi
fi

ip=`grep -e '^ip ' /mnt/mtd/if.cfg | sed -e 's/^ip //g'`
netmask=`grep -e '^netmask ' /mnt/mtd/if.cfg | sed -e 's/^netmask //g'`
#mac=`grep -e '^mac ' /mnt/mtd/if.cfg | sed -e 's/^mac //g'`

echo "set ip to $ip"
echo "set netmask to $netmask"
#echo "set mac to $mac"

ifconfig eth0 down
#ifconfig eth0 hw ether $mac
ifconfig eth0 up
ifconfig eth0 $ip netmask $netmask

ifconfig lo down
ifconfig lo up
