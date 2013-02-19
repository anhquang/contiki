#!/usr/bin/env python
# client UDP program that connect to an ipv6 server at PORT
# using: python udp_remote-upv6.python
# first, contact the borderrouter to get list of hosts in the network
# then, walk through all nodes, retrive all possible information (thanks to
# walk command)

import random, socket, sys
import json
import ast

def main():
    MAX = 128
    PORT = 1200
    hostname = 'aaaa::212:7402:2:202'

    s = socket.socket(socket.AF_INET6, socket.SOCK_DGRAM)
    s.connect((hostname, PORT))

    print 'Client socket name is', s.getsockname()
    delay = 1
    while True:
        s.send('hello')
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

    print 'The server ', address, 'says', data

    #TODO: check for error when converting into dictionary
    ls_nodes = ast.literal_eval(data)
    return ls_nodes

if __name__ == '__main__':
    main()
