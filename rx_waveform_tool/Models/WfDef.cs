using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml;
using RxWfTool.Helpers;
using System.Windows.Media;
using System.IO;
using System.Windows;
using  System.Security.Cryptography;


namespace RxWfTool.Models
{
    public class WfDef : NotificationObject
    {
        public static string FileExtension = "wfd";
        private string _FileName = "Filename";
        public event EventHandler<EventArgs> ListChanged;

        //--- constructor -----------------------------
        public WfDef()
        {       
            _List=new ObservableCollection<WfItem>();
            _Ink =new Ink();
            WfItem.OnLenChanged = LenChanged;
        }

        //--- Property FileName ----------------------------
        public string FileName
        {
            get { return _FileName; }
            set { SetProperty(ref _FileName, value); }
        }

        //--- Changed ---------------------------------------
        private bool _Changed;
        public bool Changed
        {
            get { return _Changed || _Ink.Changed; }
            set { 
                    if (SetProperty(ref _Changed, value))
                        update_delta();
                }
        }

        //--- Property PulseLength ---------------------------------------
        private double _PulseLength;
        public double PulseLength
        {
            get { return _PulseLength; }
            set { SetProperty(ref _PulseLength, value);}
        }
        
        //--- Property _PulseFreq ---------------------------------------
        private double  _PulseFreq;
        public double PulseFreq
        {
            get { return _PulseFreq; }
            set { 
                    if (SetProperty(ref _PulseFreq, value)) 
                        _Ink.MaxFreq = (int)(1000.0*value);
                }
        }

        //--- Property _PulseFreq ---------------------------------------
        private double _PulseSpeed;
        public double PulseSpeed
        {
            get { return _PulseSpeed; }
            set { SetProperty(ref _PulseSpeed, value);}
        }

        //--- LenChanged ------------------------------------------
        public void LenChanged(int no, int delta)
        {
            for (int i=no; i<_List.Count; i++)
            {
                _List[i].Position += delta;
            }
            update_delta();
        }

        //--- Property List ----------------------------------
        private ObservableCollection<WfItem> _List;
        public ObservableCollection<WfItem> List
        {
            get { return _List; }
            private set { SetProperty(ref _List, value); }
        }

        //--- insert --------------------------------------------
        public void insert(int idx)
        {
            if (idx<_List.Count)
            { 
                WfItem item = new WfItem();
                item.ListChanged += item_ListChanged;
                item.Position = _List[idx].Position;
                item.Voltage  = _List[idx].Voltage;
                item.Len      = 0;
                _List.Insert(idx, item);
                for (idx=0; idx<_List.Count(); idx++)
                {
                    _List[idx].No = idx+1;
                }
            }
            else
            {
                WfItem item = new WfItem();
                item.ListChanged += item_ListChanged;
                item.No       = 1;
                item.Position = 0;
                item.Voltage  = 0;
                _List.Add(item);
            }
        }

        //--- item_ListChanged ----------------------------------
        void item_ListChanged(object sender, EventArgs e)
        {
            update_delta();
            Changed = true;
            if (ListChanged != null) ListChanged(sender, e);
        }

        //--- delete --------------------------------------------
        public void delete(int idx)
        {
            if (idx<_List.Count)
            { 
                _List.RemoveAt(idx);
                renumber();
                Changed = true;
                if (ListChanged != null) ListChanged(this, null);
            }
        }

        //--- renumber ------------------------------------------
        private void renumber()
        {
            _List[0].Len = null;
            for (int i=0; i<_List.Count; i++)
            {
                _List[i].No = i+1;
                if (i+1<_List.Count) _List[i+1].Len = _List[i+1].Position-_List[i].Position;
            }
        }

        //--- update_delta -----------------------------------------
        private void update_delta()
        {
            double length=0;
            if (_List.Count()>0)
            {
                int first=0;
                int last=0;
                for (int i=1; i<_List.Count(); i++)
                {
                    double len=_List[i].Position-_List[i-1].Position;
                    if (_List[i-1].Voltage==0 && _List[i].Voltage>0 && first==0) first=_List[i-1].Position;
                    if (_List[i].Voltage==0 && _List[i-1].Voltage>0) last=_List[i].Position;
                    if (len>0) _List[i-1].Delta = (_List[i].Voltage-_List[i-1].Voltage)/(len/80.0);
                }
                length=(last-first+WfItem.FirstPulsePos+WfItem.Filler)*WfItem.Interval;
            }
            if (length==0)
            {
                PulseLength = 0;
                PulseFreq   = 0;
                PulseSpeed  = 0;
            }
            else
            {
                double khz=1000000/length;
                double mmin = khz*25.4/1200.0*60.0;
                PulseLength = length/1000;
                PulseFreq   = khz;
                PulseSpeed  = mmin;
            }
        }

