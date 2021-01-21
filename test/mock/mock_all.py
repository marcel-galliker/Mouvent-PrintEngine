import network
import fluid_ctrl
import head_ctrl
import logging

import argparse
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("--log", help='log level (default INFO)', default="INFO", choices=['DEBUG', 'INFO', 'WARN', 'ERROR'])
parser.add_argument("--src", help='root source folder where to find header')
args = parser.parse_args()
numeric_level = getattr(logging, args.log.upper(), None)

if args.src:
    network.Message.src_path = args.src

if not isinstance(numeric_level, int):
    raise ValueError('Invalid log level: %s' % args.log)
logging.basicConfig(format='%(asctime)s %(levelname)s:%(message)s',
                    level=numeric_level)

simulation = network.Network()
simulation.register("Fluid", fluid_ctrl)
simulation.register("Head", head_ctrl)
try:
    simulation.run()
except KeyboardInterrupt:
    # ignore Ctrl+C silently
    pass