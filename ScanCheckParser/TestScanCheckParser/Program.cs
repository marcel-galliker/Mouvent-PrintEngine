using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace TestScanCheckParser
{
    class Program
    {
        static void Main(string[] args)
        {
            var corrections = new List<ScanCheckImport.AlignmentCorrectionValues>();
            string errorMessage = "";
            bool result = ScanCheckImport.ImportManager.GetCorrectionValues(
                "D:\\Develop\\TestWPF\\ScanCheckParser\\Version1.xml",
                ref corrections, 
                ref errorMessage);
            Console.WriteLine(result == true ? "Success" : "Fail");
        }
    }
}
