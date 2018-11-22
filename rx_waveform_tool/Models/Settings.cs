using RX_Common;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace RxWfTool.Models
{
    public class Settings : RxSettingsBase
    {
        public static Settings RX_Settings = new Settings();
        public static string Path;

        //--- Constructor -----------------------
        public Settings()
        {
            Path = Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData)
                    +"\\"+System.Reflection.Assembly.GetExecutingAssembly().GetName().Name
                    +"\\"+"Settings.xml";
        }

        //--- Property LastFile ---------------------------------------
        private string _LastFile;
	    public string LastFile
	    {
		    get { return _LastFile;}
		    set { _LastFile=value;}
	    }
	
        //--- Property Width ---------------------------------------
        private double _Width;
	    public double Width
	    {
		    get { return _Width;}
		    set { _Width = value;}
	    }
	
        //--- Property Height ---------------------------------------
        private double _Height;
	    public double Height
	    {
		    get { return _Height;}
		    set { _Height=value;}
	    }

    }
}
