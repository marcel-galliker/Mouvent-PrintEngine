using RX_Common;
using RX_DigiPrint.Models;
using RX_DigiPrint.Services;
using RX_DigiPrint.Models.Enums;
using RX_DigiPrint.Views.UserControls;
using System;
using System.Linq;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;

namespace RX_DigiPrint.Views.LB702WBView
{
    /// <summary>
    /// Interaction logic for MainPar.xaml
    /// </summary>
    public partial class LB702WB_ParMain : UserControl
    {
        public Action SendParameters;
        public Action ResetParameters;

        private int _MaterialSelected = -1;
        public LB702WB_ParMain()
        {
            InitializeComponent();

            CB_Material.ItemsSource         = RxGlobals.MaterialList.List;
            CB_RotUW.ItemsSource = CB_RotUW.ItemsSource = new EN_RotationList();
            CB_RotRW.ItemsSource = CB_RotUW.ItemsSource = new EN_RotationList();
            CB_Corona.ItemsSource = new EN_OnOff();
            
            XML_MATERIAL.PropertyChanged   += XML_MATERIAL_PropertyChanged;

            CB_Mode_Coating.ItemsSource = new EN_OnOffAuto();
            CB_Mode_Print.ItemsSource = new EN_OnOffAuto();
            CB_Mode_Flexo.ItemsSource = new EN_OnOffAuto();
        }

        //--- Save_Clicked ---------------------------------------------
        private void Save_Clicked(object sender, RoutedEventArgs e)
        {
            CB_Material.EndEditMode(true, true);
            ParPanelMaterial.Send();
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_PLC_SAVE_PAR);

        //  Material material = CB_Material.SelectedItem as Material;
            Material material = RxGlobals.MaterialList.FindByName(CB_Material.DisplayText);
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
            if (ResetParameters!=null) ResetParameters();
        }

        //--- Delete_Clicked ---------------------------------------------
        private void Delete_Clicked(object sender, RoutedEventArgs e)
        {
        //  Material material = CB_Material.SelectedItem as Material;
            Material material = RxGlobals.MaterialList.FindByName(CB_Material.DisplayText);
            if (material!=null)
            {
                if (RxMessageBox.YesNo("Delte", string.Format("Delete Material {0}?", material.Name),  MessageBoxImage.Question, false))
                {
                    material.Delete();
                    // RxGlobals.MaterialList.DeleteByName(XML_MATERIAL.Value);
                    // CB_Material.SelectedItem = null;
                }
            }
        }

        //--- ParPanel_PreviewKeyDown --------------------------------
        private void ParPanel_PreviewKeyDown(object sender, KeyEventArgs e)
        {
        //    ParPanelMaterial.Editing=true;
        }

        //--- CB_Material_DropDownClosed ----------------------------------------------
        private void CB_Material_DropDownClosed(object sender, RoutedEventArgs e)
        {
        //  Material material = CB_Material.SelectedItem as Material;
            Material material = RxGlobals.MaterialList.FindByName(CB_Material.DisplayText);
            if (material!=null)
            {
                if (material.Name.Equals("--- NEW ---"))
                {
                    MaterialName.Visibility = Visibility.Visible;
                    RxTextPad pad = new RxTextPad(MaterialName);
                    pad.Show();
                
                    bool? result=pad.ShowDialog();
                    MaterialName.Visibility = Visibility.Collapsed;
                    if (result!=null && (bool)result == true)
                    {
                        XML_MATERIAL.Value = pad.Result;
                        Material newMaterial = new Material(){Name = pad.Result};
                        ParPanelMaterial.SaveValues(newMaterial);
                        RxGlobals.MaterialList.AddItem(newMaterial);
                    }
                }
                else
                {
                    XML_MATERIAL.Value = material.Name;
                    ParPanelMaterial.SetValues(material);
            }
            }            
        }

        //--- CB_Material_SelectedItemChanged --------------------------------------------------
        private void CB_Material_SelectedItemChanged(object sender, RoutedPropertyChangedEventArgs<object> e)
        {
            if (CB_Material.SelectedIndex == -1) CB_Material.SelectedIndex = _MaterialSelected;
            if (e.NewValue == null) CB_Material.SelectedItem = e.OldValue as Material;
            else
            {
                CB_Material.SelectedItem = e.NewValue as Material;
                _MaterialSelected = CB_Material.SelectedIndex;
            }
        }
      
        //--- XML_MATERIAL_PropertyChanged --------------------------------------------
        private void XML_MATERIAL_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            Material material = RxGlobals.MaterialList.FindByName(XML_MATERIAL.Value);
            CB_Material.SelectedItem = material;
        }
     }
}
