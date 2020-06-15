using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Xml;
using System.Xml.Linq;
using System.Xml.Schema;

namespace ScanCheckImport
{
    public class AlignmentCorrectionValues
    {
        public int InkSupplyNumber { get; set; }
        public List<double> AngleCorrection { get; set; }
        public List<double> StitchCorrection { get; set; }
        public List<double> RegisterCorrection { get; set; }
        public List<double> BidirectionalCorrection { get; set; }
        public double ColorOffsetCorrection { get; set; }
    }

    public class ImportManager
    {
        static public bool GetCorrectionValues(string xmlFilePath, ref List<AlignmentCorrectionValues> result, ref string errorMessage)
        {
            var path = new Uri(Path.GetDirectoryName(System.Reflection.Assembly.GetExecutingAssembly().CodeBase)).LocalPath;
            XmlSchemaSet schema = new XmlSchemaSet();
            schema.Add("", path + "\\ScanCheck.xsd");
            XmlReader rd = XmlReader.Create(xmlFilePath);
            XDocument doc = XDocument.Load(rd);
            try
            {
                doc.Validate(schema, ValidationEventHandler);
            }
            catch (Exception e)
            {
                errorMessage = e.Message;
                return false;
            }

            var correctionList = new List<AlignmentCorrectionValues>();

            foreach (var inkSupply in doc.Root.Elements("InkSupply"))
            {
                var alignmentCorrection = new AlignmentCorrectionValues();
                alignmentCorrection.InkSupplyNumber = Int32.Parse(inkSupply.Attribute("No").Value);
                var angleCorrectionString = inkSupply.Attribute("AngleCorrection").Value;
                var angleCorrectionList = angleCorrectionString.Split(' ');
                alignmentCorrection.AngleCorrection = new List<double>();
                foreach (var correction in angleCorrectionList)
                {
                    var value = correction.Replace(",", "."); // Make sure that the decimal point is a point
                    alignmentCorrection.AngleCorrection.Add(double.Parse(value, new NumberFormatInfo() { NumberDecimalSeparator = "." }));
                }
                var stitchCorrectionString = inkSupply.Attribute("StitchCorrection").Value;
                var stitchCorrectionList = stitchCorrectionString.Split(' ');
                alignmentCorrection.StitchCorrection = new List<double>();
                foreach (var correction in stitchCorrectionList)
                {
                    var value = correction.Replace(",", "."); // Make sure that the decimal point is a point
                    alignmentCorrection.StitchCorrection.Add(double.Parse(value, new NumberFormatInfo() { NumberDecimalSeparator = "." }));
                }
                var registerCorrectionString = inkSupply.Attribute("RegisterCorrection").Value;
                var registerCorrectionList = registerCorrectionString.Split(' ');
                alignmentCorrection.RegisterCorrection = new List<double>();
                foreach (var correction in registerCorrectionList)
                {
                    var value = correction.Replace(",", "."); // Make sure that the decimal point is a point
                    alignmentCorrection.RegisterCorrection.Add(double.Parse(value, new NumberFormatInfo() { NumberDecimalSeparator = "." }));
                }

                alignmentCorrection.BidirectionalCorrection = new List<double>();
                try
                {
                    var bidirectionalCorrectionString = inkSupply.Attribute("BidirectionalCorrection").Value;
                    var bidirectionalCorrectionList = bidirectionalCorrectionString.Split(' ');
                    
                    foreach (var correction in bidirectionalCorrectionList)
                    {
                        var value = correction.Replace(",", "."); // Make sure that the decimal point is a point
                        alignmentCorrection.BidirectionalCorrection.Add(double.Parse(value, new NumberFormatInfo() { NumberDecimalSeparator = "." }));
                    }
                }
                catch
                {
                    // not available for label machines
                }

                
                var colorOffset = inkSupply.Attribute("ColorOffsetCorrection").Value;
                alignmentCorrection.ColorOffsetCorrection = double.Parse(colorOffset);
                correctionList.Add(alignmentCorrection);
            }

            result = correctionList;
            return true;
        }

        static void ValidationEventHandler(object sender, ValidationEventArgs e)
        {
            XmlSeverityType type = XmlSeverityType.Warning;
            if (Enum.TryParse<XmlSeverityType>("Error", out type))
            {
                if (type == XmlSeverityType.Error) throw new Exception(e.Message);
            }
        }
    }
}
