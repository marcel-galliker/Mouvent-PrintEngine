using System;
using System.Text;
using System.Net;
using System.Threading.Tasks;
using System.Collections.Generic;
using RX_DigiPrint.Models;
using System.Reflection;
using System.Collections.Specialized;
using System.Text.Json;
using System.Text.Json.Serialization;
using Microsoft.SqlServer.Server;
using System.Linq;
using System.Diagnostics;
using System.IO;

namespace RX_DigiPrint.Services
{
    public class RestService
    {
        // to build the rest response
        public class Data
        {
            public Data(String plegend = null, String punit = null)
            {
                legend = plegend;
                unit = punit;
            }
            public String id { get; set; }
            public String handle { get; set; }
            public String value { get; set; }
            public String error { get; set; }
            public String legend { get;  }
            public String unit { get; }
            [JsonIgnore]
            public object obj { get; set; } // C# obj where to retrieve the value
            [JsonIgnore]
            public String field { get; set; } // name of field of the object
        }

        private static List<Data> restdata;
        private static Dictionary<String, Data> dataBy;

        // set up the configuration of REST service
        private static void ConfigureRest()
        {
            restdata = new List<Data>();
            dataBy = new Dictionary<string, Data>();
            void addData(Object obj, String field, String id, String legend = null, String unit = null)
            {
                Data d = new Data(legend, unit);
                d.obj = obj;
                d.id = id;
                d.field = field;
                d.handle = restdata.Count.ToString();
                restdata.Add(d);
                dataBy["var=" + d.id.ToLower()] = d;
                dataBy["handles=" + d.handle] = d;
            }

            if (RxGlobals.RxInterface.GuiConnected && RxGlobals.InkSupply.List.Count > 1 && RxGlobals.ClusterStat.List.Count > 1)
            {
                addData(RxGlobals.PrinterStatus, "CounterTotal", "Printer.Statistics.TotalMetersPrinted", "Total distance printed", "meter");

                for (int i = 0; i < RxGlobals.PrintSystem.ColorCnt; i++)
                {
                    if (RxGlobals.PrintSystem.IS_Order != null)
                    {
                        int inkCylinderIndex = i * RxGlobals.PrintSystem.InkCylindersPerColor;
                        if (inkCylinderIndex < RxGlobals.PrintSystem.IS_Order.Length)
                        {
                            InkSupply ink = RxGlobals.InkSupply.List[RxGlobals.PrintSystem.IS_Order[inkCylinderIndex]];
                            if (ink.InkType is null || ink.InkType.Name is null)
                            {
                                continue;
                            }
                            String id = "Printer.Ink." + ink.InkType.ColorCode;

                            addData(ink, "CanisterLevel", id + ".Level", "Ink Level", "gram");
                            addData(ink.InkType, "Name", id + ".Name", "Ink name");
                            addData(ink.InkType, "Color", id + ".Color", "Ink color to display", "#rgb");
                        }
                    }
                }

                foreach (ClusterStat board in RxGlobals.ClusterStat.List)
                {
                    String id = "Printer.Cluster." + board.BoardNo;
                    addData(board, "ClusterTime", id + ".Time", "Cluster time", "second");
                    addData(board, "SerialNo", id + ".SerialNo", "Cluster serial no");
                }

            }
        }

