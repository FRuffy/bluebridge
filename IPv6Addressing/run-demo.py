"""Custom topology example

One switch and 16 hosts:

   host --- switch --- switch --- host

Adding the 'topos' dict with a key/value pair to generate our newly defined
topology enables one to pass in '--topo=mytopo' from the command line.
"""
from mininet.node import CPULimitedHost
from mininet.topo import Topo
from mininet.net import Mininet
from mininet.log import setLogLevel, info
from mininet.node import RemoteController
from mininet.cli import CLI
from mininet.link import TCLink
from mininet.topolib import TreeNet

import time
from mininet.node import Host
from functools import partial


class BlueBridge(Topo):
    "Simple topology example."

    def __init__(self):
        "Create custom topo."

        # Initialize topology
        Topo.__init__(self)

        switch = self.addSwitch('s1')

        for hostNum in range(1, 4):
            # Add hosts and switches
            host = self.addHost('h' + str(hostNum))
            self.addLink(host, switch)


topos = {'BlueBridge': (lambda: BlueBridge())}


def run():
    privateDirs = [('./config', '/tmp/%(name)s/var/config')]
    # c = RemoteController('c', '0.0.0.0', 5555)

    host = partial(Host,
                   privateDirs=privateDirs)
    topo = BlueBridge()

    # controller is used by s1-s3
    net = Mininet(topo=topo, host=host, build=False)
    # net = TreeNet(depth=1, fanout=3, host=host, controller=RemoteController)
    # net.addController(c)
    net.build()
    net.start()
    directories = [directory[0] if isinstance(directory, tuple)
                   else directory for directory in privateDirs]
    info('Private Directories:', directories, '\n')
    hosts = net.hosts
    hostNum = 1
    for host in hosts:
        print host
        # switch to host.config later
        # ipstring = 'h' + str(hostNum) +'-eth0 inet6 add 0:0:01' + '{0:02x}'.format(hostNum) + '::/46'
        # host.config(ip=ipstring)

        # ip -6 neigh add proxy 2001:0DB8:A::2 dev eth0
        # host.cmdPrint('ip -6 route add 0:0:0100::/40  via '+ host.IP)
        # xterm -e bash -c
        testString = "\"proxy h" + str(hostNum) + "-eth0 { ttl 5000 router no rule 0:0:01" + '{0:02x}'.format(
            hostNum) + "::/48 { static } }\" > ./config/ndp_conf.conf"
        print testString

        testString2 = "\"interface h" + str(hostNum) + "-eth0 {\n AdvSendAdvert on;\n MinRtrAdvInterval 3;\n MaxRtrAdvInterval 10;\n prefix 0:0:01" + '{0:02x}'.format(
            hostNum) + "::/48 {\n AdvOnLink on;\n AdvAutonomous on;\n AdvRouterAddr on;\n };\n };\" > ./config/radvd.conf"
        print testString2

        host.cmdPrint('echo ' + testString)
        host.cmdPrint('echo ' + testString2)
        host.cmdPrint('ip address change dev h' + str(hostNum) +
                      '-eth0 scope global 0:0:01' + '{0:02x}'.format(hostNum) + '::/48')
        host.cmdPrint('ip -6 route add local 0:0:0100::/40  dev h' +
                      str(hostNum) + '-eth0')
        host.cmdPrint('ip -6 route add local 0:0:01' + '{0:02x}'.format(hostNum) + '::/48 dev lo')
        host.cmdPrint('xterm  -T \"server' + str(hostNum) +
                      '\" -e \"./messaging/bin/server; bash\" &')
        # host.cmdPrint('xterm  -T \"ndpproxy' + str(hostNum) + '\" -e \"valgrind ./messaging/bin/ndpproxy -i h' + str(hostNum) +
        #                '-eth0 0:0:01' + "{0:02x}".format(hostNum) + '::/48; bash\" &')
        # host.cmdPrint('xterm  -T \"ndpproxy' + str(hostNum) + '\" -e \"./messaging/launchProxy.sh -i h' + str(hostNum) +
        #               '-eth0 0:0:01' + "{0:02x}".format(hostNum) + '::/48; bash\" &')
        host.cmdPrint('xterm  -T \"ndpproxy' + str(hostNum) +
                      '\" -e \"./ndpproxy/ndppd -vvv -c ./config/ndp_conf.conf; bash\" &')
        # host.cmdPrint('xterm  -T \"ndpproxy' + str(hostNum) +
        #               '\" -e \"radvd -n --debug=5 -C ./config/radvd.conf; bash\" &')
        hostNum += 1

    net.startTerms()
    CLI(net)

    net.stop()

if __name__ == '__main__':
    setLogLevel('info')
    run()
