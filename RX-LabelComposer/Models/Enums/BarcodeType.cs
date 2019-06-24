
using RX_Common;
using System.Collections;
using System.Collections.Generic;
using System.Globalization;


namespace RX_LabelComposer.Models.Enums
{
    public enum BcTypeEnum : int
    {
        eBC_Code11 = 1,
        eBC_2OF5 = 2,
        eBC_2OF5IL = 3,
        eBC_2OF5IATA = 4,
        eBC_2OF5M = 5,
        eBC_2OF5DL = 6,
        eBC_2OF5IND = 7,
        eBC_3OF9 = 8,
        eBC_3OF9A = 9,
        eBC_EAN8 = 10,
        eBC_EAN8P2 = 11,
        eBC_EAN8P5 = 12,
        eBC_EAN13 = 13,
        eBC_EAN13P2 = 14,
        eBC_EAN13P5 = 15,
        eBC_EAN128 = 16,
        eBC_UPC12 = 17,
        eBC_CodaBar2 = 18,
        eBC_CodaBar18 = 19,
        eBC_Code128 = 20,
        eBC_DPLeit = 21,
        eBC_DPIdent = 22,
        eBC_ISBN13P5 = 23,
        eBC_ISMN = 24,
        eBC_9OF3 = 25,
        eBC_ISSN = 26,
        eBC_ISSNP2 = 27,
        eBC_Flattermarken = 28,
        eBC_GS1DataBar = 29,
        eBC_GS1DataBarLtd = 30,
        eBC_GS1DataBarExp = 31,
        eBC_TelepenAlpha = 32,
        eBC_UCC128 = 33,
        eBC_UPCA = 34,
        eBC_UPCAP2 = 35,
        eBC_UPCAP5 = 36,
        eBC_UPCE = 37,
        eBC_UPCEP2 = 38,
        eBC_UPCEP5 = 39,
        eBC_USPSPostNet5 = 40,
        eBC_USPSPostNet6 = 41,
        eBC_USPSPostNet9 = 42,
        eBC_USPSPostNet10 = 43,
        eBC_USPSPostNet11 = 44,
        eBC_USPSPostNet12 = 45,
        eBC_Plessey = 46,
        eBC_MSI = 47,
        eBC_SSCC18 = 48,
        eBC_FIM = 49,
        eBC_LOGMARS = 50,
        eBC_Pharma1 = 51,
        eBC_PZN7 = 52,
        eBC_Pharma2 = 53,
        eBC_CEPNet = 54,
        eBC_PDF417 = 55,
        eBC_PDF417Trunc = 56,
        eBC_MAXICODE = 57,
        eBC_QRCode = 58,
        eBC_Code128A = 59,
        eBC_Code128B = 60,
        eBC_Code128C = 61,
        eBC_9OF3A = 62,
        eBC_AusPostCustom = 63,
        eBC_AusPostCustom2 = 64,
        eBC_AusPostCustom3 = 65,
        eBC_AusPostReplyPaid = 66,
        eBC_AusPostRouting = 67,
        eBC_AusPostRedirect = 68,
        eBC_ISBN13 = 69,
        eBC_RM4SCC = 70,
        eBC_DataMatrix = 71,
        eBC_EAN14 = 72,
        eBC_VIN = 73,
        eBC_CODABLOCK_F = 74,
        eBC_NVE18 = 75,
        eBC_JapanesePostal = 76,
        eBC_KoreanPostalAuth = 77,
        eBC_GS1DataBarTrunc = 78,
        eBC_GS1DataBarStacked = 79,
        eBC_GS1DataBarStackedOmni = 80,
        eBC_GS1DataBarExpStacked = 81,
        eBC_Planet12 = 82,
        eBC_Planet14 = 83,
        eBC_MicroPDF417 = 84,
        eBC_USPSIntelligentMail = 85,
        eBC_PlesseyBidir = 86,
        eBC_Telepen = 87,
        eBC_GS1_128 = 88,
        eBC_ITF14 = 89,
        eBC_KIX = 90,
        eBC_Code32 = 91,
        eBC_Aztec = 92,
        eBC_DAFT = 93,
        eBC_ItalianPostal2Of5 = 94,
        eBC_ItalianPostal3Of9 = 95,
        eBC_DPD = 96,
        eBC_MicroQRCode = 97,
        eBC_HIBCLic128 = 98,
        eBC_HIBCLic3OF9 = 99,
        eBC_HIBCPas128 = 100,
        eBC_HIBCPas3OF9 = 101,
        eBC_HIBCLicDataMatrix = 102,
        eBC_HIBCPasDataMatrix = 103,
        eBC_HIBCLicQRCode = 104,
        eBC_HIBCPasQRCode = 105,
        eBC_HIBCLicPDF417 = 106,
        eBC_HIBCPasPDF417 = 107,
        eBC_HIBCLicMPDF417 = 108,
        eBC_HIBCPasMPDF417 = 109,
        eBC_HIBCLicCODABLOCK_F = 110,
        eBC_HIBCPasCODABLOCK_F = 111,
        eBC_QRCode2005 = 112,
        eBC_PZN8 = 113,
        eBC_DotCode = 115,
        eBC_HanXin = 116,
        eBC_USPSIMPackage = 117,
    }

