using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace rx_rip_gui.Services
{
    public class rip_def
    {
        public static string RX_PRENV_ROOT = "d:/PrEnv/";
        public static string PRENV_EXT     = ".xml";
              
    };

    public enum EScreeningMode : byte
    {
        sm_undef,       // 00
        sm_1,           // 01
        sm_2,           // 02
        sm_3,           // 03
        sm_4,           // 04
    };


    public enum ERenderIntent : byte
    {
        ri_undef,           //  00
        ri_perceptual,      //  01
        ri_colorimetric,    //  02
        ri_saturation,      //  03
        ri_absolute,        //  04
    };

    public enum EBlackPreserve : byte
    {
        bp_undef,           //  00
        bp_no,              //  01
        bp_black_only,      //  02
        bp_black_plane,     //  03
    };

    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct SScreeningCfg
    {
        public EScreeningMode   mode;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 256)]
        public string           profileName;
    }

    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct SRippingCfg
    {
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 256)]
        public string           outputProfileName;
        public ERenderIntent    renderIntent;
        public EBlackPreserve   blackPreserve;
        public byte             blackPtComp;
        public byte             simuOverprint;
    }

    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct SPrintEnv
    {
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 256)]
        public string          name;        
        public SScreeningCfg   screening;
        public SRippingCfg     ripping;
    }    

}
