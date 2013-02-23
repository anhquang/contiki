#!/usr/bin/env python
# client UDP program that connect to an ipv6 server at PORT
# using: python udp_remote-upv6.python
# first, contact the borderrouter to get list of hosts in the network
# then, walk through all nodes, retrive all possible information (thanks to
# walk command)

import random, socket, sys
import json
import ast
import struct

def main(hostname):
    MAX = 128
    PORT = 1200
    if not hostname:
        hostname = 'aaaa::212:7402:2:202'

    s = socket.socket(socket.AF_INET6, socket.SOCK_DGRAM)
    s.connect((hostname, PORT))

    print 'Client socket name is', s.getsockname()
    delay = 1

    data = [1, 20, 0, 20, 20]
    packet = struct.Struct('5b')
    packed_data = packet.pack(*data)

    while True:
        s.send(packed_data)
        #print 'Waiting up to', delay, 'seconds for a reply'
        s.settimeout(delay)
        try:
            data, address = s.recvfrom(MAX)
        except socket.timeout:
            delay *= 2  # wait even longer for the next request
            if delay > 1.5:
                raise RuntimeError('I think the server is down')
        else:
            break   # we are done, and can stop looping

    print 'The server ', address, 'says', data

if __name__ == '__main__':
    if sys.argv[1] == '1':
        hostname = 'aaaa::212:7401:1:101'
    elif sys.argv[1] == '2':
        hostname = 'aaaa::212:7402:2:202'
    elif sys.argv[1] == '3':
        hostname = 'aaaa::212:7403:3:303'
    elif sys.argv[1] == '4':
        hostname = 'aaaa::212:7404:4:404'
    else:
        print 'do nothing'

    print 'send message to %s' %hostname
    main(hostname)
