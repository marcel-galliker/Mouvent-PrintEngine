using Infragistics.Windows.Editors;
using RX_Common;
using RX_DigiPrint.Models;
using RX_DigiPrint.Models.Enums;
using RX_DigiPrint.Services;
using System;
using System.Diagnostics;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;

namespace RX_DigiPrint.Views.CleafView
{
    /// <summary>
    /// Interaction logic for CleafSettings.xaml
    /// </summary>
    public partial class CleafSettings : UserControl
    {
        private int _LengthReset=0;

        public CleafSettings()
        {
            InitializeComponent();

            CB_ProcessPar.ItemsSource   = RxGlobals.MaterialList.List;

            CB_WinderReduction.ItemsSource = new EN_CleafWinderReduction();
            CB_Cleaner.ItemsSource        = new EN_OnOff();
            CB_IrHeater.ItemsSource       = new EN_OnOff();
            CB_Supervision.ItemsSource    = new EN_OnOff();
            
            CB_Lamp1.ItemsSource          = new EN_OnOff();
            CB_Lamp2.ItemsSource          = new EN_OnOff();
            CB_Lamp3.ItemsSource          = new EN_OnOff();
            CB_Lamp4.ItemsSource          = new EN_OnOff();
            CB_Lamp5.ItemsSource          = new EN_OnOff();
            CB_PinnningLamp.ItemsSource   = new EN_OnOff();
            CB_Corona.ItemsSource         = new EN_OnOff();

            CB_SpliceType.ItemsSource     = new EN_CleafSpliceType();
            CB_ManualSplice.ItemsSource   = new EN_CleafManualSplice();

            XML_MATERIAL.PropertyChanged    += XML_MATERIAL_PropertyChanged;
            ProcessParPanel.PropertyChanged += ProcessParPanel_PropertyChanged;
        }

        void ProcessParPanel_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            if (e.PropertyName.Equals("Changed"))
            {
                SaveButton.IsChecked = ProcessParPanel.Changed;
            }
        }

        //--- _check_settings ----------------------------------------
        private void _check_settings()
        {
            try
            {
                int flexo = Rx.StrToInt32(CB_Flexo.Value);
                if ((flexo & 0x01)!=0) CB_Lamp1.ComboValue=1;
                if ((flexo & 0x02)!=0) CB_Lamp2.ComboValue=1;
                if ((flexo & 0x04)!=0) CB_Lamp4.ComboValue=1;
                if ((flexo & 0x08)!=0) CB_Lamp5.ComboValue=1;
            }
            catch(Exception)
            {
            }
            if (CB_Lamp1.ComboValue>0 && Rx.StrToInt32(UVPower1.Value)<30) UVPower1.Value="30"; 
            if (CB_Lamp2.ComboValue>0 && Rx.StrToInt32(UVPower2.Value)<30) UVPower2.Value="30"; 
            if (CB_Lamp3.ComboValue>0 && Rx.StrToInt32(UVPower3.Value)<30) UVPower3.Value="30"; 
            if (CB_Lamp4.ComboValue>0 && Rx.StrToInt32(UVPower4.Value)<30) UVPower4.Value="30"; 
            if (CB_Lamp5.ComboValue>0 && Rx.StrToInt32(UVPower5.Value)<30) UVPower5.Value="30"; 
        }

        //--- Save_Clicked ---------------------------------------------
        private void Save_Clicked(object sender, RoutedEventArgs e)
        {
            _check_settings();
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_PLC_SAVE_PAR);
            RxGlobals.Plc.SetVar("PAR_WINDER_WEB_LENGHT_RESET", _LengthReset);
            ProcessParPanel.Send();
            _LengthReset=0;
        }

        //--- Save_Default_Click ---------------------------------------------
        private void Save_Default_Click(object sender, RoutedEventArgs e)
        {
            _check_settings();
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_PLC_SAVE_PAR);
            RxGlobals.Plc.SetVar("PAR_WINDER_WEB_LENGHT_RESET", _LengthReset);
            ProcessParPanel.Send();
            _LengthReset=0;

            Material material = RxGlobals.MaterialList.FindByName(XML_MATERIAL.Value);
            if (material!=null)
            {
                ProcessParPanel.SaveValues(material);
                material.Send(TcpIp.CMD_PLC_SAVE_MATERIAL);
            }
        }

        //--- Reload_Clicked ---------------------------------------------
        private void Reload_Clicked(object sender, RoutedEventArgs e)
        {
            ProcessParPanel.Reset();
        }
                
        //--- Delete_Clicked ---------------------------------------------
        private void Delete_Clicked(object sender, RoutedEventArgs e)
        {
            Material  material = RxGlobals.MaterialList.FindByName(XML_MATERIAL.Value);
            if (material!=null)
            {
                if (RxMessageBox.YesNo("Delte", string.Format("Delete Material >>{0}<<?", material.Name),  MessageBoxImage.Question, false))
                {
                    material.Delete();
                }
            }
        }

        //--- ParPanel_PreviewKeyDown --------------------------------
        private void ParPanel_PreviewKeyDown(object sender, KeyEventArgs e)
        {
            ProcessParPanel.Changed=true;
        }

        //--- CB_ProcessPar_DropDownClosed ----------------------------------------------
        private void CB_ProcessPar_DropDownClosed(object sender, RoutedEventArgs e)
        {
            Material item = CB_ProcessPar.SelectedItem as Material;
            if (item!=null)
            {
                if (item.Name.Equals("--- NEW ---"))
                {
                    ProcessParName.Visibility = Visibility.Visible;
                    RxTextPad pad = new RxTextPad(ProcessParName);
                
                    bool? result=pad.ShowDialog();
                    ProcessParName.Visibility = Visibility.Collapsed;
                    if (result!=null && (bool)result == true)
                    {
                        XML_MATERIAL.Value = pad.Result;
                        Material newMaterial = new Material(){Name = pad.Result};
                        ProcessParPanel.SaveValues(newMaterial);
                        RxGlobals.MaterialList.AddItem(newMaterial);
                        CB_ProcessPar.SelectedItem = newMaterial;
                    //    ProcessParPanel.Editing = true;
                    }
                }
                else
                {
                    XML_MATERIAL.Value = item.Name;
                    ProcessParPanel.SetValues(item);
                }
            }
        }

        //--- XML_MATERIAL_PropertyChanged --------------------------------------------
        private void XML_MATERIAL_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            // if (!ProcessParPanel.Editing) 
            CB_ProcessPar.SelectedItem = RxGlobals.MaterialList.FindByName(XML_MATERIAL.Value);
        }

        //--- Grid_Loaded -------------------------
        private void Grid_Loaded(object sender, RoutedEventArgs e)
        {
            Grid grid = sender as Grid;
            int i;
            for (i=0; i<ProcessParPanel.Children.Count; i++)
            {
                if (ProcessParPanel.Children[i].Equals(sender))
                {
                    if ((i&1)==0) grid.Background=Application.Current.Resources["XamGrid_Alternate"] as Brush;
                    else          grid.Background=Brushes.Transparent;
                }
            }
        }

        //--- Reset_Length_R_Clicked ---------------------------
        private void Reset_Length_R_Clicked(object sender, RoutedEventArgs e)
        {
            _LengthReset |= 0x0004;
        }

        //--- Reset_Length_L_Clicked ---------------------------
        private void Reset_Length_L_Clicked(object sender, RoutedEventArgs e)
        {
            _LengthReset |= 0x0008;
        }

    }
}
