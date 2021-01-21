"""
    Simulate a board to boot
"""
import asyncio, functools
import logging
import struct
import os
import xml.etree.ElementTree as ET

import message_mgr
import network

# to end the infinite loop
async def main(devtype, nb):
    "main program that listen to all UDP/TCP sockets according to network.cfg"
    # 4 steppers
    boards=[]
    for n in range(nb):
        board = {}
        board["MacAddr"] = f"{devices[devtype]}{n:02x}"
        board["DevNo"] = n
        board["SerialNo"] = f"{n}"
        board["DevTypeStr"] = devtype
        boards.append(board)
    n = network.Network()
    await n.boot(boards)
    while True:
        await asyncio.sleep(3)
    

devices = {
    "Head":"00-00-00-01-00-",
    "Encoder":"00-00-00-04-00-",
    "Fluid":"00-00-00-05-00-",
    "Stepper":"00-00-00-06-00-",
    "Encoder32":"00-00-00-07-00-"
    }

def run(devtype, n):
    "run a head ctrl asynchronously by calling main in asyncio loop"
    loop = asyncio.new_event_loop()
    asyncio.set_event_loop(loop)
    try:
        loop.run_until_complete(main(devtype, n))
    finally:
        for transport in network.all_transports:
            transport.close()
        network.all_transports= []
        loop.run_until_complete(loop.shutdown_asyncgens())
        loop.close()
    

if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--log", help='log level (default INFO)', default="INFO", choices=['DEBUG', 'INFO', 'WARN', 'ERROR'])
    parser.add_argument("--src", help='root source folder where to find header')
    parser.add_argument("device", help='device to boot', choices=devices.keys())
    parser.add_argument("number", help='number of boards to simulate', type=int)
    args = parser.parse_args()
    numeric_level = getattr(logging, args.log.upper(), None)

    if args.src:
        Message.src_path = args.src

    if not isinstance(numeric_level, int):
        raise ValueError('Invalid log level: %s' % args.log)
    logging.basicConfig(format='%(asctime)s %(levelname)s:%(message)s',
                        level=numeric_level)

    try:
        run(args.device, args.number)
    except KeyboardInterrupt:
        # ignore Ctrl+C silently
        pass

