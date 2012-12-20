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

def wsn_manage():
    ls_nodes = ls_nodes_from_borderrouter()
    for node in ls_nodes['Router']:
        print node
        walk_through_node(node)

def walk_through_node(node):
    communityro = 'public'
    #floowing command is hard coded, they should be replaced with one read from cfg file
    cg = cmdgen.CommandGenerator()
    comm_data = cmdgen.CommunityData('my-manager', communityro, mpModel=0)

    #transport = cmdgen.Udp6TransportTarget(('aaaa::212:7402:2:202', 161))
    transport = cmdgen.Udp6TransportTarget((node, 161))

    variables = (1,3,6,1)
    errIndication, errStatus, errIndex, sysinfo_result = cg.nextCmd(comm_data, transport, variables,)
    if errStatus and errStatus != 2:
        raise Exception(errStatus)
    for entry in sysinfo_result:
        for name, val in entry:
            print('from: %s, %s = %s' % (node, name.prettyPrint(), val.prettyPrint()))

def ls_nodes_from_borderrouter():
    MAX = 128
    PORT = 3001
    hostname = 'aaaa::212:7401:1:101'

    s = socket.socket(socket.AF_INET6, socket.SOCK_DGRAM)
    s.connect((hostname, PORT))

    print 'Client socket name is', s.getsockname()
    delay = 1
    while True:
        s.send('hello from client')
        #print 'Waiting up to', delay, 'seconds for a reply'
        s.settimeout(delay)
        try:
            data, address = s.recvfrom(MAX)
        except socket.timeout:
            delay *= 2  # wait even longer for the next request
            if delay > 2.5:
                raise RuntimeError('I think the server is down')
        else:
            break   # we are done, and can stop looping

    #print 'The server ', address, 'says', data

    #TODO: check for error when converting into dictionary
    ls_nodes = ast.literal_eval(data)
    return ls_nodes

if __name__ == '__main__':
    wsn_manage()
