using System;
using System.Linq;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;

namespace RX_DigiPrint.Views.SetupAssistView
{
	/// <summary>
	/// Interaction logic for SA_ValueGrid.xaml
	/// </summary>
	public partial class SA_ValueGrid : UserControl
	{
		public SA_ValueGrid()
		{
			InitializeComponent();

			//--- create list -----------------------------------------------------
			for(int col=0; col<8; col++)
			{
				_Grid.ColumnDefinitions.Add(new ColumnDefinition(){ Width=new GridLength(1, GridUnitType.Star)});
				TextBlock txt = new TextBlock()
				{ 
					Foreground=Brushes.Gray,
					FontSize = (double) App.Current.FindResource("FontSizeSmall"),
					HorizontalAlignment = HorizontalAlignment.Right,
				};
				Grid.SetColumn(txt, col);
				_Grid.Children.Add(txt);
			}
		}

		//--- UserControl_DataContextChanged ------------------------------------------------------------
		private void UserControl_DataContextChanged(object sender, DependencyPropertyChangedEventArgs e)
		{
			String str = e.NewValue as string;
			if (str!=null)
			{
				String[] list = str.Split(' ');
				int i;
				int cnt=Math.Min(list.Count(), _Grid.Children.Count);
				for (i = 0; i < cnt; i++)
				{
					(_Grid.Children[i] as TextBlock).Text = list[i];
				}
				while (i<_Grid.Children.Count) 
				{
					(_Grid.Children[i++] as TextBlock).Text="";
				}
			}
		}
	}
}
