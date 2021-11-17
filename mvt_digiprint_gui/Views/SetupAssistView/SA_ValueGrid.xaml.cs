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
			String str = e.NewValue as string;
			if (str!=null)
			{
				String[] list = str.Split(' ');
			// !first item is always ""
				int i;
				int cnt=Math.Min(list.Count()-1, _Grid.Children.Count);
				for (i = 0; i < cnt; i++)
				{
					(_Grid.Children[i] as TextBlock).Text = list[i+1];
				}
				while (i<_Grid.Children.Count) 
				{
					(_Grid.Children[i++] as TextBlock).Text="";
				}
			}

			List<SA_Value> values = e.NewValue as List<SA_Value>;
			if (values!=null)
			{
				int i;
				int cnt = Math.Min(values.Count() - 1, _Grid.Children.Count);
				for (i=0; i< _Grid.Children.Count; i++)
				{
					TextBlock txt = _Grid.Children[i] as TextBlock;
					if (i< values.Count)
					{
						if (double.IsNaN(values[i].Value)) txt.Text = "---";
						else txt.Text = values[i].Value.ToString();
						txt.FontWeight = values[i].Used ? FontWeights.Bold : FontWeights.Normal;
					}
					else txt.Text = "";
				}
			}
		}
	}
}