        //--- open ----------------------------------------------
        public int open(string filename)
        {
            try
            {
                if (filename!=null && !filename.Equals(""))
                {
                    ObservableCollection<WfItem> list=new ObservableCollection<WfItem>();
                    XmlTextReader xml = new XmlTextReader(filename);

                    while (xml.Read())
                    {
                        if (_Ink.load(ref xml))
                        {
                            PulseFreq = _Ink.MaxFreq;
                        }           
                        else if (xml.NodeType==XmlNodeType.Element && xml.Name.Equals("item"))
                        {
                            WfItem item = new WfItem();
                            item.ListChanged += item_ListChanged;
                            item.No = list.Count+1;
                            RxXml.LoadProperty(xml, item, "Position");
                            RxXml.LoadProperty(xml, item, "Voltage");
                            list.Add(item);
                        }
                        else if (xml.NodeType==XmlNodeType.Element && xml.Name.Equals("check"))
                        {
                            byte[] hash = new byte[20];
                            xml.ReadBinHex(hash, 0, hash.Length);
                        //    Console.WriteLine("Code={0}", Encoding.Default.GetString(hash.ToArray()));
                        }
                    }
                    xml.Close();
                    _List = list;
                    renumber();
                    update_delta();
                    FileName = filename;
                    Changed = false;
                }
            }
            catch(Exception)
            {
                return 1;
            }

            return 0;
        }

        //---create_wave_files ------------------------------------------
        private void create_wave_files(string filename)
        {
            int i, cnt, pos, drop;
            int pos0;
            double volt;
            List<int> start=new List<int>();
            List<int> end=new List<int>();
            WfItem item;

            //---  create binary file -----------------
            BinaryWriter datfile = new BinaryWriter(File.Open(filename.Replace(".wfd", ".dat"), FileMode.Create));

            volt=0;
            pos=0;
            for (i=0; i<_List.Count; i++)
            {
                item = _List[i];
                while (pos<_List[i].Position)
                {
                    if (volt==0 && item.Voltage!=0) 
                    {
                        start.Add(new int());
                        start[start.Count()-1] = pos-1;
                    }
                    if (volt>0 && item.Voltage==0)
                    {
                        end.Add(new int());
                        end[end.Count()-1] = item.Position;
                    }
                    pos0 = pos;
                    while (pos <= item.Position)
                    {
                        datfile.Write((UInt16)((volt+(item.Voltage-volt) * (pos-pos0)/(item.Position-pos0)) * 1000));
                        pos++;
                    }
                    volt = item.Voltage;
                }
            }
            datfile.Close();
            
            if (false)
            {
                //--- crrate text file ----------------------
                StreamWriter txtfile = new StreamWriter(filename.Replace(".wfd", ".txt"));
                txtfile.WriteLine("WaveFormCnt    = {0}", start.Count());
                txtfile.WriteLine("WaveFormLength = {0}", pos);

                for (i=0; i<start.Count(); i++)
                    txtfile.WriteLine("start[{0}] = {1}", i, start[i]);

                for (i=0; i<end.Count(); i++)
                    txtfile.WriteLine("end[{0}] = {1}", i, end[i]);

                txtfile.WriteLine("All_ON = {0}", start[start.Count()-1]);

                cnt = start.Count();
                for (i=0, drop=0; i<start.Count(); i++)
                {
                    drop |= 0x01 << (cnt-i-1);
                    txtfile.WriteLine("dropsize[{0}] = {1}", i+1, drop);
                }
                txtfile.Close();
            }
        }       
        
        //--- save ----------------------------------------------
        public int save(string filepath)
        {
            if (filepath!=null && !filepath.Equals(""))
            {                
                MemoryStream buffer = new MemoryStream();
             
                XmlWriterSettings xmlWriterSettings = new XmlWriterSettings();
                xmlWriterSettings.Encoding = new UTF8Encoding(false);
                xmlWriterSettings.ConformanceLevel = ConformanceLevel.Auto;
                xmlWriterSettings.Indent = true;
                xmlWriterSettings.OmitXmlDeclaration=true;

                XmlWriter  xml = XmlWriter.Create(buffer, xmlWriterSettings);
                xml.WriteStartDocument();
                xml.WriteStartElement("WaveForm");
                _Ink.save(xml);
                for (int i=0; i<_List.Count; i++)
                { 
                    xml.WriteStartElement("item");
                    RxXml.SaveProperty(xml, _List[i], "Position");
                    RxXml.SaveProperty(xml, _List[i], "Voltage");
                    xml.WriteEndElement();
                }
                xml.Flush();

                //--- add checksum --------------------
                {
                    xml.WriteStartElement("check");
                    {
                        byte[] hash = new SHA1CryptoServiceProvider().ComputeHash(buffer.ToArray());    
                        xml.WriteBinHex(hash, 0, hash.Length);
                    }
                    xml.WriteEndElement();
                }

                xml.WriteEndElement();
                xml.WriteEndDocument(); 
                xml.Close();          

                //--- write to the file -------------------
                using(StreamWriter file = new StreamWriter(filepath))
                {
                    String str=Encoding.Default.GetString(buffer.ToArray());
                    file.Write(str);
                }
                // create_wave_files(filepath);
            }
            Changed = false;
             
            return 0;
        }

