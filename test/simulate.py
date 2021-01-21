from pathlib import Path
import os, subprocess, shutil, time

from mock import head_ctrl, fluid_ctrl, network, message_mgr, rest

Path("D:/ripped-data").mkdir(exist_ok=True)
Path("D:/radex/user").mkdir(exist_ok=True)
#copy config the first time
if not os.path.exists("D:/radex/user/config.cfg"):
    shutil.copy2("test/conf/LB/config.cfg", "D:/radex/user")
    shutil.copy2("test/conf/user.config", "D:/radex/user")
    shutil.copy2("test/conf/network.cfg", "D:/radex/user")
shutil.copytree("test/conf/waveform", "D:/radex/waveform", dirs_exist_ok=True)

gui = subprocess.Popen([r"bin\gui\RX_DigiPrint.exe"],
                                    stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
# start ctrl in simulation
main_ctrl = subprocess.Popen([r"bin\win\rx_main_ctrl.exe", "-debug", "-localsubnet", "-simuplc", "-simuencoder","-simuchiller"])
time.sleep(1) # wait before starting spooler the the main is started
spool_ctrl = subprocess.Popen([r"bin\win\rx_spooler_ctrl.exe"], stdout=subprocess.DEVNULL,
                                stderr=subprocess.DEVNULL)

simulation = network.Network()
simulation.register("Fluid", fluid_ctrl)
simulation.register("Head", head_ctrl)
try:
    simulation.run()
except KeyboardInterrupt:
    # ignore Ctrl+C silently
    pass
finally: 
    simulation.simulate = False
    # and processes
    gui.terminate()
    gui.wait()
    spool_ctrl.terminate()
    spool_ctrl.wait()
    main_ctrl.terminate()
    main_ctrl.wait()
        