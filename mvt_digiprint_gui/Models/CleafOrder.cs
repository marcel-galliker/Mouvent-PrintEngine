using RX_Common;
using RX_DigiPrint.Services;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml;

namespace RX_DigiPrint.Models
{
    public class CleafOrder : RxBindable
    {
        public static string OrdersDir = "\\\\192.168.200.1\\orders";

        public CleafFlexo[] flexo   = new CleafFlexo[4];
        public int[]        ink_ml = new int[4];
        public ObservableCollection<CleafRoll> Rolls = new    ObservableCollection<CleafRoll>();      

        public CleafOrder()
        {
            int i;
            for (i=0; i<flexo.Length; i++) flexo[i] = new CleafFlexo();
        }

        //--- Property Id ---------------------------------------
        private string _Id;
        public string Id
        {
            get { return _Id; }
            set { SetProperty(ref _Id, value); }
        }
        
        //--- Property Code ---------------------------------------
        private string _Code;
        public string Code
        {
            get { return _Code; }
            set { SetProperty(ref _Code, value); }
        }

        //--- Property Material ---------------------------------------
        private string _Material;
        public string Material
        {
            get { return _Material; }
            set { SetProperty(ref _Material, value); }
        }
        
        //--- Property AbsCode ---------------------------------------
        private string _AbsCode;
        public string AbsCode
        {
            get { return _AbsCode; }
            set { SetProperty(ref _AbsCode, value); }
        }

        //--- Property Speed ---------------------------------------
        private int _Speed;
        public int Speed
        {
            get { return _Speed; }
            set { SetProperty(ref _Speed, value); }
        }

        //--- Property PrintHeight ---------------------------------------
        private double _PrintHeight;
        public double PrintHeight
        {
            get { return _PrintHeight; }
            set { SetProperty(ref _PrintHeight, value); }
        }

        //--- Property PrintFile ---------------------------------------
        private string _PrintFile;
        public string PrintFile
        {
            get { return _PrintFile; }
            set { SetProperty(ref _PrintFile, value); }
        }
        
        //--- Property Length ---------------------------------------
        private int _Length;
        public int Length
        {
            get { return _Length; }
            set { SetProperty(ref _Length, value); }
        }

        //--- Property Produced ---------------------------------------
        private int _Produced;
        public int Produced
        {
            get { return _Produced; }
            set { SetProperty(ref _Produced, value); }
        }

        //--- Property Waste ---------------------------------------
        private int _Waste;
        public int Waste
        {
            get { return _Waste; }
            set { SetProperty(ref _Waste, value); }
        }
        
        //--- Property Gloss ---------------------------------------
        private int _Gloss;
        public int Gloss
        {
            get { return _Gloss; }
            set { SetProperty(ref _Gloss, value); }
        }

        //--- Property User ---------------------------------------
        private string _User;
        public string User
        {
            get { return _User; }
            set { SetProperty(ref _User, value); }
        }

        //--- Property Note ---------------------------------------
        private string _Note;
        public string Note
        {
            get { return _Note; }
            set { SetProperty(ref _Note, value); }
        }

        //--- ReadFile -----------------------------
        public void ReadFileInDir(string dir)
        {
            try 
            { 
                string[] part = dir.Split('\\');
                string path = dir+'\\'+part[4]+".xml";
                ReadFile(path);
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
            }
        }

        //--- Update_Order -------------------------------------------
        public void Update_Order(TcpIp.SCleafOrder order)
        {
            Id          = order.id;
            Code        = order.code;
            AbsCode     = order.absCode;
            Material    = order.material;
            Speed       = order.speed;
            Length      = order.length;
            PrintHeight = order.printHeight;
            PrintFile   = order.printFile;
            Gloss       = order.gloss;
            Note        = order.note;
            int i;
            for (i=0; i<4; i++) 
            {
                flexo[i].LacCode    = order.flexo[i].lacCode;
                flexo[i].Anilox     = order.flexo[i].anilox;
                flexo[i].RubberRoll = order.flexo[i].rubberRoll;
            }
        }

        //--- Update_Prodction -------------------------------------------
        public void Update_Prodction(TcpIp.SCleafProduction prod)
        {
            int i;
            Produced = prod.produced;
            Waste    = prod.waste;
            for (i=0; i<ink_ml.Length; i++)
                ink_ml[i] = prod.inkUsed[i];
        }

        //--- Update_Roll -------------------------------------------
        public void Update_Roll(TcpIp.SCleafRoll roll)
        {
            int idx=roll.no-1;
            if (Rolls.Count>idx) Rolls[idx] = new CleafRoll(roll);
            else                 Rolls.Add(new CleafRoll(roll));
        }

        //--- ReadFile --------------------------------------------------
        public void ReadFile(string path)
        {
            int flxNo=0;
            XmlTextReader xml = new XmlTextReader(path);
            while(xml.Read())
            {
                if (xml.NodeType==XmlNodeType.Element)
                {
                    if (xml.Name.Equals("Flexo"))
                    {
                        CleafFlexo flx = new CleafFlexo();
                        flx.LoadFromXml(xml);
                        flexo[flxNo++] = flx;
                    }
                    else 
                    {
                        foreach(var prop in GetType().GetProperties())
                        {
                            var attr=xml.GetAttribute(prop.Name);
                        
                            if (attr!=null)
                            { 
                                try
                                {
                                    prop.SetValue(this, TypeDescriptor.GetConverter(prop.PropertyType).ConvertFromString(attr));
                                }
                                catch (Exception ex)
                                {
                                    Console.WriteLine(ex.Message); 
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
