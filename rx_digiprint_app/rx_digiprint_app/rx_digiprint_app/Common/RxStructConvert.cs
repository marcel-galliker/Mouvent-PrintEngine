using System;
using System.Runtime.InteropServices;

namespace DigiPrint.Common
{
    public class RxStructConvert
    {
        //--- ToStruct --------------------------------------
        public static int ToStruct<Type>(out Type dst, byte[] src, int offset=0)
        {
            int size = Marshal.SizeOf<Type>();
            IntPtr ptr = Marshal.AllocHGlobal(size);
            Marshal.Copy(src, offset, ptr, size);
            dst = (Type)Marshal.PtrToStructure<Type>(ptr);
            Marshal.FreeHGlobal(ptr);
            return size;
        }

        //--- ToBuffer --------------------------------------------
        public static void ToBuffer<Type>(out byte[] dst, Type src)
        {
            int size    = Marshal.SizeOf<Type>();           
            IntPtr ptr  = Marshal.AllocHGlobal(size);
            Marshal.StructureToPtr(src, ptr, false);
            dst = new byte[size];
            Marshal.Copy(ptr, dst, 0, size);
            Marshal.FreeHGlobal(ptr);            
        }

        //--- ToBuffer_Byte --------------------------------------------
        public static void ToBuffer_Byte(out byte[] dst, char[] src, int hdrsize, int size)
        {
            IntPtr ptr  = Marshal.AllocHGlobal(hdrsize+size);
            Marshal.StructureToPtr(src, ptr, false);
            dst = new byte[size];
            Marshal.Copy(ptr, dst, hdrsize, hdrsize+size);
            Marshal.FreeHGlobal(ptr);            
        }

        //--- OverwriteBuffer ---------------------------------------
        public static void OverwriteBuffer<Type>(byte[] dst, Type src)
        {
            int size = Marshal.SizeOf<Type>();
            IntPtr ptr = Marshal.AllocHGlobal(size);
            Marshal.StructureToPtr(src, ptr, false);
            Marshal.Copy(ptr, dst, 0, size);
            Marshal.FreeHGlobal(ptr);            
        }
    }
}
