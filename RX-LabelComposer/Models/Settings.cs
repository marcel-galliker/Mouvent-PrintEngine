using RX_Common;

namespace RX_LabelComposer.Models
{
    public class Settings : RxSettingsBase
    {
        //--- Constructor --------------------------------------------------
        public Settings()
        {
        }
            
        //--- Property LastUsedFile ---------------------------------------
        private string _LastUsedFile;
        public string LastUsedFile
        {
            get { return _LastUsedFile; }
            set { _LastUsedFile = value; }
        }

        //--- Property WindowState ---------------------------------------
        private System.Windows.WindowState _WindowState = System.Windows.WindowState.Maximized;
        public System.Windows.WindowState WindowState
        {
            get { return _WindowState; }
            set { _WindowState = value; }
        }

        //--- Property WindowHeight ---------------------------------------
        private int _WindowHeight;
        public int WindowHeight
        {
            get { return _WindowHeight; }
            set { _WindowHeight = value; }
        }

        //--- Property WindowWidth ---------------------------------------
        private int _WindowWidth;
        public int WindowWidth
        {
            get { return _WindowWidth; }
            set { _WindowWidth = value; }
        }

        //--- Property LabelPropExpanded ---------------------------------------
        private bool _LabelPropExpanded;
        public bool LabelPropExpanded
        {
            get { return _LabelPropExpanded; }
            set { _LabelPropExpanded = value; }
        }

        //--- Property LabelBoxExpanded ---------------------------------------
        private bool _LabelBoxPropExpanded;
        public bool LabelBoxPropExpanded
        {
            get { return _LabelBoxPropExpanded; }
            set { _LabelBoxPropExpanded = value; }
        }

        //--- Property DataFontSize ---------------------------------------
        private double? _DataFontSize;
        public double? DataFontSize
        {
            get { return _DataFontSize; }
            set { _DataFontSize = value; }
        }        
    }
}
