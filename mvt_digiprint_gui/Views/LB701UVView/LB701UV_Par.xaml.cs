using RX_Common;
using RX_DigiPrint.Models;
using RX_DigiPrint.Models.Enums;
using RX_DigiPrint.Services;
using System.Linq;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;

namespace RX_DigiPrint.Views.LB701UVView
{
    /// <summary>
    /// Interaction logic for MainPar.xaml
    /// </summary>
    public partial class LB701UV_Par : UserControl
    {
        public LB701UV_Par()
        {
            InitializeComponent();

            CB_Material.ItemsSource   = RxGlobals.MaterialList.List;

            CB_RotUW.ItemsSource      = new EN_RotationList();
            CB_RotRW.ItemsSource      = new EN_RotationList();
            CB_Corona.ItemsSource     = new EN_OnOff();
            CB_Pinning1.ItemsSource   = new EN_OnOffAuto();
            CB_Pinning2.ItemsSource   = new EN_OnOffAuto();
            CB_UVLamp.ItemsSource     = new EN_OnOffAuto();

            CB_DieMode.ItemsSource      = new EN_OnOff();
            CB_RW_WASTE.ItemsSource     = new EN_OnOff();
            CB_RW_DIECUT.ItemsSource    = new EN_OnOff();
            RxGlobals.PrintSystem.PropertyChanged += PrintSystem_PropertyChanged;
            XML_MATERIAL.PropertyChanged          += XML_MATERIAL_PropertyChanged;
			ParPanelMaterial.Update               += ParPanelMaterial_Update;
        }

		//--- ParPanelMaterial_Update -------------------------------------------
		private bool? _DieCutUsed;
		private void ParPanelMaterial_Update()
		{
            string str  = RxGlobals.Plc.GetVar(ParPanelMaterial.UnitID, "CFG_DIECUT");
            bool used=(str!=null && str.Equals("TRUE"));
            #if DEBUG
			    used=true;
            #endif
            if (used!=_DieCutUsed)
			{
                _DieCutUsed = used;
                if ((bool)_DieCutUsed) CB_RotRW.ItemsSource = new EN_RotationList_OFF();
                else                   CB_RotRW.ItemsSource = new EN_RotationList();
                COL_DieCutSep.Width  = (bool)_DieCutUsed? new GridLength(20) : new GridLength(0);
                COL_DieCut.Width     = (bool)_DieCutUsed? GridLength.Auto    : new GridLength(0);
                Grid_RW.Visibility   = (bool)_DieCutUsed? Visibility.Collapsed : Visibility.Visible;
                Reload_Clicked(this, null);
			}
		}

		//--- PrintSystem_PropertyChanged ----------------------------------------------------
		void PrintSystem_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
        //    if (e.PropertyName.Equals("SpeedList"))
        //         CB_Speed.ItemsSource  = RxGlobals.PrintSystem.SpeedList;
        }

        //--- Save_Clicked ---------------------------------------------
        private void Save_Clicked(object sender, RoutedEventArgs e)
        {
            CB_Material.EndEditMode(true, true);
            ParPanelMaterial.Send();
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_PLC_SAVE_PAR);
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
        }

        //--- Delete_Clicked ---------------------------------------------
        private void Delete_Clicked(object sender, RoutedEventArgs e)
        {
            Material material = RxGlobals.MaterialList.FindByName(CB_Material.DisplayText);
            if (material!=null)
            {
                if (MvtMessageBox.YesNo("Delte", string.Format("Delete Material {0}?", material.Name),  MessageBoxImage.Question, false))
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
            Material material = RxGlobals.MaterialList.FindByName(CB_Material.DisplayText);
            if (material != null)
            {
                if (material.Name.Equals("--- NEW ---"))
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
                    XML_MATERIAL.Value = material.Name;
                    ParPanelMaterial.SetValues(material);
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
