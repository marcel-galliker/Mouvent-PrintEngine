#!/usr/bin/python3

import sys
import shutil
import os.path
import netifaces
import yaml

# list of main interfaces
main_interface = {
      'eno1': 'em1',
      'eno2': 'em2',
      'lo': 'lo',    
}
# add values as keys to themselve to be able to relaunch .py
v=list(main_interface.values())
main_interface.update(zip(v,v))

# read the netplan config for ip address setting
eth_path = '/etc/netplan/01-eth-config.yaml'
ethconf = yaml.load(open(eth_path, "rt").read(), Loader=yaml.Loader)
network = ethconf["network"]["ethernets"]

# rename interfaces by adding the mac adress in the yaml file of netplan configuration
# p1p1 to p2p4 interfaces for UDP are the other interfaces in the list
card=1
port=1
for interface in netifaces.interfaces():
      print (interface)
      # "main" inteface
      if interface in main_interface:
            newi = main_interface[interface]
      else: # or "pxpx" UDP
            newi = f"p{card}p{port}"
            port += 1
            # there are only 4 ports by card
            if port == 5:
                  port = 1
                  card += 1
      mac = netifaces.ifaddresses(interface)[netifaces.AF_LINK][0]['addr']
      if newi in network:
            print("Changed {0} to {1}".format(interface, newi))
            network[newi]["match"] = {"macaddress": mac}

# and remove not existing interfaces from netplan
for interface in list(network.keys()):
      if "match" not in network[interface]:
            del network[interface]

# write the new netplan configuration
open(eth_path,"wt").write(yaml.dump(ethconf))
