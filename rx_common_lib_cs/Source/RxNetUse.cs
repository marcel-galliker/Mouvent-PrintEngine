using System.Diagnostics;
using System.IO;
using System.Linq;

namespace RX_Common.Source
{
    public class RxNetUse
    {
        //--- OpenShare -------------------------------------
        static public int OpenShare(string share, string user, string password)
        {
            if (share!=null && share!="localhost")
            {
                if (!RxPing.HostReachable(share)) return 2;
                Process process = new Process();
                process.StartInfo.RedirectStandardOutput = true;
                process.StartInfo.UseShellExecute = false;
                process.StartInfo.CreateNoWindow = true;
                process.StartInfo.FileName = @"net.exe";
                if (user!=null) process.StartInfo.Arguments = string.Format("use {0} /USER:{1} {2}", share, user, password);
                else            process.StartInfo.Arguments = string.Format("use {0}", share);
                process.Start();
                process.WaitForExit();
                return process.ExitCode;
            }
            return 2;
        }

        //--- DeleteShare -------------------------------------
        static public int DeleteShare(string share)
        {
            if (share!=null && share!="localhost")
            {
                Process process = new Process();
                process.StartInfo.RedirectStandardOutput = true;
                process.StartInfo.UseShellExecute = false;
                process.StartInfo.CreateNoWindow = true;
                process.StartInfo.FileName = @"net.exe";
                process.StartInfo.Arguments = string.Format("use {0} /Delete", share);
                process.Start();
                process.WaitForExit();
                return process.ExitCode;
            }
            return 2;
        }

        //--- ShareConnected ---------------------------------------
        static public bool ShareConnected(string share)
        {
            if (RxPing.HostReachable(share))
            {
                Process process = new Process();
                string  output;
                int     ret;

                process.StartInfo.RedirectStandardOutput = true;
                process.StartInfo.UseShellExecute = false;
                process.StartInfo.CreateNoWindow = true;
                process.StartInfo.FileName = @"net.exe";
                process.StartInfo.Arguments = string.Format("use {0}", Path.GetPathRoot(share));
                if (process.StartInfo.Arguments.EndsWith(":\\"))
                    return true;

                process.Start();
                if (!process.WaitForExit(200)) return false;
                output = process.StandardOutput.ReadToEnd();
                ret = process.ExitCode;
                string[] lines=output.Split('\n');
                if (ret==0 && lines.Count()==3) 
                    return true; // erfolgreich ausgeführt
                if (lines.Count()>3 && lines[3].EndsWith("OK\r")) 
                    return true;
            }
            return false;
        }
    }
}
