using MahApps.Metro.IconPacks;
using rx_CamLib;
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
		public void StartAlign()
		{
			if (!RxGlobals.SA_StateMachine.Running && RxGlobals.PrintSystem.ReadyToPrint())
			{
				ResultHdr.HeaderText = string.Format("Result (tol A=±{0:0.0} Rev, S=±{1:0.0} Rev)", RxGlobals.Settings.SetupAssistCam.ToleranceAngle, RxGlobals.Settings.SetupAssistCam.ToleranceStitch);
				Actions.ItemsSource  = RxGlobals.SA_StateMachine.StartAlign();
				_ActionsChanged();
			}
		}

		//--- StartDensity -------------------------------------------------
		public void StartDensity()
		{
			if (!RxGlobals.SA_StateMachine.Running && RxGlobals.PrintSystem.ReadyToPrint())
			{
				Actions.ItemsSource = RxGlobals.SA_StateMachine.StartDensity();
				_ActionsChanged();
			}
		}

		//--- StartDensity -------------------------------------------------
		public void StartRegister()
		{
			if (!RxGlobals.SA_StateMachine.Running && RxGlobals.PrintSystem.ReadyToPrint())
			{
				Actions.ItemsSource = RxGlobals.SA_StateMachine.StartRegister();
				_ActionsChanged();
			}
		}

		//--- _ActionsChanged -----------------------------------------------------------
		private void _ActionsChanged()
		{
			List< SA_Action> actions = Actions.ItemsSource as List<SA_Action>;
			if (actions==null) return;

			// hide some actions
			for (int i=0; i<actions.Count; i++)
			{
				if (actions[i].Function == ECamFunction.CamMoveScan
				|| actions[i].Function == ECamFunction.CamMoveWeb)
				{
					Actions.Rows[i].Height = new Infragistics.Controls.Grids.RowHeight(0);
				}
			}
		}

		//--- ActionDone --------------------------------------------------------------
		public void ActionDone()
		{
			RxGlobals.SA_StateMachine.Continue();
		}

		//--- PrintReport --------------------------------
		public void PrintReport()
		{
			//	new SA_Report().PrintReport(RxGlobals.SA_StateMachine.GetActions(), RxGlobals.SA_StateMachine.TimePrinted(), true);
			new SA_Report().PrintReport(Actions.ItemsSource as List<SA_Action>, RxGlobals.SA_StateMachine.TimePrinted(), true);
		}

		//--- FileOpen --------------------------------
		public void FileOpen()
		{
			List<SA_Action> actions = SA_Action.FileOpen();
			if (actions != null)
			{
				Actions.ItemsSource = actions;
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
			RxGlobals.SA_StateMachine.ConfirmFocus();
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

	//--- FunctionImageVisible_Converter ----------------------------------------------------
	public class FunctionImageVisible_Converter : IValueConverter
	{
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
		{
			if ((PackIconMaterialKind)value == PackIconMaterialKind.None) return Visibility.Collapsed;
			return Visibility.Visible;
		}

		public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
			return null;
        }
	}
}
