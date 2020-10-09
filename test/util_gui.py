"""
Utility functions and main classes for testing GUI
"""
import unittest, os, subprocess, time, shutil
from pathlib import Path
import xml.etree.ElementTree as ET

from appium import webdriver
from selenium.webdriver.support.wait import WebDriverWait, TimeoutException
from selenium.webdriver.support import expected_conditions
from selenium.webdriver.common.keys import Keys
from selenium.webdriver.common.action_chains import ActionChains
from selenium.common.exceptions import NoSuchElementException

from PIL import Image
Image.MAX_IMAGE_PIXELS = None

from head_ctrl_mock import head_ctrl


class PrinterTests(unittest.TestCase):
    "Master abstract class for printer tests"
    driver = None
    head_ctrl = None
    main_ctrl = None
    spooler_ctrl = None
    print_id = 0
    @classmethod
    def setUpClass(self):
        # save config
        d = Path("D:/radex/user/")
        if d.exists():
            shutil.copy2("D:/radex/user/config.cfg", "D:/radex/user/config.cfg.ori")
            shutil.copy2("D:/radex/user/user.config", "D:/radex/user/user.config.ori")
            if os.path.exists("D:/radex/user/network.cfg"):
                os.replace("D:/radex/user/network.cfg", "D:/radex/user/network.cfg.ori")
        else:
            d.mkdir(parents=True)
        Path("D:/ripped-data").mkdir(exist_ok=True)
        #copy config
        shutil.copy2("test/conf/%s/config.cfg" % self.__name__, "D:/radex/user")
        shutil.copy2("test/conf/user.config", "D:/radex/user")
        shutil.copy2("test/conf/network.cfg", "D:/radex/user")
        shutil.copytree("test/conf/waveform", "D:/radex/waveform", dirs_exist_ok=True)
        self.start()

    @classmethod 
    def start(self):
        # hack to ensure we stop all thread if unexpected exception
        import sys
        def excepthook(type, value, traceback):
            print(80*"*")
            sys.excepthook = sys.__excepthook__
            self.tearDownClass()
            sys.__excepthook__(type, value, traceback)
        sys.excepthook = excepthook

        # remove queue to ensure nothing to print
        if os.path.exists("D:/radex/user/print_queue.xml"):
            os.replace("D:/radex/user/print_queue.xml", "D:/radex/user/print_queue.xml.ori")

        #set up appium and start GUI
        desired_caps = { "app" : os.path.join(os.getcwd(), r"bin\gui\RX_DigiPrint.exe"),
                          "platformName" : 'Windows',
                           "createSessionTimeout" : 20000}
        self.driver = webdriver.Remote(command_executor='http://127.0.0.1:4723', desired_capabilities= desired_caps)

        # read printer configuration (colors, heads by color...)
        self.read_configuration()

        # start ctrl in simulation
        # note that the debug version of rx_main should be compile with SUBNET="127.168.200."
        self.main_ctrl = subprocess.Popen([r"_out\Debug_x64\rx_main_ctrl.exe", "-simuplc", "-simuencoder","-simuchiller"],
                                         stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        time.sleep(1) # wait before starting spooler the the main is started
        self.spool_ctrl = subprocess.Popen([r"bin\win\rx_spooler_ctrl.exe"], stdout=subprocess.DEVNULL,
                                        stderr=subprocess.DEVNULL)

        # removed old printed bmp
        shutil.rmtree("printed", True)
        # start the head ctrl mock in another thread
        import threading, logging
        logging.basicConfig(level=logging.FATAL) # silently
        self.head_ctrl = threading.Thread(target=head_ctrl.run)
        self.head_ctrl.start()

    @classmethod
    def read_configuration(self):
        # read printer configuration
        root = ET.parse("D:/radex/user/config.cfg").getroot()
        self.coloroffset = []
        self.head = int(root.attrib["HeadsPerColor"])
        self.colors = []
        self.printer = root.find("Printer").attrib["type"]
        convcol = {"Black": "K", "Cyan": "C", "Magenta": "M", "Yellow": "Y"}
        for ink in root.iter('InkSupply'):
            if ink.attrib["InkName"]:
                for c in convcol:
                    if c in ink.attrib["InkName"]:
                        self.colors.append(convcol[c])
                        break
                else:
                    self.assertTrue(False, "color unknown " + ink.attrib["InkName"])
            else:
                self.colors.append(None)
            self.coloroffset.append(int(ink.attrib["ColorOffset"]) * 1200 / 25400) # ofset in pixels for each color

    @classmethod
    def tearDownClass(self):
        if self.driver:
            self.driver.quit()
        # stop the mock
        head_ctrl.simulate = False
        if self.head_ctrl:
            self.head_ctrl.join(5.0)
        # and processes
        if self.main_ctrl:
            self.main_ctrl.terminate()
            self.main_ctrl.wait()
        if self.spool_ctrl:
            self.spool_ctrl.terminate()
            self.spool_ctrl.wait()

        # restore original configuration
        if os.path.exists("D:/radex/user/config.cfg.ori"):
            os.replace("D:/radex/user/config.cfg.ori", "D:/radex/user/config.cfg")
            os.replace("D:/radex/user/network.cfg.ori", "D:/radex/user/network.cfg")
            os.replace("D:/radex/user/user.config.ori", "D:/radex/user/user.config")
            if os.path.exists("D:/radex/user/print_queue.xml.ori"):
                os.replace("D:/radex/user/print_queue.xml.ori", "D:/radex/user/print_queue.xml")

    @classmethod
    def name(self):
        return self.__name__

    def tearDown(self):
        "save screenshot and source code on failure"
        result = self.defaultTestResult()
        self._feedErrorsToResult(result, self._outcome.errors)
        error = result.errors
        failure = result.failures
        if error or failure:
            fn = "d:/radex/log/test_"+self.name()+time.strftime("%Y%m%d-%H%M%S")
            print(fn+" saved")
            self.driver.save_screenshot(fn+".png")
            with open(fn+".xml","wt") as fp:
                fp.write(self.driver.page_source)

    def wait_loaded(self):
        "Wait for Print Queue tab is loaded and wait"
        WebDriverWait(self.driver, 20).until(lambda x:x.find_element_by_name("Print Queue").is_displayed())

    def enter_value(self, elt, val):
        "enter a value in a text box using the calculator popup"
        # we should also click on enter on the calc to activate Save Button
        elt.click()
        windows=self.driver.window_handles
        self.driver.switch_to.window(windows[0])
        self.driver.find_element_by_name("0").send_keys(str(val))
        self.driver.find_element_by_name("Enter").click()
        self.driver.switch_to.window(windows[1])

    def get_counters(self):
        "return the 2 counter (actual, total) as float"
        actual = float(self.driver.find_element_by_xpath("//Text[@AutomationId='CounterUnit1']/following-sibling::Text").text.replace(",",".").replace(" ",""))
        total = float(self.driver.find_element_by_xpath("//Text[@AutomationId='CounterUnit2']/following-sibling::Text").text.replace(",",".").replace(" ",""))
        return (actual, total)

    def reset_counter(self):
        "reset actual counter"
        cnt = WebDriverWait(self.driver, 15).until(expected_conditions.element_to_be_clickable(("accessibility id", "CounterUnit2")))
        cnt.click()
        self.accept_all()

    def select_job(self, job):
        # click on the element does not work (pb with XAMGrid), so we got it with arrows
        g = WebDriverWait(self.driver, 5).until(expected_conditions.visibility_of_element_located(("name", "DirGrid")))
        jobpane = None
        previous = None
        while jobpane is None: # to display all existing jobs
            g.send_keys(Keys.PAGE_DOWN) 
            try:
                jobpane = g.find_element_by_xpath("//DataItem[@Name='"+job+"']/parent::*")
            except NoSuchElementException:
                first = g.find_element_by_class_name("CellsPanel").get_attribute("Name") 
                if first == previous:
                    raise
                else:
                    previous = first 

        g.send_keys(2 * Keys.HOME + int(jobpane.get_attribute("Name")) * Keys.ARROW_DOWN + Keys.ARROW_RIGHT + Keys.ARROW_LEFT)
        jobpane.find_element_by_class_name("Button").click()
    
    lenbox = "LengthBox"
    def add_job(self, job, length=None):
        "add an existing job to the queue"
        # do not copy the job to test, it should be prensent on the machine
        # shutil.copytree("test/conf/jobs/" + job, "D:/ripped-data/" + job, dirs_exist_ok=True)
        self.assertTrue(os.path.exists("D:/ripped-data/" + job))
        self.driver.find_element_by_name("Print Queue").click()
        WebDriverWait(self.driver, 5).until(expected_conditions.element_to_be_clickable(("accessibility id", "Button_Add"))).click()
        self.select_job(job)
        
        if length is not None:
            box = WebDriverWait(self.driver, 5).until(expected_conditions.element_to_be_clickable(("accessibility id", self.lenbox)))
            self.enter_value(box, length)
            # ensure we are in copy mode
            f = self.driver.find_element_by_name("copies")
            if f.is_displayed():
                f.click()

        # on textile select bi-directionnal scan
        if self.scan:
            self.driver.find_element_by_accessibility_id("CB_ScanMode").click()
            self.driver.find_element_by_name("<-->").click()

        # then save if needed
        # TODO change the UI to acces the element by name and not by ugly relative position!
        # note than XamGrid with template is not accesible for automation (?)
        # so we look for the new GUI button by offset on the left of the name of the job
        pane = self.driver.find_element_by_name("PrintQueueGrid").find_element_by_name(job)
        ActionChains(self.driver).move_to_element_with_offset(pane, -100, 20).click().perform() 

        # add 1 to the id (could not retrieve it from GUI as it is a XamGrid)
        self.print_id += 1
        return self.print_id

    def start_print(self):
        self.driver.find_element_by_accessibility_id("Button_Start").click()
        self.accept_all()

    def accept_all(self):
        # click Yes on different questions regarding Ink and UV... to start printing
        windows = self.driver.window_handles
        while len(windows)>1:
            self.driver.switch_to.window(windows[0])
            self.driver.find_element_by_accessibility_id("Button_Yes").click()
            windows = self.driver.window_handles
        self.driver.switch_to.window(windows[0])

    def wait_printed(self):
        "wait for print to finish"
        while self.driver.find_element_by_name("PrintedQueueGrid").find_element_by_class_name("ProgressBar").text != "100":
            WebDriverWait(self.driver.find_element_by_name("PrintedQueueGrid")
                .find_element_by_class_name("ProgressBar"), 120).until(lambda x:x.text == "100")

    def bmp_name(self, color, id, copy, width):
        "return the name of the bmp simulated files"
        self.assertTrue(False, "bmp name missing for printer " + self.name())

    scan = False
    def assert_bmp(self, job, id, copy):
        "compare simulated bmp to TIF and return the size of the job in meter"
        # compare file of different color
        for color in self.colors:
            if not color: # only if color is used
                continue
            # get the originbal ripped TIF file
            with Image.open(r"D:\ripped-data\%s\%s_%s.tif" % (job, job, color)) as ori:
                # compute the printed size in inches
                dpi = ori.info['dpi']
                if not self.scan: # LX or TX does not print in the same direction
                    dist = copy * ori.size[1]/dpi[1]
                else:
                    dist = (copy * ori.size[0] + max(self.coloroffset)) /dpi[0]

                nbcopy = copy
                end = ori.width
                images = []
                used = False
                # retreive all bmp files send to heads in reverse order (start from the right of the image)
                for bmp in self.bmp_name(color, id, copy, ori.width):
                    # for the next copy we should restart for the end
                    if  (not self.scan and bmp.newcopy):
                        end = ori.width
                        used = False
                    if not os.path.exists(bmp.name):
                        import time
                        time.sleep(5) # wait for thread to finish
                    im = Image.open(bmp.name)
                    if im.size != (1, 1): 
                        used = True
                        if self.scan:
                            if im.width < 128:
                                im.close()
                                continue
                            # on textile scan has space on borders
                            wakeup = (len(self.colors)+1)*128
                            im = im.crop((128, wakeup, im.width, im.height-wakeup))

                    if not used:
                        im.close()
                    else:
                        images.append(im)
                        while images:
                            im = images.pop()
                            # not out the original image
                            if end > 0:
                                # crop the original image to the size of the bmp send to the head
                                start = end - im.width
                                # the original image is only part of this bmp (left of the image)
                                if start < 0:
                                    ie = im.crop((0, 0, -start, im.height))
                                    # it should be blank for LB 
                                    if not self.scan or nbcopy == 1:
                                        self.assertEqual(ie.histogram()[-1], ie.width * ie.height, "Not only white in crop: "+bmp.name)
                                    else: # or the end of the previous copy on TX
                                        images.append(ie)
                                    # take only the part of the image that is in the current copy
                                    im = im.crop((-start, 0, im.width, im.height))
                                    start = 0

                                # check same drop size between the part of the tif and bmp
                                imo = ori.crop((start, 0, end, ori.height))
                                try:
                                    self.assertEqual(imo.tobytes(), im.tobytes(), "BMP not equal to TIF: "+bmp.name)
                                except AssertionError:
                                    # save differences for inspection
                                    expbmp = bmp.name.replace(".bmp",".print.bmp")
                                    im.save(expbmp)
                                    expbmp = bmp.name.replace(".bmp",".exp.bmp")
                                    imo.save(expbmp)
                                    print("file %s saved" % (expbmp,))
                                    raise

                                im.close()
                                # Head size 2048, overlap 128 on Label
                                if self.scan or start == 0:
                                    width = im.width
                                else:
                                    width = im.width - 128
                                end = end - min(2048, width)
                                # start a new copy
                                if end == 0 and self.scan:
                                    end = ori.width
                                    nbcopy -= 1
                            else: # outside the original TIF, should only be blank
                                self.assertEqual(im.histogram()[-1], im.width * im.height, "Not only white for unused head: "+bmp.name)

        # return the distance printed in meters
        return dist * 0.0254

    def print_and_check(self, jobs):
        self.reset_counter()
        ids=[]
        for job in jobs:
            ids.append(self.add_job(*job))
        self.start_print()
        self.wait_printed()
        # check printing
        actual = self.get_counters()[0]
        dist = 0
        for (i, id) in enumerate(ids):
            dist += self.assert_bmp(jobs[i][0], id, jobs[i][1])
        self.assertAlmostEqual(actual, dist, delta=0.1, msg="counter not updated")


