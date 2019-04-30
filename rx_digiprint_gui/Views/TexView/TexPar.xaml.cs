using RX_Common;
using RX_DigiPrint.Models;
using RX_DigiPrint.Models.Enums;
using RX_DigiPrint.Services;
using RX_DigiPrint.Views.UserControls;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace RX_DigiPrint.Views.TexView
{
    /// <summary>
    /// Interaction logic for MainPar.xaml
    /// </summary>
    public partial class TexPar : UserControl
    {
        public TexPar()
        {
            InitializeComponent();

            CB_Material.ItemsSource   = RxGlobals.MaterialList.List;
            CB_RotUW.ItemsSource      = CB_RotRW.ItemsSource = new EN_RotationList();

            XML_MATERIAL.PropertyChanged += XML_MATERIAL_PropertyChanged;
            RxGlobals.Timer.TimerFct += Timer;
        }

        //--- Save_Clicked ---------------------------------------------
        private void Save_Clicked(object sender, RoutedEventArgs e)
        {
            ParPanelMaterial.Send();
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_PLC_SAVE_PAR);
        }

        //--- Save_Default_Click ---------------------------------------------
        private void Save_Default_Click(object sender, RoutedEventArgs e)
        {
            Save_Clicked(sender, e);
            Material material = RxGlobals.MaterialList.FindByName(XML_MATERIAL.Value);
            if (material!=null)
            {
                ParPanelMaterial.SaveValues(material);
                material.Send(TcpIp.CMD_PLC_SAVE_MATERIAL);
            }
        }

        //--- Reload_Clicked ---------------------------------------------
        private void Reload_Clicked(object sender, RoutedEventArgs e)
        {
            ParPanelMaterial.Reset();
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
         //   ParPanel1.Editing=true;
         //   ParPanelMaterial.Editing=true;
        }

        //--- CB_Material_DropDownClosed ----------------------------------------------
        private void CB_Material_DropDownClosed(object sender, RoutedEventArgs e)
        {
            Material item = CB_Material.SelectedItem as Material;
            if (item!=null)
            {
                if (item.Name.Equals("--- NEW ---"))
                {
                    MaterialName.Visibility = Visibility.Visible;
                    RxTextPad pad = new RxTextPad(MaterialName);
                
                    bool? result=pad.ShowDialog();
                    MaterialName.Visibility = Visibility.Collapsed;
                    if (result!=null && (bool)result == true)
                    {
                        XML_MATERIAL.Value = pad.Result;
                        Material newMaterial = new Material(){Name = pad.Result};
                        ParPanelMaterial.SaveValues(newMaterial);
                        RxGlobals.MaterialList.AddItem(newMaterial);
                        CB_Material.SelectedItem = newMaterial;
                    }
                }
                else
                {
                    XML_MATERIAL.Value = item.Name;
                    ParPanelMaterial.SetValues(item);
                }
            }
        }

        private int recursive=0;
        private void CB_Material_SelectedItemChanged(object sender, RoutedPropertyChangedEventArgs<object> e)
        {
            if (recursive>0)
            {
                recursive--;
                return;
            }
            Material material = e.NewValue as Material;
            if (material==null) 
            {
                recursive = 2;
                CB_Material.SelectedItem = RxGlobals.MaterialList.List.First();
                CB_Material.SelectedItem = null;
            }
        }
      
        //--- XML_MATERIAL_PropertyChanged --------------------------------------------
        private void XML_MATERIAL_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            Material material = RxGlobals.MaterialList.FindByName(XML_MATERIAL.Value);
            CB_Material.SelectedItem = material;
        }

                        
        //--- Timer ---------------------------------------------
        private bool _speedcorrection = false;
        private void Timer(int no)
        {
            string str = "Application.GUI_00_001_Main"+"\n"+ "PAR_SPEED_CORRECTION_RW"+"\n";
            RxGlobals.RxInterface.SendMsgBuf(TcpIp.CMD_PLC_GET_VAR, str);

            str = RxGlobals.Plc.GetVar("Application.GUI_00_001_Main", "PAR_SPEED_CORRECTION_RW");
            bool sc = str!=null && !str.Equals("ERROR");
            if (str!=null && sc!=_speedcorrection)
            {
                if (sc)
                {
                    RW_Flexibility.ID="PAR_SPEED_CORRECTION_RW";
                    RW_Flexibility.Label="Speed Offset";
                    RW_Flexibility.Unit="%";
                    RW_Tension.Unit = "%";
                }
                else
                {
                    RW_Flexibility.ID="PAR_MATERIAL_FLEXIBILITY_RW";
                    RW_Flexibility.Label="Mat Flexibility";
                    RW_Flexibility.Unit="";
                    RW_Tension.Unit = "N";
                }
                _speedcorrection = sc;
            }
        }
     }
}
