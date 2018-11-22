using rx_rip_gui.Services;
using System;
using System.Runtime.InteropServices;
using System.Text;

namespace rx_rip_gui.External
{
    class RX_EmbRip
    {
        [DllImport("rx_rip_lib.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int embrip_create_xml();

        [DllImport("rx_rip_lib.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int embrip_save_xml(int doc, string path);

        [DllImport("rx_rip_lib.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int embrip_open_xml(string path);

        [DllImport("rx_rip_lib.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int embrip_close_xml(int doc);

        [DllImport("rx_rip_lib.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int    embrip_screening_cfg(int doc, bool write, ref SScreeningCfg cfg);

        [DllImport("rx_rip_lib.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int    embrip_ripping_cfg(int doc, bool write, ref SRippingCfg cfg);

        [DllImport("rx_rip_lib.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int embrip_ScreeningModeStr(EScreeningMode mode, StringBuilder str);
    }
}
