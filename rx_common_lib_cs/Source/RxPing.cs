using System;
using System.Linq;

namespace RX_Common
{
    public class RxPing
    {
        public static string    _Host;
        public static DateTime  _Time;

        public static bool Ping(string ipaddr)
        {
            int ret=Rx.ExecuteProcess("ping.exe", string.Format("-n 1 -w 20 {0}", ipaddr));
            if (ret==0) return true;
            return false;
        }

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
                    TimeSpan diff = DateTime.Now.Subtract(_Time);
                    if (part[2].Equals(_Host) && diff.TotalSeconds<20) return true;

                    int ret=Rx.ExecuteProcess("ping.exe", string.Format("-n 1 -w 20 {0}", part[2]));
                    if (ret==0)
                    {
                        _Time = DateTime.Now;
                        _Host=part[2];
                        return true;
                    }
                    _Host=null;
                    return false;
                }
            }
            catch(Exception e)
            {
                Console.WriteLine(e.Message);
            }
            return false;
        }
    }
}
