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
import traceback
from ctypes import *

MNPORT = 10000
MAX = 128

MAX_NUM_SENSOR = 10

VERSION_01	= 1
REQUEST		= 0
RESPONSE	= 1
COMMAND_START	= 1
COMMAND_STOP	= 0
SEND_ACTIVE_NO	= COMMAND_STOP
SEND_ACTIVE_YES	= COMMAND_START

ERROR = -1

class collectd_resp_data_t(Structure):
    _fields_ = [("seqno", c_ubyte),
                ("len", c_ushort),
                ("clock", c_ushort),
                ("timesynch_time", c_ushort ) ,
                ("cpu", c_ushort),
                ("lpm", c_ushort),
                ("transit", c_ushort),
                ("listen", c_ushort),
                ("parent", c_ushort),
                ("parent_etx", c_ushort),
                ("current_rtmetric", c_ushort),
                ("num_neighbors", c_ushort),
                ("beacon_interval", c_ushort),
                ("sensors", c_ushort*MAX_NUM_SENSOR)]

class collectd_object_t(Structure):
    _fields_ = [("version", c_ubyte),
                ("id", c_ubyte),
                ("type", c_ubyte),
                ("resp_data", collectd_resp_data_t)]

def send_request(hostname):
    PORT = 8775
    if not hostname:
        hostname = 'aaaa::212:7402:2:202'

    s = socket.socket(socket.AF_INET6, socket.SOCK_DGRAM)
    try:
        s.connect((hostname, PORT))
    except:
        raise RuntimeError('Host %s is unreachable' %hostname)

    print 'Client socket name is', s.getsockname()
    delay = 1

    data = [1, 20, 0, 1, MNPORT]
    packet = struct.Struct('!4B H')
    packed_data = packet.pack(*data)

    s.send(packed_data)
    #print 'Waiting up to', delay, 'seconds for a reply'
    s.settimeout(delay)

    print 'Did send a request'

def collectd_dispatcher(address, payload):
    print 'The client at ', address, 'says', repr(payload)

    code = str(len(payload))+'B'
    s = struct.Struct(code)
    encodeddata = s.unpack(payload)


    pos = 0
    if encodeddata[pos] <> VERSION_01:
        print "ERROR, different version"
        return ERROR

    pos += 1
    id = encodeddata[pos]
    #ignore id for now
    pos += 1
    type = encodeddata[pos]
    if type <> RESPONSE:
        print "ERROR, not a response msg"
        return ERROR

    pos += 1
    seqno = encodeddata[pos]
    #ignore sequence number for now
    pos += 1
    collectdata_len = encodeddata[pos]*256 + encodeddata[pos+1]
    if collectdata_len*2+pos <> len(payload):
        print "ERROR, corrupted msg (based on len)"
        return ERROR

    pos += 2
    clock= encodeddata[pos]*256 + encodeddata[pos+1]
    pos += 2
    timesynch_time= encodeddata[pos]*256 + encodeddata[pos+1]
    pos += 2
    cpu = encodeddata[pos]*256 + encodeddata[pos+1]
    pos += 2
    lpm = encodeddata[pos]*256 + encodeddata[pos+1]
    pos += 2
    transit = encodeddata[pos]*256 + encodeddata[pos+1]
    pos += 2
    listen = encodeddata[pos]*256 + encodeddata[pos+1]
    pos += 2
    parent = encodeddata[pos]*256 + encodeddata[pos+1]
    pos += 2
    parent_etx = encodeddata[pos]*256 + encodeddata[pos+1]
    pos += 2
    current_rtmetric = encodeddata[pos]*256 + encodeddata[pos+1]
    pos += 2
    num_neighbors = encodeddata[pos]*256 + encodeddata[pos+1]
    pos += 2
    beacon_interval = encodeddata[pos]*256 + encodeddata[pos+1]

    print "Data len (extracted) is ", collectdata_len


def process_response():
    host = ''
    rec_sock = socket.socket(socket.AF_INET6, socket.SOCK_DGRAM)
    rec_sock.bind((host, MNPORT))
    while(1):
        try:
            data, address = rec_sock.recvfrom(MAX)
        except KeyboardInterrupt:
            raise
        except:
            traceback.print_exc()
            continue
        collectd_dispatcher(address, data)
        code = str(len(data))+'B'
        s = struct.Struct(code)

        print "--------------------------------------"
        print s.unpack(data)

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
    send_request(hostname)
    process_response()
