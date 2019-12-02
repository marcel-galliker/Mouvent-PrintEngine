using RX_Common;
using RX_DigiPrint.Models;
using RX_DigiPrint.Models.Enums;
using RX_DigiPrint.Services;
using System;
using System.Collections;
using System.ComponentModel;
using System.Globalization;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;

namespace RX_DigiPrint.Views.UserControls
{
    /// <summary>
    /// Interaction logic for PlcPar.xaml
    /// </summary>
    public partial class PlcParCtrl : UserControl, INotifyPropertyChanged
    {                
        public event PropertyChangedEventHandler PropertyChanged;
                        
        private PropertyInfo _ComboValueProp;
        private CUnit        _Unit;

        public enum ECtrlType : byte
        {
            Text,
            Combo,
            ComboStr,
            ImgCombo,
            CheckBox,
            Led
        };

        //--- constructor -----------------------------------------------
        public PlcParCtrl()
        {
            InitializeComponent();
            DataContext = this;
            Type = ECtrlType.Text;
                       
            RxGlobals.Settings.PropertyChanged += _Settings_PropertyChanged;
        }

        //--- _Settings_PropertyChanged -----------------------------------------
        private void _Settings_PropertyChanged(object sender, PropertyChangedEventArgs e)
        {
            if (e.PropertyName.Equals("Units"))
            {
                if (_Unit!=null) TB_Unit.Text = _Unit.Name;
            }
        }

        //--- UserControl_Loaded ----------------------------------
        private void UserControl_Loaded(object sender, RoutedEventArgs e)
        {
            PlcParPanel panel=_find_panel(CheckBoxCtrl);
            if (panel!=null)
            {
                if (panel.ChildPadding!=null && !panel.ChildPadding.Equals(new Thickness(0)))
                    this.Padding = panel.ChildPadding;
            } 
        }

        //--- Property HorizontalContentAlignment ---------------------------------------
	    public HorizontalAlignment HCA
	    {
		    get { return TextEditCtrl.HorizontalContentAlignment;}
		    set { 
                    TextEditCtrl.HorizontalContentAlignment = value;
                    TextCtrl.HorizontalAlignment = value;
                }
	    }

        //--- _CallPropertyChanged --------------------------------------------------------------
        private void _CallPropertyChanged([CallerMemberName]String info = null)
        {
            if (PropertyChanged != null)
                PropertyChanged(this, new PropertyChangedEventArgs(info));
        }

        //--- _checkbox_init ---------------------------------------------
        private void _checkbox_init()
        {
            int i;
            for (i=0; i<4; i++)
            {
                CheckBoxCtrl.ColumnDefinitions.Add(new ColumnDefinition());
                CheckBox box=new CheckBox()
                {
                    HorizontalAlignment=HorizontalAlignment.Center, 
                    VerticalAlignment=VerticalAlignment.Center,
                };
                box.Content=4-i;
                CheckBoxCtrl.Children.Add(box);
                box.Checked   +=_checkbox_checked;
                box.Unchecked +=_checkbox_checked;
                Grid.SetColumn(CheckBoxCtrl.Children[i], i);
            }
        }

        private bool _skip_checked=false;
        private void _checkbox_setvalue(int value)
        {
            _skip_checked = true;
            int i=CheckBoxCtrl.Children.Count;
            while (i>0)
            {
                i--;
                CheckBox box=(CheckBoxCtrl.Children[i] as CheckBox);
                box.IsChecked = ((value&0x01)!=0);
                value = value/2;
            }                            
            _skip_checked=false;
        }

        private void _checkbox_checked(object sender, RoutedEventArgs e)
        {
            if (!_skip_checked)
            {
                PlcParPanel panel=_find_panel(CheckBoxCtrl);
                if (panel!=null) panel.Changed=true;
                Changed = true;
            }

            int i=CheckBoxCtrl.Children.Count;
            int weight=1;
            int val=0;
            while (i>0)
            {
                i--;
                CheckBox box=(CheckBoxCtrl.Children[i] as CheckBox);
                if (box.IsChecked==true) val+=weight;
                weight = weight*2;
            }
            TextEditCtrl.Text = _Value = val.ToString();
            Console.WriteLine("set Flexo={0}", TextEditCtrl.Text);
        }

