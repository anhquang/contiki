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
import time

def ls_nodes_from_borderrouter():
    MAX = 128
    PORT = 2000
    hostname = 'aaaa::212:7402:2:202'

    s = socket.socket(socket.AF_INET6, socket.SOCK_DGRAM)
    s.connect((hostname, PORT))

    print 'Client socket name is', s.getsockname()
    delay = 1
    while True:
        print "begin to send"
        s.send('hello from client')
        data, address = s.recvfrom(MAX)
        time.sleep(10)


if __name__ == '__main__':
    ls_nodes_from_borderrouter()
