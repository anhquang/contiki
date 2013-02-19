#!/usr/bin/env python
# client UDP program that connect to an ipv6 server at PORT
# using: python udp_remote-upv6.python
# first, contact the borderrouter to get list of hosts in the network
# then, walk through all nodes, retrive all possible information (thanks to
# walk command)

import random, socket, sys
import json
import ast
from pysnmp.entity.rfc3413.oneliner import cmdgen
from pysnmp.entity import engine, config


def walk_through_node():
    communityro = 'public'
    node = 'aaaa::212:7404:4:404'

    #floowing command is hard coded, they should be replaced with one read from cfg file
    cg = cmdgen.CommandGenerator()
    comm_data = cmdgen.CommunityData('my-manager', communityro, mpModel=0)

    #transport = cmdgen.Udp6TransportTarget(('aaaa::212:7402:2:202', 161))
    transport = cmdgen.Udp6TransportTarget((node, 161))

    variables = (1,3,6,1)
    errIndication, errStatus, errIndex, sysinfo_result = cg.nextCmd(comm_data, transport, variables,)

    print 'errStatus = %s, errIndex = %s' %(errStatus, errIndex)
    if errStatus and errStatus != 2:
        raise Exception(errStatus)
    for entry in sysinfo_result:
        for name, val in entry:
            print('from: %s, %s = %s' % (node, name.prettyPrint(), val.prettyPrint()))


def get_from_node():
    node = 'aaaa::212:7404:4:404'

    communityro = 'public'
    variables = (1,3,6,1,2,1,1,5,0)

    #floowing command is hard coded, they should be replaced with one read from cfg file
    cg = cmdgen.CommandGenerator()
    comm_data = cmdgen.CommunityData('my-manager', communityro, mpModel=0)

    transport = cmdgen.Udp6TransportTarget((node, 161))
    errIndication, errStatus, errIndex, sysinfo_result = cg.getCmd(comm_data, transport, variables,)

    print 'errStatus = %s, errIndex = %s' %(errStatus, errIndex)

    if errStatus and errStatus != 2:
        print 'error'
        raise Exception(errStatus)
    for name, val in sysinfo_result:
        print('from: %s, %s = %s' % (node, name.prettyPrint(), val.prettyPrint()))


if __name__ == '__main__':
    #get_from_node()
    walk_through_node()
