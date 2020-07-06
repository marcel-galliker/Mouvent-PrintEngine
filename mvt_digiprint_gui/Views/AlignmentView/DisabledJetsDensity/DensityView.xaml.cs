﻿using RX_Common;
using RX_DigiPrint.Models;
using RX_DigiPrint.Services;
using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Windows;
using System.Windows.Controls;

namespace RX_DigiPrint.Views.Density
{
    /// <summary>
    /// Interaction logic for DensityView.xaml
    /// </summary>
    public partial class DensityView : UserControl
    {
        private Models.Density _Density = RxGlobals.Density;

        public DensityView()
        {
            InitializeComponent();
            DataContext = _Density;
        }

        //--- Save_Clicked ---------------------------------------------
        public void Save_Clicked(object sender, RoutedEventArgs e)
        {
            _Density.Save();
        }

        private void LoadFile_Clicked(object sender, RoutedEventArgs e)
        {
            TcpIp.SDensityValuesMsg msg = new TcpIp.SDensityValuesMsg();
            msg.values.head = _Density.GetHeadNumber();
            RxGlobals.RxInterface.SendMsg(TcpIp.CMD_GET_DENSITY_VAL, ref msg);
        }

        private void DensityValueBox_GotFocus(object sender, RoutedEventArgs e)
        {
            var tmpSender = sender as MvtNumBox;
            var text = tmpSender.Text;
        }

        private void DensityValueBox_LostFocus(object sender, RoutedEventArgs e)
        {
            var tmpSender = sender as MvtNumBox;
            var text = tmpSender.Text;
        }
    }
}
