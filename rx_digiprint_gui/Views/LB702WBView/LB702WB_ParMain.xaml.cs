﻿using RX_Common;
using RX_DigiPrint.Models;
using RX_DigiPrint.Services;
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

        public LB702WB_ParMain()
        {
            InitializeComponent();

            CB_Material.ItemsSource   = RxGlobals.MaterialList.List;
        }

        //--- Save_Clicked ---------------------------------------------
        private void Save_Clicked(object sender, RoutedEventArgs e)
        {
            ParPanelMaterial.Send();
            if (SendParameters!=null) SendParameters();
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_PLC_SAVE_PAR);
        }

        //--- Save_Default_Click ---------------------------------------------
        private void Save_Default_Click(object sender, RoutedEventArgs e)
        {
            ParPanelMaterial.Send();
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_PLC_SAVE_PAR);
            Material material = CB_Material.SelectedItem as Material;
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
            Material  material = CB_Material.SelectedItem as Material;
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
     }
}