        //--- Property Type ---------------------------------------
        private ECtrlType _Type;
        public ECtrlType Type
        {
            get { return _Type; }
            set 
            { 
                _Type = value;
                TextCtrl.Visibility     = _Type==ECtrlType.ComboStr ? Visibility.Visible : Visibility.Hidden;
                TextEditCtrl.Visibility = _Type==ECtrlType.Text     ? Visibility.Visible : Visibility.Hidden;
                ComboCtrl.Visibility    = _Type==ECtrlType.Combo    ? Visibility.Visible : Visibility.Hidden;
                ImgComboCtrl.Visibility = _Type==ECtrlType.ImgCombo ? Visibility.Visible : Visibility.Hidden;
                CheckBoxCtrl.Visibility = _Type==ECtrlType.CheckBox ? Visibility.Visible : Visibility.Hidden;
                LedCtrl.Visibility      = _Type==ECtrlType.Led      ? Visibility.Visible : Visibility.Hidden;
                if (_Type==ECtrlType.CheckBox && CheckBoxCtrl.Children.Count==0) _checkbox_init();
            }
        }

        //--- Property ItemsSource ---------------------------------------
        public IEnumerable ItemsSource 
        { 
            set 
            {
                ImgComboCtrl.ItemsSource=value;

                ComboCtrl.ItemsSource=value;
                              
                var l = value.GetEnumerator();
                l.Reset();
                l.MoveNext();
                _ComboValueProp = l.Current.GetType().GetProperty("Value");                
            } 
        }        
        
        //--- GetVal --------------------------
        private Int32 GetVal(object item)
        {
            if (_ComboValueProp==null || item==null) return 0;

            try
            {
                return (Int32)_ComboValueProp.GetValue(item);
            }
            catch(Exception)
            {
                return 0;
            }
        }

        //--- Property ReadOnly ---------------------------------------
        public bool ReadOnly
        {
            set 
            { 
                ComboCtrl.IsReadOnly = value;
                if (_Type==ECtrlType.Text)
                {
                    TextEditCtrl.Visibility = value ? Visibility.Hidden  : Visibility.Visible;
                    TextCtrl.Visibility     = value ? Visibility.Visible : Visibility.Hidden;
                }
            }
        }

        //--- Property ContentBack ---------------------------------------
        private Brush _ContentBack;
        public Brush ContentBack
        {
            get { return _ContentBack; }
            set 
            { 
                _ContentBack = value;
                TextEditCtrl.Background  = _ContentBack;
                ComboCtrl.Background = _ContentBack;
            }
        }     
        
        //--- Property ID ---------------------------------------
        private string _ID;
        public string ID    
        {
            get { return _ID; }
            set { _ID = value; }
        }

        //--- Property No ---------------------------------------
        private int _No;
        public int No
        {
            get { return _No; }
            set 
            {             
                _No=value;
                string cat = _ID.Remove(0, 4);
                _Value = RxGlobals.Language.GetString(String.Format("{0} {1:00}", cat, _No));
                Visibility = Visibility.Collapsed;
            }
        }
        
        //--- Property Label ---------------------------------------
        private string _Label;
        public string Label
        {
            get { return _Label; }
            set 
            { 
                if (_Label != value)
                {
                    _Label = value;
                    LABEL.Text=_Label;
                    if (_Label.Equals(""))
                    {
                        MainGrid.ColumnDefinitions[0].Width=new GridLength(0);
                    }
                    else
                    {
                        PlcParPanel panel=_find_panel(this);
                        MainGrid.ColumnDefinitions[0].Width = new GridLength(panel.LabelWidth);
                    }
                }
            }
        }

        //--- Property LabelWidth ---------------------------------------
	    public int LabelWidth
	    {
		    set { MainGrid.ColumnDefinitions[0].Width = new GridLength(value);}
	    }

