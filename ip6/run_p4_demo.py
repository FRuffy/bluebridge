#!/usr/bin/env python2

# Copyright 2013-present Barefoot Networks, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

import os
from subprocess import Popen, PIPE
from functools import partial
from time import sleep

from mininet.net import Mininet
from mininet.topo import Topo
from mininet.log import setLogLevel, info
from mininet.cli import CLI
from mininet.term import makeTerm
from mininet.node import Host
from mininet.node import RemoteController
from p4_mininet.p4_mininet import P4Switch, P4Host


HOSTS = 6


class BlueBridgeTopo(Topo):

    def __init__(self, sw_path, json_path, thrift_port, pcap_dump, n, **opts):
        "Create custom topo."

        # Initialize topology
        Topo.__init__(self, **opts)

        switch = self.addSwitch('s1',
                                enable_debug=True,
                                sw_path=sw_path,
                                log_console=True,
                                json_path=json_path,
                                thrift_port=thrift_port,
                                pcap_dump=pcap_dump)
        # Create a network topology of a single switch
        # connected to three nodes.
        # +------s1------+
        # |      |       |
        # h1     h2      h3
        for h in range(n):
            host = self.addHost('h%d' % (h + 1),
                                ip="10.0.0.%d/24" % (h + 1),
                                mac='00:04:00:00:00:%02x' % h)
            self.addLink(host, switch)




def configureHosts(net):
    hostNum = 1
    hosts = net.hosts
    for host in hosts:
        print(host)
        for off in ["rx", "tx", "sg"]:
            cmd = "/sbin/ethtool --offload h" + str(hostNum) + "-eth0 %s off" % off
            host.cmdPrint(cmd)
        # Insert host configuration
        configString = "\"INTERFACE=" + \
            str(host) + \
            "-eth0\n\HOSTS=0:0:104::,0:0:105::,0:0:106::\n\SERVERPORT=5000\n\SRCPORT=0\n\SRCADDR=0:0:01" + \
            '{0:02x}'.format(hostNum) + "::\n\DEBUG=1\" > ./tmp/config/distMem.cnf"
        host.cmdPrint('echo ' + configString)

        # Configure the interface and respective routing
        host.cmdPrint('ip address change dev h' + str(hostNum) +
                      '-eth0 scope global 0:0:01' + '{0:02x}'.format(hostNum) + '::/48')
        host.cmdPrint('ip -6 route add local 0:0:0100::/40  dev h' +
                      str(hostNum) + '-eth0')
        # for j in range(HOSTS):
        #     host.cmdPrint('arp -s 10.0.0.%d 00:04:00:00:00:0%d' % (j + 1, j))
        # host.cmdPrint('ip -6 route add local 0:0:01' +
        #               '{0:02x}'.format(hostNum) + '::/48 dev lo')
        # Gotta get dem jumbo frames
        host.cmdPrint('ifconfig h' + str(hostNum) + '-eth0 mtu 9000')
        if 'h' in str(host) and hostNum > 3:
            # Run the server
            host.cmdPrint('xterm  -T \"server' + str(host)[1] +
                          '\" -e \"./apps/bin/server -c tmp/config/distMem.cnf; bash\" &')
            #host.cmdPrint('./apps/bin/server tmp/config/distMem.cnf &')
        if (hostNum == 2):
            host.cmdPrint('xterm  -T \"thrift' + str(host)[1] +
                          '\" -e \"./thrift/tutorial/c_glib/tutorial_remote_mem_test_server -c ./tmp/config/distMem.cnf; bash\" &')
        if (hostNum == 3):
            host.cmdPrint('xterm  -T \"thrift' + str(host)[1] +
                          '\" -e \"./thrift/tutorial/c_glib/tutorial_simple_array_comp_server -c ./tmp/config/distMem.cnf; bash\" &')

        hostNum += 1


def clean():
    ''' Clean any the running instances of POX '''
    Popen("killall xterm", stdout=PIPE, shell=True)
    # Popen("mn -c", stdout=PIPE, shell=True)


def main():

    privateDirs = [('./tmp/config', '/tmp/%(name)s/var/config')]
    heimdall = RemoteController('c', '0.0.0.0', 6633)
    # os.system("p4c-bmv2 --json p4_switch/ip6.json p4_switch/ip6.p4")
    host = partial(Host,
                   privateDirs=privateDirs)
    behavioral_simple = 'p4_switch/simple_switch/simple_switch'
    json_router = 'p4_switch/ip6_16.json'
    thrift_port = 9090
    topo = BlueBridgeTopo(behavioral_simple,
                          json_router,
                          thrift_port,
                          False,
                          HOSTS)
    net = Mininet(topo=topo,
                  host=host,
                  switch=P4Switch,
                  controller=heimdall)
    net.start()

    configureHosts(net)
    makeTerm(net.hosts[0])
    heimdall.cmdPrint('xterm  -T \"heimdall\" -e \"' +
                      'sudo ./p4_switch/heimdall/heimdall; bash" &')

    # makeTerm(net.hosts[1])

    # Our current "switch"
    i = 1
    while i <= HOSTS:
        # Routing entries per port
        # Gotta get dem jumbo frames
        os.system('ifconfig s1-eth' + str(i) + ' mtu 9000')
        i += 1
    os.system('p4_switch/simple_switch/simple_switch_CLI < p4_switch/commands.txt')
    CLI(net)
    net.stop()
    clean()


if __name__ == '__main__':
    setLogLevel('info')
    main()
