﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using RX_DigiPrint.Services;

namespace RX_DigiPrint.Helpers
{
    class Constants
    {
        public const int JetsPerHead = 2048;

        public const int PrintSystem_MaxPrintHeads = 8;

        public const int kHeadsPerCluster = 4;

        public const double HeadWidth = (25.4 / 1200) * JetsPerHead;

        public const int Waste = TcpIp.InkSupplyCnt + 1;    // Waste cannister number

        public const int WasteMax = 18;     // Waste maximum weight
    }
}