        //--- Property Unit ---------------------------------------
        public string Unit
        {
            set 
            { 
                _Unit = new CUnit(value);
                string text=value;
                TB_Unit.Text = _Unit.Name;
                TB_Unit.Visibility=Visibility.Visible;
            }
        }

        //--- Property Unit ---------------------------------------
        public string UnitWidth
        {
            set 
            { 
                TB_Unit.Width=Convert.ToDouble(value); 
                TB_Unit.Visibility=Visibility.Visible;
            }
        }

        //--- Property User ---------------------------------------
        private EUserType _UserEnabled = EUserType.usr_operator;
        public EUserType UserEnabled
        {
            get { return _UserEnabled; }
            set
            { 
                RxGlobals.User.PropertyChanged += User_PropertyChanged;
                _UserEnabled = value;
                _setUserType();
            }
        }

        //--- Property User ---------------------------------------
        private EUserType _UserVisible = EUserType.usr_operator;
        public EUserType UserVisible
        {
            get { return _UserVisible; }
            set
            { 
                RxGlobals.User.PropertyChanged += User_PropertyChanged;
                _UserVisible = value;
                _setUserType();
            }
        }

        //--- _setUserType ---------------
        private void _setUserType()
        {
            IsEnabled  = (RxGlobals.User.UserType>=_UserEnabled);
            Visibility = (RxGlobals.User.UserType>=_UserVisible) ? Visibility.Visible : Visibility.Collapsed;
        }

        //--- User_PropertyChanged ----------------------------
        void User_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            _setUserType();            
        }

        //--- Property Format ---------------------------------------
        private char _Format='t';
        //  't': Text
        //  'n': numeric ### ###.##
        public char Format
        {
            set { _Format = value; }
        }
        
        //--- Property Changed ---------------------------------------
        private bool _Changed;
        public bool Changed
        {
            get { return _Changed; }
            set 
            { 
                if (value!=_Changed)
                {
                    _Changed = value;
                    if (_Changed)
                    {
                    //  Debug.WriteLine("Control.Changed");
                        PlcParPanel panel=_find_panel(this);
                        if (panel!=null) panel.Changed=_Changed;
                    }
                }
            }
        }

        //--- Property MinVal ---------------------------------------
        private int _MinVal;
        public int MinVal
        {
            get { return _MinVal; }
            set { _MinVal = value; }
        }
        
        //--- Property _MaxVal ---------------------------------------
        private int _MaxVal;
        public int MaxVal
        {
            get { return _MaxVal; }
            set { _MaxVal = value; }
        }
        
        //--- UpdateValue ----------------------------------
        private bool _UpdateValue = false;
        public void UpdateValue(string value)
        {
            _UpdateValue = true;
            Value = value;
            _UpdateValue = false;
        }

