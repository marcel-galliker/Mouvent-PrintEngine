using RX_DigiPrint.Helpers;
using RX_DigiPrint.Models;
using RX_DigiPrint.Models.Enums;
using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Shapes;

namespace RX_DigiPrint.Views.SetupAssistView
{
	/// <summary>
	/// Interaction logic for AlignmentSettings.xaml
	/// </summary>
	public partial class SA_AlignSettingsWnd : CustomWindow
    {
		//--- constructor -----------------------------------------------------
		public SA_AlignSettingsWnd()
        {
            InitializeComponent();
            DataContext = this;
            RxGlobals.SA_AlignSettings.Init();
            Init();
        }

        private CheckBox[] CB_Angle;
        private CheckBox[] CB_Stitch;

        //--- Init --------------------------------
        private void Init()
		{
            CB_ToleranceAngle.ItemsSource  = new SA_Tolerance_List();
            CB_ToleranceStitch.ItemsSource = new SA_Tolerance_List();
            CB_Passes.ItemsSource          = new EN_Numbers(1,5,1);
            CB_Passes.SelectedValue = RxGlobals.SA_AlignSettings.Passes;

            CB_ToleranceAngle.SelectedValue  = RxGlobals.SA_AlignSettings.ToleranceAngle;
            CB_ToleranceStitch.SelectedValue = RxGlobals.SA_AlignSettings.ToleranceStitch;
            
            int col=1;
            CB_Angle  = new CheckBox[RxGlobals.PrintSystem.ColorCnt];
            CB_Stitch = new CheckBox[RxGlobals.PrintSystem.ColorCnt];

            for (int i=0; i<RxGlobals.PrintSystem.ColorCnt; i++)
			{
                int color = RxGlobals.PrintSystem.Color_Order[i];
                InkType ink = RxGlobals.InkSupply.List[color * RxGlobals.PrintSystem.InkCylindersPerColor].InkType;
                if (ink != null)
				{
                    BoolGrid.ColumnDefinitions.Add(new ColumnDefinition() { Width=new GridLength(50)});
                    UIElement obj = ink.guiCtrl;
                    Grid.SetColumn(obj, col);
                    Grid.SetRow(obj, 0);
                    BoolGrid.Children.Add(obj);

                    CB_Angle[i] = new CheckBox() { VerticalAlignment = VerticalAlignment.Center, HorizontalAlignment = HorizontalAlignment.Center };
                    Grid.SetColumn(CB_Angle[i], col);
                    Grid.SetRow(CB_Angle[i], 1);
                    BoolGrid.Children.Add(CB_Angle[i]);
                    CB_Angle[i].IsChecked = RxGlobals.SA_AlignSettings.MeasureAngle[i];

                    CB_Stitch[i] = new CheckBox(){ VerticalAlignment=VerticalAlignment.Center, HorizontalAlignment=HorizontalAlignment.Center};
                    Grid.SetColumn(CB_Stitch[i], col);
                    Grid.SetRow(CB_Stitch[i], 2);
                    BoolGrid.Children.Add(CB_Stitch[i]);
                    CB_Stitch[i].IsChecked = RxGlobals.SA_AlignSettings.MeasureStitch[i];

                    col++;
                }
            }
        }

        //--- Save_Clicked ------------------------------------
        private void Save_Clicked(object sender, RoutedEventArgs e)
        {
            RxGlobals.SA_AlignSettings.Passes          = (int)CB_Passes.SelectedValue;
            RxGlobals.SA_AlignSettings.ToleranceAngle  = Convert.ToDouble(CB_ToleranceAngle.SelectedValue);
            RxGlobals.SA_AlignSettings.ToleranceStitch = Convert.ToDouble(CB_ToleranceStitch.SelectedValue);
            for (int i = 0; i < RxGlobals.PrintSystem.ColorCnt; i++)
			{
                if (CB_Angle[i]==null || CB_Angle[i].IsChecked==null) 
                     RxGlobals.SA_AlignSettings.MeasureAngle[i]=false;
                else 
                     RxGlobals.SA_AlignSettings.MeasureAngle[i]  = (bool)CB_Angle[i].IsChecked;
                if (CB_Angle[i] == null || CB_Angle[i].IsChecked == null)
                    RxGlobals.SA_AlignSettings.MeasureStitch[i] = false;
                else
                    RxGlobals.SA_AlignSettings.MeasureStitch[i] = (bool)CB_Stitch[i].IsChecked;
            }

            RxGlobals.SA_AlignSettings.Save();
            DialogResult = true;
        }

        //--- Cancel_Clicked --------------------------------
        private void Cancel_Clicked(object sender, RoutedEventArgs e)
        {
            DialogResult = false;
        }

        //--- Window_Loaded ----------------------------
        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            RxGlobals.Screen.PlaceWindow(this);
        }
    }
}