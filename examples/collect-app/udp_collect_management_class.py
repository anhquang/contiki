#!/usr/bin/env python

#TODO: replace print by logging

import random, socket, sys
import struct
import traceback

COLLECTD_MANG_PORT = 10000
COLLECTD_CLIENT_PORT = 8775
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

class collectd_recv_pk(object):
    def __init__(self):
        #all value is set to zero
        self.seqno = \
        self.len = self.clock = self.timesynch_time = self.cpu = \
        self.lpm = self.transit = self.listen = self.parent = \
        self.parent_etx = self.current_rtmetric = self.num_neighbors = \
        self.beacon_interval = 0

        self.sensors = []
        for i in range(0, MAX_NUM_SENSOR):
            self.sensors.append(0)

    def dispatcher(self, address, payload):
        DEFAULT_LEN_PAYLOAD = 48
        DEFAULT_COLLECTD_DATA_ITEM = 22

        if len(payload) <> DEFAULT_LEN_PAYLOAD:
            print 'Received payload does not have the right len for collectd protocol'
            return ERROR

        extractedcode = '!' + '4B' + '22H'
        extracteddata = struct.unpack(extractedcode, payload)

        print 'extracted data: ', extracteddata

        pos = 0
        if extracteddata[pos] <> VERSION_01:
            print "ERROR, different version"
            return ERROR

        pos += 1
        id = extracteddata[pos]
        #ignore id
        pos += 1
        type = extracteddata[pos]
        if type <> RESPONSE:
            print "ERROR, not a response msg"
            return ERROR

        pos += 1
        seqno = extracteddata[pos]
        #ignore sequence number

        pos += 1
        collect_item_len = extracteddata[pos]
        if collect_item_len <> DEFAULT_COLLECTD_DATA_ITEM:
            print "ERROR, corrupted msg (based on len)"
            return ERROR
        pos += 1

        temitem = [extracteddata[pos+i] for i in range(0,11)]

        self.clock, self.timesynch_time, \
        self.cpu, self.lpm, self.transit, \
        self.listen, self.parent, \
        self.parent_etx, self.current_rtmetric, \
        self.num_neighbors, self.beacon_interval = temitem

        pos += 11

        for i in range(MAX_NUM_SENSOR):
            self.sensors[i] = extracteddata[pos+i]


    def __repr__(self):
        return "clock %d, timesynch %d, cpu %d, lpm %d, transit %d,\
                listen %d, parent %d, parent_etx %d, current_rtmetric %d, \
                num_neighbors %d, beacon_interval %d" %(
                    self.clock, self.timesynch_time, \
                    self.cpu, self.lpm, self.transit, \
                    self.listen, self.parent, \
                    self.parent_etx, self.current_rtmetric, \
                    self.num_neighbors, self.beacon_interval) \
                + \
                ", sensors = %s" %self.sensors


class collectd_req_pk(object):
    def reqs_sock_create(self, hostname):
        s = socket.socket(socket.AF_INET6, socket.SOCK_DGRAM)
        try:
            s.connect((hostname, COLLECTD_CLIENT_PORT))
        except:
            raise RuntimeError('Host %s is unreachable' %hostname)

        print 'Client socket name is', s.getsockname()
        return s

    def reqs_start(self, hostname):
        data = [VERSION_01, 20, REQUEST, COMMAND_START, COLLECTD_MANG_PORT]
        packet = struct.Struct('!4B H')
        packed_data = packet.pack(*data)

        s = self.reqs_sock_create(hostname)
        s.send(packed_data)
        #print 'Waiting up to', delay, 'seconds for a reply'
        #s.settimeout(delay)

    def reqs_stop(self, hostname):
        data = [VERSION_01, 20, REQUEST, COMMAND_STOP, COLLECTD_MANG_PORT]
        packet = struct.Struct('!4B H')
        packed_data = packet.pack(*data)

        s = self.reqs_sock_create(hostname)
        s.send(packed_data)
        #print 'Waiting up to', delay, 'seconds for a reply'
        #s.settimeout(delay)


def process_response():
    rec_sock = socket.socket(socket.AF_INET6, socket.SOCK_DGRAM)
    rec_sock.bind(('', COLLECTD_MANG_PORT))
    while(1):
        try:
            data, address = rec_sock.recvfrom(MAX)
        except KeyboardInterrupt:
            raise
        except:
            traceback.print_exc()
            continue
        recv_pk_process = collectd_recv_pk()
        recv_pk_process.dispatcher(address, data)
        print recv_pk_process

def send_request(hostname = None):
    if not hostname:
        hostname = 'aaaa::212:7402:2:202'

    sendreq = collectd_req_pk()
    sendreq.reqs_start(hostname)

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
