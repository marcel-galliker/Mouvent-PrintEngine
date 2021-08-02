#!/usr/bin/python3

import sys
import shutil
import os.path
import netifaces
import yaml
interfaces = netifaces.interfaces()

# list to rename 
change = {
      'em3': 'p1p1',
      'em4': 'p1p2',
      'em5': 'p1p3',
      'em6': 'p1p4',
      'eno1': 'em1',
      'eno2': 'em2',
      'eno3': 'p1p1',
      'eno4': 'p1p2',
      'eno5': 'p1p3',
      'eno6': 'p1p4',
      'em49': 'p1p1',
      'em50': 'p1p2',
      'em51': 'p1p3',
      'em52': 'p1p4',
      'ens2f0': 'p2p1',
      'ens2f1': 'p2p2',
      'ens2f2': 'p2p3',
      'ens2f3': 'p2p4',
}
# add values as keys to themselve
v=list(change.values())
change.update(zip(v,v))

# read the netplan config to solve the mtu bug
eth_path = '/etc/netplan/01-eth-config.yaml'
ethconf = None
network = {}
if os.path.isfile(eth_path):
      with open(eth_path) as fe:
            ethconf = yaml.load(fe.read(), Loader=yaml.Loader)
            network = ethconf["network"]["ethernets"]
else:
      print("WARNING: No file '{0}' to modify".format(eth_path))

for interface in interfaces:
      print (interface)
      mac = netifaces.ifaddresses(interface)[netifaces.AF_LINK][0]['addr']
      if interface in change:
            newi = change[interface]
            print("Changed {0} to {1}".format(interface, newi))
            # add the match by macaddress as workaround on bug on mtu
            if newi in network:
                  network[newi]["match"] = {"macaddress": mac}
      else:
            print('-> Interface not changed')
if ethconf:
      # write the new netplan configuration
      open(eth_path,"wt").write(yaml.dump(ethconf))
