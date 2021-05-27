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

		//--- Start -------------------------------------------------
		public void StartDensity()
		{
			if (!RxGlobals.SA_StateMachine.Running && RxGlobals.PrintSystem.ReadyToPrint())
			{
				Actions.ItemsSource = RxGlobals.SA_StateMachine.StartDensity();
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

		//--- MoveLeft_Clicked --------------------------------------
		private void MoveLeft_Clicked(object sender, RoutedEventArgs e)
		{
			RxGlobals.SetupAssist.ScanMoveTo(RxGlobals.SetupAssist.ScanPos+0.2);
		}

		//--- MoveRight_Clicked --------------------------------------
		private void MoveRight_Clicked(object sender, RoutedEventArgs e)
		{
			RxGlobals.SetupAssist.ScanMoveTo(RxGlobals.SetupAssist.ScanPos-0.2);
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
			return 	((ECamFunctionState)value==ECamFunctionState.printing 
				  || (ECamFunctionState)value==ECamFunctionState.runningCam);
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
				case ECamFunctionState.runningCam:	return "VideoOutline";
				case ECamFunctionState.waitRob:		return "VideoCheckOutline";
				case ECamFunctionState.runningRob:	return "RobotIndustrial";
				case ECamFunctionState.done:		return "Check";
				case ECamFunctionState.aborted:		return "Cancel";
				case ECamFunctionState.error:		return "AlertCircleOutline";
				default: return null;
			}
		}
		public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            return null;
        }
	}

	//--- FunctionImageVisible_Converter ----------------------------------------------------
	public class FunctionImageVisible_Converter : IValueConverter
	{
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
		{
			switch((ECamFunction)value)
			{
				case ECamFunction.CamMeasureAngle:	return Visibility.Visible;
				case ECamFunction.CamMeasureStitch:	return Visibility.Visible;
				case ECamFunction.CamMeasureDist:	return Visibility.Visible;
				default: return Visibility.Collapsed;
			}
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
