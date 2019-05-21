using RX_Common;
using RX_DigiPrint.Models;
using RX_DigiPrint.Services;
using RX_DigiPrint.Views.UserControls;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;

namespace RX_DigiPrint.Views.UserControls
{
    public interface IPlcParPanel
    {
         void Send();
         void Reset();
    }

    public class PlcParPanel : StackPanel, INotifyPropertyChanged, IPlcParPanel
    {       
        public event PropertyChangedEventHandler PropertyChanged;
        public event Action                      Update;

        //--- Constructor ---------------------------------------
        public PlcParPanel()
        {
            RxGlobals.Plc.PropertyChanged += Plc_PropertyChanged;
            base.Loaded += PlcParPanel_Loaded;
            RxGlobals.Timer.TimerFct +=  Tick;
        }
        
        //--- destructor -----------------------------------------
        ~PlcParPanel()
        {
            RxGlobals.Plc.PropertyChanged -= Plc_PropertyChanged;
        }

        //--- _CallPropertyChanged --------------------------------------------------------------
        private void _CallPropertyChanged([CallerMemberName]String info = null)
        {
            if (PropertyChanged != null)
                PropertyChanged(this, new PropertyChangedEventArgs(info));
        }
        
        //--- _children_changed -----------------------------------------------
        private bool _children_changed(UIElementCollection children)
        {
            foreach(var item in children)
            {
                PlcParCtrl par = item as PlcParCtrl;
                if (par!=null)
                {
                     if (par.Changed) return true;
                } 
                else
                {
                    Grid grid = item as Grid;
                    if (grid!=null)
                    {
                        if(_children_changed(grid.Children)) return true;
                    }
                }                    
            }
            return false;
        }

        //--- Property Changed ---------------------------------------
        private bool _Changed;
        public bool Changed
        {
            get 
            { 
           //     _Changed = _children_changed(Children);
                return _Changed;
            }
            
            set 
            { 
                if (value!=_Changed)
                {
                    if (value)
                        Debug.WriteLine("Panel.Changed");
                    else 
                        Debug.WriteLine("Panel.Changed RESET");
                    _Changed =value;
                    _CallPropertyChanged();
                }
            }
        }
                
        //--- 
        private void _send_children(UIElementCollection children)
        {
            foreach(var item in children)
            {
                PlcParCtrl ctrl = item as PlcParCtrl;
                if (ctrl!=null) 
                    ctrl.Send(this);
                else
                {
                    StackPanel panel = item as StackPanel;
                    if (panel!=null) _send_children(panel.Children);
                    else
                    {
                        Grid grid = item as Grid;
                        if (grid!=null) _send_children(grid.Children);
                    }
                }
            }            
        }

        //--- Send ----------------------------------
        public void Send()
        {
            _send_children(Children);
            string str = string.Format("{0}\n{1}={2}\n", UnitID, "CMD_SET_PARAMETER", 1);
            Changed=false;
            RxGlobals.RxInterface.SendMsgBuf(TcpIp.CMD_PLC_SET_VAR, str);
        }

        //--- PlcParPanel_Loaded -----------------------------------
        void PlcParPanel_Loaded(object sender, RoutedEventArgs e)
        {
            int i=0;
            foreach(var item in Children)
            {
                PlcParCtrl par = item as PlcParCtrl;
                if (par!=null) 
                {
                    par.ReadOnly = _ReadOnly;
                    if (i==0) par.Background=Application.Current.Resources["XamGrid_Alternate"] as Brush;
                    else      par.Background=Brushes.Transparent;
                    i = 1-i;
                }
            }
            Refresh();
        }

        //--- Tick ---------------------------------------------------
        private void Tick(int no)
        {
            if (IsVisible && !Changed)
            {
                Refresh();
            }
        }

        //--- Property ReadOnly ---------------------------------------
        private bool _ReadOnly;
        public bool ReadOnly
        {
            get { return _ReadOnly; }
            set { _ReadOnly = value;}
        }

        //--- Property ChildPadding ---------------------------------------
        private Thickness _ChildPadding;
        public Thickness ChildPadding
        {
            get { return _ChildPadding; }
            set {_ChildPadding=value; }
        }        

