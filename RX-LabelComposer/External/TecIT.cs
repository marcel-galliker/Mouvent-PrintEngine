using RX_LabelComposer.Models.Enums;
using System.Runtime.InteropServices;

namespace RX_LabelComposer.External
{
    class TecIT
    {

        #if x64
            private const string _dll = "TBarCode11_x64.dll";
        #else
            private const string _dll = "TBarCode11.dll";
        #endif

        [DllImport(_dll, CallingConvention = CallingConvention.StdCall)]
        public static extern int BCGetCountBars(BcTypeEnum eBarCType);

        [DllImport(_dll, CallingConvention = CallingConvention.StdCall)]
        public static extern int BCGetCountSpaces(BcTypeEnum eBarCType);

        [DllImport(_dll, CallingConvention = CallingConvention.StdCall)]
        public static extern bool BCIsCompositeComponentAllowed(int barcodeType, int componentType);

    }
}
