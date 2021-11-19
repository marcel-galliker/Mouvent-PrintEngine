using RX_DigiPrint.Models;
using System;
using System.Collections.Generic;
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

		//--- FontSize -----------------------------------
		new public int FontSize 
		{
			set {
					foreach(var ctrl in _Grid.Children)
					{
						if (ctrl is TextBlock) (ctrl as TextBlock).FontSize = value;
					}
				} 
		}

		//--- UserControl_DataContextChanged ------------------------------------------------------------
		private void UserControl_DataContextChanged(object sender, DependencyPropertyChangedEventArgs e)
		{
			List<SA_Value> values = e.NewValue as List<SA_Value>;
			for (int i=0; i< _Grid.Children.Count; i++)
			{
				TextBlock txt = _Grid.Children[i] as TextBlock;
				if (values != null && i < values.Count)
				{
					if (double.IsNaN(values[i].Value)) txt.Text = "---";
					else txt.Text = string.Format("{0:0.0}",  values[i].Value);
					txt.FontWeight = values[i].Used ? FontWeights.Bold : FontWeights.Normal;
				}
				else txt.Text = "";
			}
		}
	}
}
