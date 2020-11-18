using rx_CamLib;
using RX_Common;
using RX_DigiPrint.Converters;
using RX_DigiPrint.Models;
using RX_DigiPrint.Services;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;

namespace RX_DigiPrint.Views.SetupAssistView
{
	//--- StateBusy_Converter ----------------------------------------------------
	public class StateBusy_Converter : IValueConverter
	{
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
		{
			return 	((ECamFunctionState)value==ECamFunctionState.running);
		}
		public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
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
				default: return null;
			}
		}
		public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
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

	/// <summary>
	/// Interaction logic for SA_Actions.xaml
	/// </summary>
	public partial class SA_Actions :UserControl
	{
		private const bool		 _SimuMachine = false;
		private const bool		 _SimuCamera  = true;

		private RxCamFunctions	 _CamFunctions;
		private List<SA_Action>  _Actions;
		private int				 _ActionIdx;
		private SA_Action		 _Action;

		public SA_Actions()
		{
			InitializeComponent();

			RxGlobals.SetupAssist.PropertyChanged +=SetupAssist_PropertyChanged;
		}

		private void SetupAssist_PropertyChanged(object sender,System.ComponentModel.PropertyChangedEventArgs e)
		{
			if (e.PropertyName.Equals("ActionRunning"))
			{
				if (!RxGlobals.SetupAssist.ActionRunning) Stop_Action();
			}
		}

		//--- _InitActions -----------------------------
		public void InitActions(RxCam cam)
		{
			int color, n;
			_CamFunctions = new RxCamFunctions(cam);
			_CamFunctions.Simulation = _SimuCamera;
			_Actions = new List<SA_Action>();
			_Actions.Add(new SA_Action(){Name="Print Image" });
			_Actions.Add(new SA_Action()
			{
				Name="Find Mark",
				ScanPos	= 10.0,
			});

			//--- measurmentfunctions -----------------------------
			const double pos0 = 20.0;
			const double headdist = 43.328;

			for (color=0; color<RxGlobals.PrintSystem.ColorCnt; color++)
			{
				InkType ink = RxGlobals.InkSupply.List[color*RxGlobals.PrintSystem.InkCylindersPerColor].InkType;
				if (ink!=null)
				{
					string colorName = new ColorCode_Str().Convert(ink.ColorCode, null, color*RxGlobals.PrintSystem.InkCylindersPerColor, null).ToString();
					for (n=0; n<RxGlobals.PrintSystem.HeadsPerColor-1; n++)
					{
						SA_Action action=new SA_Action()
						{
							PrintbarNo	= color,
							HeadNo		= n,
							Name		= String.Format("Measure {0}-{1}..{2}",  colorName, n+1, n+2),
							WebMoveDist = (n==0)? 20.0 : 0,
							ScanPos	    = pos0+n*headdist,
						};
						
						_Actions.Add(action);
					}
				}
			}

			Actions.ItemsSource = _Actions;
			_ActionIdx = 0;
			StartAction();
		}

		//--- StartAction -----------------------------------------
		public void StartAction()
		{
			if (_ActionIdx>=_Actions.Count()) return;
			_Action=_Actions[_ActionIdx];
			_Action.State = ECamFunctionState.running;
			RxGlobals.SetupAssist.ActionRunning = true;
			if (_ActionIdx==0)		_StartTestPrint();
			else if (_ActionIdx==1)	_FindMark();
			else					_Measure();
		}

		//--- ActionDone --------------------------------------------
		public void ActionDone()
		{
			_Actions[_ActionIdx].State = ECamFunctionState.done;
			_ActionIdx++;
			if (_ActionIdx==_Actions.Count()) RxGlobals.RxInterface.SendCommand(TcpIp.CMD_SA_REFERENCE);
			StartAction();
		}

		//--- Stop_Action -----------------------------------------
		public void Stop_Action()
		{
			_Action.State = ECamFunctionState.aborted;
			_ActionIdx=_Actions.Count();
			RxGlobals.RxInterface.SendCommand(TcpIp.CMD_SA_REFERENCE);
		}		

		//--- _StartTestPrint --------------------------------------
		private void _StartTestPrint()
        {
			PrintQueueItem item = new PrintQueueItem();

            if (InkSupply.AnyFlushed()) return;

            if (RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_cleaf && !(RxGlobals.StepperStatus[0].DripPans_InfeedDOWN && RxGlobals.StepperStatus[0].DripPans_OutfeedDOWN))
            {
                MvtMessageBox.YesNo("Print System", "Drip Pans below the clusters. Move it out before printing", MessageBoxImage.Question, true);
                return;
            }

            if (!RxGlobals.PrinterStatus.AllInkSupliesOn)
            {
                if (MvtMessageBox.YesNo("Print System", "Some ink supplies are OFF. Switch them ON.", MessageBoxImage.Question, true))
                {
                    TcpIp.SFluidCtrlCmd msg = new TcpIp.SFluidCtrlCmd();
                    msg.no = -1;
                    msg.ctrlMode = EFluidCtrlMode.ctrl_print;

                    RxGlobals.RxInterface.SendMsg(TcpIp.CMD_FLUID_CTRL_MODE, ref msg);
                }
            }

            if (RxGlobals.UvLamp.Visible == Visibility.Visible && !RxGlobals.UvLamp.Ready)
            {
                if (!MvtMessageBox.YesNo("UV Lamp", "The UV Lamp is NOT READY.\n\nStart Printing?", MessageBoxImage.Question, false))
                    return;
            }

			item.TestImage	= ETestImage.fullAlignment;
			item.Dots		= "L";
			item.Speed		= 50;
			item.ScanMode	= EScanMode.scan_std;
			item.PageMargin = 0;
            item.ScanLength = item.Copies = 1;
            item.SendMsg(TcpIp.CMD_TEST_START);
			RxGlobals.SetupAssist.OnWebMoveDone=ActionDone;
			if (_SimuMachine) ActionDone();
        }

		//--- _FindMark ------------------------------------
		private void _FindMark()
		{
			if (!_SimuMachine) RxGlobals.SetupAssist.ScanMoveTo(_Action.ScanPos, _ScanMoveDone);
			else _ScanMoveDone();
		}

		//--- _Camera_CamMarkFound ---------------------------------------
		private void _Camera_CamMarkFound()
		{
			RxGlobals.RxInterface.SendCommand(TcpIp.CMD_SA_STOP);
			ActionDone();
		}

		//--- _Camera_CamMarkNotFound -------------------------------------------
		private void _Camera_CamMarkNotFound()
		{
			if (_ActionIdx==0)
			{
				_ActionIdx = _Actions.Count();
				MvtMessageBox.Information("Find Mark", "Start mark not found!");
			}
		}

		//--- _Measure ----------------------------------------------
		private void _Measure()
		{
			if (_SimuMachine)
			{
				_CamFunctions.MeasurePosition(_OnPositionMeasured);
			}
			else
			{
				Console.WriteLine("Action[{0}]: ScanPos={1}, WebMoveDist={2}", _ActionIdx, _Action.ScanPos, _Action.WebMoveDist);
				RxGlobals.SetupAssist.ScanMoveTo(_Action.ScanPos,  _ScanMoveDone);
				RxGlobals.SetupAssist.WebMove(_Action.WebMoveDist, _WebMoveDone);
			}
		}

		//--- _ScanMoveDone -------------------------------
		private void _ScanMoveDone()
		{
			Console.WriteLine("Action[{0}]: _ScanMoveDone", _ActionIdx);
			_Action.ScanMoveDone=true;
			if (_ActionIdx==1) 
			{
				_CamFunctions.FindMark(_Camera_CamMarkFound);
				RxGlobals.SetupAssist.WebMove(100*1000, _Camera_CamMarkNotFound);
			}
			else _StartCamFunction();
		}

		//--- _WebMoveDone ---------------------------------
		private void _WebMoveDone()
		{
			Console.WriteLine("Action[{0}]: _WebMoveDone", _ActionIdx);
			_Action.WebMoveDone=true;
			_StartCamFunction();
		}

		//--- _StartCamFunction ----------------------------------------------
		private void _StartCamFunction()
		{
			Console.WriteLine("Action[{0}]: _StartCamFunction ScanMoveDone={1} WebMoveDist={2} WebMoveDone={3}", _ActionIdx, _Action.ScanMoveDone, _Action.WebMoveDist, _Action.WebMoveDone);

			if (_Action.ScanMoveDone && (_Action.WebMoveDist==0 || _Action.WebMoveDone)) 
			{
				if (_ActionIdx>1)  _CamFunctions.MeasurePosition(_OnPositionMeasured);
			}
		}
		//--- _OnPositionMeasured -------------------------------------
		private void _OnPositionMeasured(SPosition pos)
		{
			_Action.Measured = pos;
			ActionDone();
		}
	}
}
