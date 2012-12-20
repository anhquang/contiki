#snmpwalk -v 1 -c public udp6:[fe80::ff:fe00:10%tap0] .1.3.6
echo "---------------------------------------"
snmpwalk -v 1 -c public udp6:[aaaa::212:7402:2:202] .1.3.6
echo "---------------------------------------"
snmpwalk -v 1 -c public udp6:[aaaa::212:7404:4:404] .1.3.6
#echo "---------------------------------------"
#snmpwalk -v 1 -c public udp6:[aaaa::212:7405:5:505] .1.3.6