    public enum BcFontAboveEnum : int
    {
        unused,
        fix_false,
        fix_true,
        variable
    }

    public class BcProperties : RxEnum<BcTypeEnum>
    {
        public BcProperties(BcTypeEnum value, string display, string font=null)
        {
            Value = value;
            Display = display;

            Font = font;
            Above = BcFontAboveEnum.fix_false;
            Stretch = false;
        }

        private string _Font;
        public string Font
        {
            get { return _Font; }
            set { SetProperty<string>(ref _Font, value); }
        }

        public bool Stretch{get; set;}
        public BcFontAboveEnum Above { get; set; }

    }
   
    public class BarcodeTypeList : IEnumerable
    {

        private static List<BcProperties> _List;
        public BarcodeTypeList()
        {
            if (_List == null)
            {
                _List = new List<BcProperties>();
                _List.Add(new BcProperties(BcTypeEnum.eBC_Code11,                    "Code 11",               "OCRB") { Above = BcFontAboveEnum.variable });
                _List.Add(new BcProperties(BcTypeEnum.eBC_2OF5,                      "Code 2OF5 Standard",    "OCRB"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_2OF5IL,                    "Code 2OF5 Interleaved", "OCRB"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_2OF5IATA,                  "Code 2OF5 IATA",        "OCRB"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_2OF5M,                     "Code 2OF5 Matrix",      "OCRB"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_2OF5DL,                    "Code 2OF5 DataLogic",   "OCRB"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_2OF5IND,                   "Code 2OF5 Industry",    "OCRB"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_3OF9,                      "Code 39",               "OCRB"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_3OF9A,                     "Code 39 Full ASCII",    "OCRB"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_EAN8,                      "EAN8",                  "OCRB"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_EAN8P2,                    "EAN8 2 Digits add on",  "OCRB"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_EAN8P5,                    "EAN8 5 Digits add on",  "OCRB"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_EAN13,                     "EAN13",                 "OCRB"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_EAN13P2,                   "EAN13 2 Digits add on", "OCRB"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_EAN13P5,                   "EAN13 5 Digits add on", "OCRB"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_EAN128,                    "UCC/EAN-128",           "OCRB"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_UPC12,                     "UPC12",                 "OCRB"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_CodaBar2,                  "Codabar 2 Widths",      "OCRB"));
            //    _List.Add(new BcProperties(BcTypeEnum.eBC_CodaBar18,                 "Codabar 18 Widths",     "OCRB"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_Code128,                   "Code128",               "OCRB"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_DPLeit,                    "DP Leitcode",           "OCRB"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_DPIdent,                   "DP Identcode",          "OCRB"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_ISBN13P5,                  "ISBN13P5",              "OCRB"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_ISMN,                      "ISMN",                  "OCRB"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_9OF3,                      "Code 93",               "OCRB"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_ISSN,                      "ISSN",                  "OCRB"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_ISSNP2,                    "ISSNP2",                "OCRB"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_Flattermarken,             "Flattermarken",         "OCRB"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_GS1DataBar,                "GS1 DataBar (RSS-14)"));
                // _List.Add(new BcProperties(BcTypeEnum.eBC_GS1DataBarLtd,             "GS1 DataBar Limited (RSS)"));
                // _List.Add(new BcProperties(BcTypeEnum.eBC_GS1DataBarExp,             "GS1 DataBar Expanded (RSS)"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_TelepenAlpha,              "Telepen Alpha",           "OCRB"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_UCC128,                    "UCC128",                  "OCRB"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_UPCA,                      "UPCA",                    "OCRB"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_UPCAP2,                    "UPCAP2",                  "OCRB"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_UPCAP5,                    "UPCAP5",                  "OCRB"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_UPCE,                      "UPCE",                    "OCRB"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_UPCEP2,                    "UPCEP2",                  "OCRB"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_UPCEP5,                    "UPCEP5",                  "OCRB"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_USPSPostNet5,              "USPS PostNet5",           "OCRB"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_USPSPostNet6,              "USPS PostNet6",           "OCRB"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_USPSPostNet9,              "USPS PostNet9",           "OCRB"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_USPSPostNet10,             "USPS PostNet10",          "OCRB"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_USPSPostNet11,             "USPS PostNet11",          "OCRB"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_USPSPostNet12,             "USPS PostNet12",          "OCRB"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_Plessey,                   "Plessey",                 "OCRB"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_MSI,                       "MSI",                     "OCRB"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_SSCC18,                    "SSCC-18"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_FIM,                       "FIM"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_LOGMARS,                   "LOGMARS",                 "OCRB"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_Pharma1,                   "Pharmacode One-Track",    "OCRB"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_PZN7,                      "PZN7"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_Pharma2,                   "Pharmacode Two-Track",    "OCRB"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_CEPNet,                    "Brazilian CEPNet"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_PDF417,                    "PDF417"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_PDF417Trunc,               "PDF417 Truncated"));
           //     _List.Add(new BcProperties(BcTypeEnum.eBC_MAXICODE,                  "MaxiCode") { Stretch = true });
                _List.Add(new BcProperties(BcTypeEnum.eBC_QRCode,                    "QR-Code") { Stretch=true});
                _List.Add(new BcProperties(BcTypeEnum.eBC_Code128A,                  "Code128A",                "OCRB"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_Code128B,                  "Code128B",                "OCRB"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_Code128C,                  "Code128C",                "OCRB"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_9OF3A,                     "Code 93 Full ASCII",      "OCRB"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_AusPostCustom,             "Australian Post Custom",  "OCRB"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_AusPostCustom2,            "Australian Post Custom2", "OCRB"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_AusPostCustom3,            "Australian Post Custom3", "OCRB"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_AusPostReplyPaid,          "Australian Post Reply Paid", "OCRB"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_AusPostRouting,            "Australian Post Routing",    "OCRB"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_AusPostRedirect,           "Australian Post Redirect",   "OCRB"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_ISBN13,                    "ISBN 13",                    "OCRB"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_RM4SCC,                    "Royal Mail 4 State (RM4SCC)", "OCRB"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_DataMatrix,                "Data Matrix") { Stretch=true });
                _List.Add(new BcProperties(BcTypeEnum.eBC_EAN14,                     "EAN14",                     "OCRB"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_VIN,                       "VIN / FIN"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_CODABLOCK_F,               "CODABLOCK-F"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_NVE18,                     "NVE-18"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_JapanesePostal,            "Japanese Postal",             "OCRB"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_KoreanPostalAuth,          "Korean Postal Authority",     "OCRB"));
                // _List.Add(new BcProperties(BcTypeEnum.eBC_GS1DataBarTrunc,           "GS1 DataBar Truncated (RSS)"));
                // _List.Add(new BcProperties(BcTypeEnum.eBC_GS1DataBarStacked,         "GS1 DataBar Stacked (RSS)"));
                // _List.Add(new BcProperties(BcTypeEnum.eBC_GS1DataBarStackedOmni,     "GS1 DataBar Stacked Omni (RSS)"));
                // _List.Add(new BcProperties(BcTypeEnum.eBC_GS1DataBarExpStacked,      "GS1 DataBar Expanded Stacked (RSS)"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_Planet12,                  "Planet 12 digit",             "OCRB"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_Planet14,                  "Planet 14 digit",             "OCRB"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_MicroPDF417,               "MicroPDF417"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_USPSIntelligentMail,       "USPS Intelligent Mail Barcode (IM)", "OCRB"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_PlesseyBidir,              "Plessey Bidirectional",           "OCRB"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_Telepen,                   "Telepen",                         "OCRB"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_GS1_128,                   "GS1-128",                         "OCRB"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_ITF14,                     "ITF-14",                          "OCRB"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_KIX,                       "KIX",                             "OCRB"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_Code32,                    "Code 32",                         "OCRB"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_Aztec,                     "Aztec Code") { Stretch=true});
                _List.Add(new BcProperties(BcTypeEnum.eBC_DAFT,                      "DAFT Code"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_ItalianPostal2Of5,         "Italian Postal 2 of 5",       "OCRB"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_ItalianPostal3Of9,         "Italian Postal 3 of 9",       "OCRB"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_DPD,                       "DPD",                         "OCRB"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_MicroQRCode,               "Micro QR-Code") { Stretch = true });
                //_List.Add(new BcProperties(BcTypeEnum.eBC_HIBCLic128,                "HIBC LIC 128"));
                //_List.Add(new BcProperties(BcTypeEnum.eBC_HIBCLic3OF9,               "HIBC LIC 39"));
                //_List.Add(new BcProperties(BcTypeEnum.eBC_HIBCPas128,                "HIBC PAS 128"));
                //_List.Add(new BcProperties(BcTypeEnum.eBC_HIBCPas3OF9,               "HIBC PAS 39"));
                //_List.Add(new BcProperties(BcTypeEnum.eBC_HIBCLicDataMatrix,         "HIBC LIC Data Matrix"));
                //_List.Add(new BcProperties(BcTypeEnum.eBC_HIBCPasDataMatrix,         "HIBC PAS Data Matrix"));
                //_List.Add(new BcProperties(BcTypeEnum.eBC_HIBCLicQRCode,             "HIBC LIC QR-Code"));
                //_List.Add(new BcProperties(BcTypeEnum.eBC_HIBCPasQRCode,             "HIBC PAS QR-Code"));
                //_List.Add(new BcProperties(BcTypeEnum.eBC_HIBCLicPDF417,             "HIBC LIC PDF417"));
                //_List.Add(new BcProperties(BcTypeEnum.eBC_HIBCPasPDF417,             "HIBC PAS PDF417"));
                //_List.Add(new BcProperties(BcTypeEnum.eBC_HIBCLicMPDF417,            "HIBC LIC MicroPDF417"));
                //_List.Add(new BcProperties(BcTypeEnum.eBC_HIBCPasMPDF417,            "HIBC PAS MicroPDF417"));
                //_List.Add(new BcProperties(BcTypeEnum.eBC_HIBCLicCODABLOCK_F,        "HIBC LIC CODABLOCK-F"));
                //_List.Add(new BcProperties(BcTypeEnum.eBC_HIBCPasCODABLOCK_F,        "HIBC PAS CODABLOCK-F"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_QRCode2005,                "QR-Code 2005") { Stretch = true });
                _List.Add(new BcProperties(BcTypeEnum.eBC_PZN8,                      "PZN8"));
                //_List.Add(new BcProperties(BcTypeEnum.eBC_DotCode,                   "DotCode"));
                _List.Add(new BcProperties(BcTypeEnum.eBC_HanXin,                    "Han Xin Code") { Stretch = true });
                _List.Add(new BcProperties(BcTypeEnum.eBC_USPSIMPackage,             "USPS Intelligent Mail Package (IMpb)", "OCRB"));

                //--- sort the list ------------------------------------------------------------
                CompareInfo compare = CultureInfo.InvariantCulture.CompareInfo;
                _List.Sort((BcProperties a, BcProperties b) => compare.Compare(a.Display, b.Display));
            }
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return (IEnumerator)new RxListEnumerator<BcProperties>(_List);
        }
    }
 }
