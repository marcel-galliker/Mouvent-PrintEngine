"""
Testing a TX802 a 4 colors job 
"""
from util_gui import PrinterTests
import time


class TX(PrinterTests):
    scan = True    
    lenbox = "NumBox_ScanLen"

    def bmp_name(self, color, id, copies, width):
        def rd(x, y):
            return (x + y - 1) // y
        from collections import namedtuple
        BmpName = namedtuple("BmpName", ["name"])
        # compute the number of needed scans for the color and their position
        prw = (2048 - 128)
        n = self.colors.index(color)
        offset = int(self.coloroffset[n])
        board = (n * self.head) // 4
        fhead = (n * self.head) % 4
        start = offset // prw
        size = rd(offset - start * prw + width * copies, prw)
        scan_by_copy = rd(width, prw ) + len(self.colors)
        for scan in range(start + size, start, -1):
            yield BmpName(r"printed\fake id%d c%d p1 s%d b%d h%d.bmp" % (id, rd(scan, scan_by_copy) , rd(scan, self.head), board, fhead + (scan % self.head))) 

    def test_print(self):
        # wait the Print Queue tab is loaded and wait for it
        self.wait_loaded()
        jobs = [("Job1 Test170mm-J-0284", 2)] 
        self.print_and_check(jobs)


if __name__ == '__main__':
    import unittest
    unittest.main()