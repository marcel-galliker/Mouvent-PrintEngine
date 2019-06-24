using RX_LabelComposer.Models.Enums;
using System;
using System.Runtime.InteropServices;

namespace RX_LabelComposer.External
{
    // Help: http://msdn.microsoft.com/de-de/library/cc431203.aspx

    struct SFieldDef
    {
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
        public string   name;
        public Int32    pos;
        public Int32    len;
    }

    struct SFileDef
    {
        public Int32    size;

        public Int32    id;

        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 256)]
        public string   dataFile;
        public Int32    header;
        public Int32    unicode;
        public Int32    codePage;
        public Int32    recordCnt;

        public FileFormatEnum       fileFormat;
        public Int32                recLen;
        public Int32                filter;
        public FieldSeparatorEnum   fieldSep;

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 64)]
        public SFieldDef[]    field;
    }

    class RX_Dat
    {
        //--- Interface to RIP DLL --------------------------------
        [DllImport("rx_rip_lib.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int dat_load_file_def(IntPtr doc, string tempPath, ref SFileDef fileDef);

        [DllImport("rx_rip_lib.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int dat_save_file_def(IntPtr doc, string tempPath, ref SFileDef fileDef);

        [DllImport("rx_rip_lib.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void dat_set_file_def(ref SFileDef fileDef);

        [DllImport("rx_rip_lib.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int dat_seek(int recNo);

        [DllImport("rx_rip_lib.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int dat_read_next_record();

        [DllImport("rx_rip_lib.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int dat_get_field(int no, byte[] data, int maxsize);

        [DllImport("rx_rip_lib.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int dat_get_buffer(int pos, int len, byte[] data);

        [DllImport("rx_rip_lib.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int dat_set_buffer(int pos, int len, byte[] data);

        [DllImport("rx_rip_lib.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int dat_analyse_file(ref SFileDef fileDef);

        [DllImport("rx_rip_lib.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int dat_max_record_size();
    }
}
