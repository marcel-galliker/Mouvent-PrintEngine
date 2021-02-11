﻿using rx_CamLib;
using RX_Common;
using RX_DigiPrint.Models;
using RX_DigiPrint.Services;
using System;
using System.Collections.Generic;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;

namespace RX_DigiPrint.Views.SetupAssistView
{

	/// <summary>
	/// Interaction logic for SA_Actions.xaml
	/// </summary>
	public partial class SA_ActionsView :UserControl
	{
		//--- SA_ActionsView -------------------------------------------------
		public SA_ActionsView()
		{
			InitializeComponent();
		}

		//--- Start -------------------------------------------------
		public void Start()
		{
			if (!RxGlobals.SA_StateMachine.Running && RxGlobals.PrintSystem.ReadyToPrint())
			{
				Actions.ItemsSource = RxGlobals.SA_StateMachine.Start();
			}
		}

		//--- ActionDone --------------------------------------------------------------
		public void ActionDone()
		{
			RxGlobals.SA_StateMachine.Continue();
		}

		public void Test()
		{
			List<SA_Action> actions=RxGlobals.SA_StateMachine.Test();
			if (Actions.ItemsSource==null)
			{
				Actions.ItemsSource=actions;
			}
		}

		//--- Confirm_Clicked --------------------------------------
		private void Confirm_Clicked(object sender, RoutedEventArgs e)
		{
			RxGlobals.SA_StateMachine.ConfirmPosAndFocus();
		}
	}

	//--- StateBusy_Converter ----------------------------------------------------
	public class StateBusy_Converter : IValueConverter
	{
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
		{
			return 	((ECamFunctionState)value==ECamFunctionState.running);
		}
		public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            return null;
        }
	}

	//--- StateImage_Converter ----------------------------------------------------
	public class StateImage_Converter : IValueConverter
	{
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
		{
			switch((ECamFunctionState)value)
			{
				case ECamFunctionState.done:    return "Check";
				case ECamFunctionState.aborted: return "Cancel";
				case ECamFunctionState.error:	return "AlertCircleOutline";
				default: return null;
			}
		}
		public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            return null;
        }
	}

	//--- Visible_Converter ----------------------------------------------------
	public class Visible_Converter : IValueConverter
	{
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
		{
			if (value!=null) return Visibility.Visible;
			return Visibility.Collapsed;
		}
		public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
			return null;
        }
	}

	public class AngleStr_Converter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
			List<float> list = parameter as List<float>;
            if (list!=null)
            {
				return list.Count.ToString();
            }
            return null;
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }

}
