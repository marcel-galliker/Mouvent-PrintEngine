using System;
using System.Runtime.InteropServices;
using System.Text;

namespace RX_LabelComposer.External
{
    // Help: http://msdn.microsoft.com/de-de/library/cc431203.aspx

    public enum IncrementEnum : int
    {
        INC_Undef       = 0,
        INC_perLabel    = 1,
        INC_perRow      = 2
    };

    struct SCounterDef
    {
        public Int32    size;

        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 64)]
        public string   elements_n;

        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 64)]
        public string   elements_a;

        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 64)]
        public string   elements_x;

        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
        public string   elements;

        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
        public string   start;

        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
        public string   end;

        public Int32    leadingZeros;

        public IncrementEnum increment;
    }

    class RX_Counter
    {
        //--- Interface to RIP DLL --------------------------------
        [DllImport("rx_rip_lib.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int ctr_load_def(IntPtr doc, ref SCounterDef ctrDef);

        [DllImport("rx_rip_lib.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int ctr_save_def(IntPtr doc, ref SCounterDef ctrDef);

        [DllImport("rx_rip_lib.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool ctr_def_is_equal(ref SCounterDef ctrDef1, ref SCounterDef ctrDef2);

        [DllImport("rx_rip_lib.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void ctr_set_def(ref SCounterDef ctrDef);

        [DllImport("rx_rip_lib.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void ctr_set_counter(Int32 recNo);

        [DllImport("rx_rip_lib.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void ctr_inc_counter();

        [DllImport("rx_rip_lib.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int ctr_get_counter(Int32 recNo, StringBuilder str);
    }
}