        //--- Property TextValue ---------------------------------------
        private string _Value = "####";
        private string _ValueInit;
        public string Value
        {
            get 
            { 
                return _Value; 
            }

            set 
            {
                if (value==null) return;
                if (value!=_Value)
                {
                    if (_ValueInit==null) _ValueInit=_Value;
                    switch(_Type)
                    {
                    case ECtrlType.Combo:
                    case ECtrlType.ImgCombo:
                        try
                        {
                            if (value.Equals("ERROR"))      ComboValue = (int)TcpIp.INVALID_VALUE;
                            else if (value.Equals("FALSE")) ComboValue = 0;
                            else if (value.Equals("TRUE"))  ComboValue = 1;
                            else                            ComboValue = Rx.StrToInt32(value);
                        }
                        catch(Exception)
                        {
                            ComboValue=0;
                        }
                        break;

                    case ECtrlType.ComboStr:
                        if (ComboCtrl.ItemsSource!=null && value!=null)
                        {
                            int val = Rx.StrToInt32(value); 
                            foreach (var item in ComboCtrl.ItemsSource)
                            {
                                if (GetVal(item)==val)
                                {
                                    try
                                    {
                                        TextCtrl.Text=(item as RxEnum<int>).Display;
                                    }
                                    catch(Exception)
                                    {
                                    }
                                    break;
                                }
                            }
                        }
                        break;

                    case ECtrlType.CheckBox: try{_checkbox_setvalue(Rx.StrToInt32(value));}
                                             catch(Exception) {_checkbox_setvalue(0);}
                                             break;

                    default:
                        try
                        {
                            double factor = 1.0;
                            int digits =1;
                            if (_Unit!=null)
                            {
                                if (TextEditCtrl.Tag!=null && TextEditCtrl.Tag.Equals("RxNumPad"))
                                {
                                    TextEditCtrl.Tag=null;
                                    digits=3;
                                }
                                else if (_UpdateValue) factor=_Unit.Factor;
                                if (factor!=1.0) digits=3;
                            }
                            switch(_Format)
                            {
                                case 'b':
                                        {
                                           UInt32 val = Convert.ToUInt32(value);
                                           char[] str = new char[9];
                                           int i, n;
                                           for (i=0, n=8; i<8; i++)
                                           {
                                               if ((val & (1<<i))!=0) str[n--]='1';
                                               else                   str[n--]='0';
                                               if ((i%4)==3 && n>0) str[n--]=' ';
                                           }
                                           _Value = new String(str); 
                                        }
                                        break;
                                case 'h': _Value = string.Format("{0:X}", Convert.ToInt64(value));  break;
                                case 'f': _Value = Rx.StrNumFormat(value, 3,      factor); break;
                                case '1': _Value = Rx.StrNumFormat(value, digits, factor); break;
                                case 'n': _Value = Rx.StrNumFormat(value, 0,      factor); break;
                                case 'l': _Value = value.Replace(';', '\n'); break;
                                default : _Value = value; break;
                            }
                        }
                        catch(Exception) 
                        { _Value = value;}
                        if (!_UpdateValue) Changed = (value!=_ValueInit);
                        break;
                    }
                    _CallPropertyChanged();
                }
            }
        }

        //--- SendValue ------------------------------
        public string SendValue
        {
            get
            {
                if (_Type==ECtrlType.Combo || _Type==ECtrlType.ImgCombo)
                {
                    return GetVal(ComboCtrl.SelectedItem).ToString(); 
                }
                else
                {
                    if (!TextEditCtrl.Text.Equals("####"))
                    {
                        if (_Format=='b')
                        {
                            int i;
                            UInt32 n=1, val=0;
                            string text = TextEditCtrl.Text;
                            for (i=text.Length-1; i>=0; i--)
                            {
                                if (text[i]=='1')  {val += n; n*=2;}
                                else if (text[i]=='0') n*=2;
                            }
                            return val.ToString(new CultureInfo("en-US"));
                        }
                        else 
                        {
                            string val=TextEditCtrl.Text;
                            if (_Format=='n')
                            {   // remove formatting spaces, commas (very special!)
                                for (int i=0; i<val.Length; )
                                {
                                    if ((int)val[i]==160) val=val.Remove(i,1);
                                    if ((int)val[i]==27) val=val.Remove(i,1);
                                    if ((val[i]<'0' || val[i]>'9') && val[i]!='-' && val[i]!='.' && val[i]!=',') val=val.Remove(i,1);
                                    else i++;
                                }
                            }
                            if (_Unit==null || _Unit.Factor==1.0) return val.Replace(',', '.');
                            return val.Replace(',', '.');
                        }
                    }
                }
                return null;
            }
        }

        //--- Property ComboValue ---------------------------------------
        private int _ComboValue;
        private int? _ComboValueInit;
        public int? ComboValue
        {
            get 
            { 
                if ((_Type==ECtrlType.Combo || _Type==ECtrlType.ImgCombo) && _ComboValue!=TcpIp.INVALID_VALUE) return _ComboValue; 
                else return null;
            }

            set 
            {       
                if (value!=_ComboValue)
                {
                    if (_ComboValueInit==null) _ComboValueInit=value;
                    _ComboValue = (int)value;

                    if (ComboCtrl.ItemsSource!=null && value!=null)
                    {
                        foreach(var item in ComboCtrl.ItemsSource)
                        {
                            if (GetVal(item)==value)
                            {
                                ComboCtrl.SelectedItem=item;
                                break;
                            }
                        }
                    }

                    _CallPropertyChanged();
                    if (!_UpdateValue) Changed = (value!=_ComboValueInit);
                }
            }
        }        