        // Manage incomig HTTP query
        public static async Task HandleIncomingConnections()
        {
            HttpListener listener = null;

            try
            {
                // Create a Http server and start listening for incoming connections
                listener = new HttpListener();
                listener.Prefixes.Add("http://+:5555/logsrv/resources/");
                listener.Start();
            }
            catch (Exception)
            {
                // add firewall authorization (netsh)
                String fileName = Path.GetTempFileName();
                FileInfo fileInfo = new FileInfo(fileName);
                fileInfo.Attributes = FileAttributes.Temporary;

                StreamWriter streamWriter = File.AppendText(fileName);
                streamWriter.WriteLine("http add urlacl url=http://+:5555/logsrv/resources/ sddl=D:(A;;GA;;;WD)");
                streamWriter.WriteLine("advfirewall firewall add rule name=RESTRX dir=in action=allow protocol=TCP localport=5555");
                streamWriter.Flush();
                streamWriter.Close();

                ProcessStartInfo startInfo = new ProcessStartInfo("netsh.exe");
                startInfo.Arguments = "-f " + fileName;
                startInfo.Verb = "runas";
                startInfo.UseShellExecute = true;
                Process p = Process.Start(startInfo);
                p.WaitForExit();
                File.Delete(fileName);

                try
                {
                    listener = new HttpListener();
                    listener.Prefixes.Add("http://+:5555/logsrv/resources/");
                    listener.Start();
                }
                catch (Exception e)
                {
                    listener = null;
                    Console.WriteLine(e);
                }
            }

            // listen for ever for connection (if setup OK)
            while (listener != null)
            {
                // Will wait here until we hear from a connection
                HttpListenerContext ctx = await listener.GetContextAsync();

                // retreive the configuration of the Printer
                ConfigureRest();

                // Peel out the requests and response objects
                HttpListenerRequest req = ctx.Request;
                HttpListenerResponse resp = ctx.Response;
                resp.Headers.Add("Access-Control-Allow-Origin", "*");
                resp.Headers.Add("Access-Control-Allow-Methods", "GET");
                resp.Headers.Add("Access-Control-Allow-Credentials", "true");

                // use a dictionary with one entry (resources) to build the json
                Dictionary<string, List<Data>> pageData = new Dictionary<string, List<Data>>();
                var options = new JsonSerializerOptions
                {
                    IgnoreNullValues = true,
                    WriteIndented = true,
                    // readonly are legend and unit
                    IgnoreReadOnlyProperties = true,
                };

                // construct the response if we get a configuration
                if (req.HttpMethod == "GET" && restdata.Count > 1)
                {
                    NameValueCollection coll = req.QueryString;

                    // analyze the query
                    if (coll.Count == 0 || req.RawUrl.EndsWith("?attr"))
                    {
                        pageData["resources"] = restdata;
                        // with ?attr parameter add additional information
                        if (req.RawUrl.EndsWith("?attr")) options.IgnoreReadOnlyProperties = false;

                    } 
                    else if (coll["var"] != null || coll["handles"] != null )
                    {
                        pageData["resources"] = new List<Data>();
                        // retreive values for rest response
                        foreach (String key in coll)
                        {
                            if (key == "var" || key == "handles")
                            {
                                Data p = null;
                                foreach (String i in coll[key].ToLower().Split(','))
                                {
                                    if (dataBy.ContainsKey(key + "=" + i))
                                    {
                                        p = dataBy[key + "=" + i];
                                        p.value = p.obj.GetType().GetProperty(p.field).GetValue(p.obj, null).ToString();
                                        if (p.value == TcpIp.INVALID_VALUE.ToString())
                                        {
                                            p.value = "---";
                                            p.error = "undefined";
                                        }
                                    }
                                    else 
                                    {
                                        p = new Data();
                                        if (key == "var")
                                        {
                                            p.id = i;
                                        }
                                        else 
                                        {
                                            p.handle = i;
                                        }
                                        p.error = "not found";
                                    }
                                    pageData["resources"].Add(p);
                                }

                            }
                        } 
                    }
                }

                // if there is data to send
                if (pageData.Count == 1)
                {
                    byte[] data = JsonSerializer.SerializeToUtf8Bytes(pageData, options);
                    resp.ContentType = "application/json";
                    resp.ContentEncoding = Encoding.UTF8;
                    resp.ContentLength64 = data.LongLength;
                    // Write out to the response stream (asynchronously)
                    await resp.OutputStream.WriteAsync(data, 0, data.Length);
                }
                // else http error
                else resp.StatusCode = (int)HttpStatusCode.BadRequest;
                resp.Close();
            }
        }


        // Start the rest service (HttpListener) async
        public static void Start()
        {

            // Handle request
            Task listenTask = HandleIncomingConnections();

        }
    }
}