        //--- import --------------------------------------------------
        public int import(string filename)
        {
            StreamReader file;
            try
            {
                file = new StreamReader(filename);
            }
            catch (Exception e)
            {
                MessageBox.Show(e.Message);
                return 1;
            }
            try
            {
                int     time=64;
                int     maxVolt=32;
                int     volt=0;
                int     pos=0;
                bool    waveFormSamples=false;
                bool    sectionNumbers=false;
                char    separator=(char)0;

                ObservableCollection<WfItem> list=new ObservableCollection<WfItem>();
                { 
                    WfItem item = new WfItem();
                    item.ListChanged += item_ListChanged;
                    item.No = list.Count+1;
                    item.Position = 0;
                    item.Voltage  = 0;
                    list.Add(item);
                }

                while(!file.EndOfStream)
                {
                    string line = file.ReadLine();

                    Console.WriteLine(line);
                    if (separator==(char)0)
                    {
                        if (line.Contains('\t')) separator='\t';
                        else if (line.Contains(';')) separator=';';
                        else if (line.Contains(',')) separator=',';
                        else if (line.Contains(',')) separator=',';                         
                    }

                    if (line.StartsWith("Start Section-to-Multibit Mapping Table"))
                    {
                        sectionNumbers = true;
                    }
                    else if (line.StartsWith("End Section-to-Multibit Mapping Table"))
                    {
                        sectionNumbers = false;
                    }
                    else if (line.Contains("Start Waveform Samples"))
                    {
                        waveFormSamples = true;
                    }
                    else if (line.Contains("End Waveform Samples"))
                    {
                        waveFormSamples = false;
                    }

                    if (waveFormSamples)
                    { 
                    }                        
                    else if (sectionNumbers)
                    {
                        string[] value = line.Split(separator);
                        try
                        {
                            int dotszie=Convert.ToInt32(value[0]);
                            int greyLevel=0;
                            int i;
                            for (i=0; i<8; i++)
                            {
                                if (value[2+i].Equals("1")) greyLevel |= (1<<i);
                            }
                            Ink.GreyLevel[dotszie] = greyLevel;
                        }
                        catch(Exception)
                        { };
                    }
                    else if (line.StartsWith("Sample Clock Period [in ns]"))
                    {
                        string[] value = line.Split(separator);
                        time = Convert.ToInt32(value[1]);
                    }
                    else if (line.StartsWith("Maximum Sample Voltage"))
                    {
                        string[] value = line.Split(separator);
                        maxVolt = Convert.ToInt32(value[1]);
                    }
                    else if (line.StartsWith("Control Points"))
                    {
                        string[] value = line.Split(separator);
                        int v = Convert.ToInt32(value[2]);
                        int len  = Convert.ToInt32(value[3]);
                        double ramp = Convert.ToDouble(value[4]);
                        int rtime=0;

                        if      (volt<v) rtime = (int)((v-volt+ramp-1) / ramp);
                        else if (volt>v) rtime = (int)((volt-v+ramp-1) / ramp);

                        if (pos>0)
                        {                        
                            WfItem item = new WfItem();
                            item.ListChanged += item_ListChanged;
                            item.No = list.Count+1;                        
                            item.Position = Convert.ToInt32((pos-1+rtime)*time/WfItem.Interval);
                            item.Voltage  = (Int32)((v * maxVolt)/256.0+0.5);
                            list.Add(item);
                        }
                        volt=v;
                        pos += len;
                        { 
                            WfItem item = new WfItem();
                            item.ListChanged += item_ListChanged;
                            item.No = list.Count+1;
                            item.Position = Convert.ToInt32((pos-1)*time/WfItem.Interval);
                            item.Voltage  = (Int32)((v * maxVolt)/256.0+0.5);
                            list.Add(item);
                        }
                    }
                }

                _List = list;
                update_delta();
                renumber();
                /*
                if (filename.EndsWith("csv"))
                    FileName = filename.Replace("csv", FileExtension);
                else 
                    FileName = filename;
                */
                return 0;
            }
            catch (Exception e)
            {
                return 1;
            }
        }

        //--- Property Ink ---------------------------------------
        private Ink _Ink;
        public Ink Ink
        {
            get { return _Ink; }
            set { SetProperty(ref _Ink, value); }
        }
        
    }
}