        //--- Plc_PropertyChanged --------------------------------
        void Plc_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            if (e.PropertyName.Equals("ALL_PLC_PAR"))
            {
                Refresh();
            }
            else if (e.PropertyName.Equals(UnitID))
            {
                _update_controls();
            }
        }

        //--- Property UnitID ---------------------------------------
        private string _UnitID;
        public string UnitID
        {
            get { return _UnitID; }
            set { _UnitID = value; }
        }

        //--- Property LabelWidth ---------------------------------------
        private double _LabelWidth=150;
        public double LabelWidth
        {
            get { return _LabelWidth; }
            set { _LabelWidth = value; }
        }
        
        //--- _refresh_children ----------------------
        private bool _refresh_children(UIElementCollection children, ref StringBuilder str)
        {
            bool send=false;
            string id="";

            foreach(var item in children)
            {
                PlcParCtrl par = item as PlcParCtrl;
                if (par!=null)
                {    
                    if (!par.ID.Equals(id))
                    {
                        id = par.ID;
                        par.Changed = false;
                        str.AppendFormat("{0}\n", par.ID);
                    }                
                    send=true;
                }
                else
                {
                    StackPanel panel = item as StackPanel;
                    if (panel!=null) send |= _refresh_children(panel.Children, ref str);
                    else
                    {
                        Grid grid = item as Grid;
                        if (grid!=null) send |=_refresh_children(grid.Children, ref str);
                    }
                }     
            }
            return send;      
        }

        //--- Reset ------------------------------------------
        public void Reset()
        {
            Changed=false;
            Refresh();
        }

        //--- Refresh ---------------------------------------
        public void Refresh()
        {
            if (!RxGlobals.Plc.Connected) return;
            StringBuilder str = new StringBuilder(2048);
            bool send=false;

            str.AppendFormat("{0}\n", UnitID);
            send = _refresh_children(Children, ref str);
            if (send) RxGlobals.RxInterface.SendMsgBuf(TcpIp.CMD_PLC_GET_VAR, str.ToString());
        }

        //--- SaveValues ----------------------------------------------
        private void _saveValuesChildren(Material material, UIElementCollection children)
        {
            foreach(var item in children)
            {
                PlcParCtrl par = item as PlcParCtrl;
                if (par!=null) 
                {
                    material.SaveValue(par.ID, par.SendValue);
                }
                else
                {
                    Grid grid=item as Grid;
                    if (grid!=null) _saveValuesChildren(material, grid.Children);
                }
            }
            
        }

        public void SaveValues(Material material)
        {
            _saveValuesChildren(material, Children);
        }

        //--- SetValues ----------------------------------------------
        private void _SetValuesChildren(Material material, UIElementCollection children)
        {
            foreach(var item in children)
            {
                PlcParCtrl par = item as PlcParCtrl;
                if (par!=null)
                {
                    string val= material.Value(par.ID);
                    par.Value = val;
                    // _Editing = true;
                }
                else
                {
                    Grid grid = item as Grid;
                    if (grid!=null) _SetValuesChildren(material, grid.Children);
                }
            }
        }

        public void SetValues(Material material)
        {
            _SetValuesChildren(material, Children);
        }


        //--- _update_children --------------------
        private void _update_children(UIElementCollection children)
        {
            foreach(var item in children)
            {
                PlcParCtrl par = item as PlcParCtrl;
                if (par!=null)
                {
                    string str = RxGlobals.Plc.GetVar(UnitID, par.ID);
                    par.UpdateValue(str);
                    continue;
                }

                StackPanel panel = item as StackPanel;
                if (panel!=null) 
                {
                    _update_children(panel.Children); 
                    continue;
                }

                Grid grid = item as Grid;
                if (grid!=null) 
                {
                    _update_children(grid.Children); 
                    continue;
                }
            }           
        }

        //--- _update_controls -----------
        private void _update_controls()
        {
//            if (!_refresh || _Changed) return;
            if (_Changed) return;
            _update_children(Children);
            if (Update!=null) Update();
        }

    }
}
