#snmpwalk -v 1 -c public udp6:[fe80::ff:fe00:10%tap0] .1.3.6
#echo "---------------------------------------"
#snmpwalk -v 1 -c public udp6:[aaaa::212:7402:2:202] .1.3.6.1
#echo "---------------------------------------"
#snmpwalk -v 1 -c public udp6:[aaaa::212:7404:4:404] .1.3.6.1
#echo "---------------------------------------"
#snmpwalk -v 1 -c public udp6:[aaaa::212:7405:5:505] .1.3.6.1

#snmpget -v 1 -c public udp6:[aaaa::212:7404:4:404] SNMPv2-MIB::sysName.0

snmpget -v 1 -c public udp6:[aaaa::212:7404:4:404] SNMPv2-MIB::sysName.0
snmpset -v 1 -c public udp6:[aaaa::212:7404:4:404] SNMPv2-MIB::sysName.0 s "khakha"
snmpget -v 1 -c public udp6:[aaaa::212:7404:4:404] SNMPv2-MIB::sysName.0

snmpset -v 1 -c public udp6:[aaaa::212:7404:4:404] SNMPv2-MIB::sysName.0 s "day la mot chuong trinh kha dai"
snmpget -v 1 -c public udp6:[aaaa::212:7404:4:404] SNMPv2-MIB::sysName.0