        //--- Send ------------------------------------------------------
        public void Send(PlcParPanel panel)
        {
            if (panel==null) panel = (this as Control).Parent as PlcParPanel;
            string str=null;
            if (panel!=null)
            {
                if (_Type==ECtrlType.Combo || _Type==ECtrlType.ImgCombo) 
                    str = string.Format("{0}\n{1}={2}\n", panel.UnitID, ID, SendValue);
                else
                {
                    if (!TextEditCtrl.Text.Equals("####"))
                    {
                        if (_Format=='b')
                        {
                            int i;
                            UInt32 n=1, val=0;
                            string text = TextEditCtrl.Text;
                            for (i=text.Length-1; i>=0; i--)
                            {
                                if (text[i]=='1')  {val += n; n*=2;}
                                else if (text[i]=='0') n*=2;
                            }
                            str = string.Format("{0}\n{1}={2}\n", panel.UnitID, ID, val);
                        }
                        else 
                        {
                            if (_Format=='n' || _Format=='1' || _Format=='f')
                            {   
                                double val=Rx.StrToDouble(TextEditCtrl.Text);
                                if (_Unit!=null) val /= _Unit.Factor;
                                str = string.Format("{0}\n{1}={2}\n", panel.UnitID, ID, val.ToString(new CultureInfo("en-US")));
                            }
                            else str = string.Format("{0}\n{1}={2}\n", panel.UnitID, ID, TextEditCtrl.Text);
                        }
                    }
                }
	            if (str!=null) 
                {
            //      Console.WriteLine("PlcPar.send>>{0}<<", str);
                    RxGlobals.RxInterface.SendMsgBuf(TcpIp.CMD_PLC_SET_VAR, str);         
                }
            }
            Changed = false;
            _ValueInit      = null; // _Value;
            _ComboValueInit = null; // _ComboValue;
        }

        //--- _find_panel -----------------------------------------
        private PlcParPanel _find_panel(object obj)
        {
            while (obj!=null)
            {
                PlcParPanel panel  = obj as PlcParPanel;
                if (panel!=null) return panel;
                if (obj as Control!=null) obj=(obj as Control).Parent;
                else if (obj as Grid!=null) obj=(obj as Grid).Parent;
                else if (obj as StackPanel!=null) obj=(obj as StackPanel).Parent;
            }
            return null;
        }

        //--- ImgComboCtrl_SelectionChanged ------------------------
        private void ImgComboCtrl_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (!_UpdateValue && e.RemovedItems.Count>0)
            {
                Changed = true;
            }
        }

        //--- ComboCtrl_SelectedItemChanged ---------------------------------------
        private void ComboCtrl_SelectedItemChanged(object sender, RoutedPropertyChangedEventArgs<object> e)
        {
            if (!_UpdateValue &&  _Type==ECtrlType.Combo && e.OldValue!=null) Changed=true;
        }

        //--- TextEditCtrl_TouchUp --------------------------------------------------
        /*
        private void TextEditCtrl_TouchUp(object sender, System.Windows.Input.TouchEventArgs e)
        {
            Changed=true;
        }
        */

        /*
        //--- ImgComboCtrl_PreviewMouseDown -------------------------------------------------
        private void ImgComboCtrl_PreviewMouseDown(object sender, System.Windows.Input.MouseButtonEventArgs e)
        {
            RxComboBox ctrl = sender as RxComboBox;
            if (ctrl!=null) CmdPopup.Open(ctrl);
            e.Handled = true;
        }

        //--- ImgCombo_Clicked -------------------------------------------------------------
        private void ImgCombo_Clicked(object sender, System.Windows.Input.MouseButtonEventArgs e)
        {
            FrameworkElement ctrl = sender as FrameworkElement;
            if (ctrl!=null) 
            {
                ImgComboCtrl.SelectedIndex = Rx.StrToInt32(ctrl.Tag);
            }
            CmdPopup.IsOpen = false;
            e.Handled = true;
        }
         * */
    }
}
