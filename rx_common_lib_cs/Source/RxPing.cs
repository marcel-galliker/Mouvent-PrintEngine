using System;
using System.Linq;

namespace RX_Common
{
    public class RxPing
    {
        public static bool HostReachable(string path)
        {
            if (path==null)  return false;
            try
            {
                string[] part = path.Split('\\');
                if (part[0].EndsWith(":")) return true;
                else if (part[2].Equals("localhost")) return true;
                else if (part.Length>1 && part[0].Equals("") && part[1].Equals("") && part.Count()>2)
                {
                    int ret=Rx.ExecuteProcess("ping.exe", string.Format("-n 1 -w 20 {0}", part[2]));
                    return ret==0;
                }
            }
            catch(Exception)
            { }
            return false;
        }
    }
}
