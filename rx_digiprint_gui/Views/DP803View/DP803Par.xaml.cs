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

namespace RX_DigiPrint.Views.DP803View
{
    /// <summary>
    /// Interaction logic for MainPar.xaml
    /// </summary>
    public partial class DP803Par : UserControl
    {
        public DP803Par()
        {
            InitializeComponent();

            CB_Material.ItemsSource   = RxGlobals.MaterialList.List;
            CB_RotUW.ItemsSource      = CB_RotRW.ItemsSource = new EN_RotationList();

            CB_LampModeR.ItemsSource   = new EN_OnOffAuto();
            CB_LampModeV.ItemsSource   = new EN_OnOffAuto();

            XML_MATERIAL.PropertyChanged          += XML_MATERIAL_PropertyChanged;
        }

        //--- Save_Clicked ---------------------------------------------
        private void Save_Clicked(object sender, RoutedEventArgs e)
        {
            CB_Material.EndEditMode(true, true);
            ParPanelMaterial.Send();
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_PLC_SAVE_PAR);
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

        //--- CB_Material_SelectedItemChanged --------------------------------------------------
        private void CB_Material_SelectedItemChanged(object sender, RoutedPropertyChangedEventArgs<object> e)
        {
            if (e.NewValue==null) CB_Material.SelectedItem = e.OldValue as Material;
            else                  CB_Material.SelectedItem = e.NewValue as Material;
        }

        //--- XML_MATERIAL_PropertyChanged --------------------------------------------
        private void XML_MATERIAL_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            Material material = RxGlobals.MaterialList.FindByName(XML_MATERIAL.Value);
            CB_Material.SelectedItem = material;
        }

        //--- Grid_Loaded -------------------------
        private void Grid_Loaded(object sender, RoutedEventArgs e)
        {
            Grid grid = sender as Grid;
            int i;
            for (i=0; i<ParPanelMaterial.Children.Count; i++)
            {
                if (ParPanelMaterial.Children[i].Equals(sender))
                {
                    if ((i&1)==0) grid.Background=Application.Current.Resources["XamGrid_Alternate"] as Brush;
                    else          grid.Background=Brushes.Transparent;
                }
            }
        }

     }
}
