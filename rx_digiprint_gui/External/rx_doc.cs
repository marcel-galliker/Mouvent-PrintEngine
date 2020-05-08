using System;
using System.Runtime.InteropServices;

namespace RX_LabelComposer.External
{
    class RX_Doc
    {
        //--- Interface to RIP DLL --------------------------------
        [DllImport("rx_rip_lib.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr rx_xml_load(string path);
        
        [DllImport("rx_rip_lib.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr rx_xml_new();
        
        [DllImport("rx_rip_lib.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int rx_xml_save(string path, IntPtr doc);

        [DllImport("rx_rip_lib.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int rx_xml_free(IntPtr doc);
    }
}
