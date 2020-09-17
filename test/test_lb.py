"""
Testing a LB701 job
"""
from util_gui import PrinterTests
import time

class LB(PrinterTests):
    def bmp_name(self, color, id, copies, width):
        from collections import namedtuple
        BmpName = namedtuple("BmpName", ["name", "newcopy"])
        board = self.colors.index(color)
        for copy in range(copies):
            newcopy = True
            for head in range(self.head):
                h = self.head - head - 1
                yield BmpName(r"printed\fake id%d c%d p1 s1 h%d b%d.bmp" % (id, copy+1, h % 4, board + h // 4), newcopy)
                newcopy = False

    def test_print(self):
        jobs = [("Job1 Test170mm-J-0284", 2)] 
        #jobs = [("FQC_OVCMYK_340mm_V2-J-0180", 2)]
        self.print_and_check(jobs)

if __name__ == '__main__':
    import unittest
    unittest.main()