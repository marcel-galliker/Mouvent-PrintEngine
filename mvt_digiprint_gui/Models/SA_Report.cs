using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Threading;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Markup;
using rx_CamLib;
using MahApps.Metro.IconPacks;
using RX_DigiPrint.Converters;
using System.Globalization;
using System.ComponentModel;
using System.Windows.Shapes;
using System.Windows.Media;

namespace RX_DigiPrint.Models
{
	public class SA_Report
	{
		//--- PrintReport ----------------------------------
		public void PrintReport(List<SA_Action>  actions, DateTime timePrinted)
		{
			if (actions==null) return;

			PrintDialog pd = new PrintDialog();	
		//	if(pd.ShowDialog()==true) // use default printer
			{
				try
				{
					FixedDocument doc = new FixedDocument();
					doc.DocumentPaginator.PageSize = new Size(pd.PrintableAreaHeight, pd.PrintableAreaWidth);
					doc.Pages.Add(_composePage(doc.DocumentPaginator.PageSize, actions, timePrinted));
					pd.PrintTicket.PageOrientation = System.Printing.PageOrientation.Landscape;
					pd.PrintDocument(doc.DocumentPaginator, "Adjustment");
				}
				catch(Exception)
				{ }
			}
		}

		//--- _composeAction -----------------------------------
		private void _composeAction(Grid grid, SA_Action action)
		{
			if (action.Function==ECamFunction.CamMeasureAngle || action.Function==ECamFunction.CamMeasureStitch)
			{
				int row = grid.RowDefinitions.Count;
				int col=0;
				grid.RowDefinitions.Add(new RowDefinition(){ Height=GridLength.Auto});
				
				//--- background --------
				if ((row&1)==0)
				{
					Color c = Color.FromRgb(240,240,240);
					System.Windows.Shapes.Rectangle rect =new Rectangle()
					{
						Fill = new SolidColorBrush(c)
					};
					Grid.SetRow(rect, row);
					Grid.SetColumn(rect, 0);
					Grid.SetColumnSpan(rect, 10);
					grid.Children.Add(rect);
				}

				//--- 0: state -----------
				string kind = (string)new SA_StateImage_Converter().Convert(action.State, null, null, null);
				if (kind!=null)
				{
					PackIconMaterial state = new PackIconMaterial();
					state.Kind = (PackIconMaterialKind)TypeDescriptor.GetConverter(typeof(PackIconMaterialKind)).ConvertFromString(kind);
					Grid.SetRow(state, row);
					Grid.SetColumn(state, col);
					grid.Children.Add(state);
				}
				col++;

				//--- 1: Color ---------------------
				Ellipse color = new Ellipse()
					{
						HorizontalAlignment=HorizontalAlignment.Center, 
						VerticalAlignment=VerticalAlignment.Center, 
						Width=15, 
						Height=15,  
						Stroke=Brushes.Transparent,
						Margin=new Thickness(5,5,5,5)
					};
				color.Fill = action.ColorBrush;
				Grid.SetRow(color, row);
				Grid.SetColumn(color, col++);
				grid.Children.Add(color);
				col++;

				//--- 2: Function --------------
				PackIconMaterial function = new PackIconMaterial(){ VerticalAlignment=VerticalAlignment.Center};
				function.Kind = action.IconKind;
				Grid.SetRow(function, row);
				Grid.SetColumn(function, col);
				grid.Children.Add(function);
				col++;

				//--- 3: Name --------------------
				TextBlock name=new TextBlock();
				name.Text = action.Name;
				Grid.SetRow(name, row);
				Grid.SetColumn(name, col++);
				grid.Children.Add(name);

				if (RxGlobals.PrintSystem.HostName.Equals("TEST-0001"))
				{
					action.Correction = row;
					if ((row&1)!=0) action.Correction = -action.Correction;
				}
				if (action.Correction==null)
				{
				}
				else
				{
					//--- 4: Result turn CW --------------------
					double cw, ccw;
					if (action.Correction<0)
					{
						cw = Math.Abs((double)action.Correction-2);
						ccw = 2;
					}
					else
					{
						cw=0;
						ccw=Math.Abs((double)action.Correction);
					}
				
					if (cw==0 && ccw==0)
					{
						PackIconMaterial ok = new PackIconMaterial(){Kind = PackIconMaterialKind.Check };
						Grid.SetRow(ok, row);
						Grid.SetColumn(ok, col);
						grid.Children.Add(ok);
						col+=4;
					}
					else
					{
						if (cw!=0)
						{
							PackIconMaterial turncw = new PackIconMaterial(){ Kind=PackIconMaterialKind.AxisZRotateClockwise, Margin=new Thickness(5,2,2,2)};
							Grid.SetRow(turncw, row);
							Grid.SetColumn(turncw, col++);
							grid.Children.Add(turncw);

							TextBlock txtcw=new TextBlock();
							txtcw.Text = String.Format("{0:0.0}", cw);
							Grid.SetRow(txtcw, row);
							Grid.SetColumn(txtcw, col++);
							grid.Children.Add(txtcw);
						}
						else col+=2;

						//--- 5: Result turn CCW --------------------
						if (ccw!=0)
						{
							PackIconMaterial turnccw = new PackIconMaterial(){ Kind=PackIconMaterialKind.AxisZRotateCounterclockwise, Margin=new Thickness(5,2,2,2)};
							Grid.SetRow(turnccw, row);
							Grid.SetColumn(turnccw, col++);
							grid.Children.Add(turnccw);

							TextBlock txtccw=new TextBlock();
							txtccw.Text = String.Format("{0:0.0}", ccw);
							Grid.SetRow(txtccw, row);
							Grid.SetColumn(txtccw, col++);
							grid.Children.Add(txtccw);
						}
						else col+=2;
					}
				}
			}
		}

		//--- _composeColor ------------------------------------------
		private Grid _composeColor(List<SA_Action>  actions, int color)
		{
			//--- actions grid ----------------------------------------
			Grid actionsGrid = new Grid() {Margin = new Thickness(20,50,20,0)};
			
			actionsGrid.ColumnDefinitions.Add(new ColumnDefinition(){ Width=GridLength.Auto});
			actionsGrid.ColumnDefinitions.Add(new ColumnDefinition(){ Width=GridLength.Auto});
			actionsGrid.ColumnDefinitions.Add(new ColumnDefinition(){ Width=GridLength.Auto});
			actionsGrid.ColumnDefinitions.Add(new ColumnDefinition(){ Width=GridLength.Auto});
			actionsGrid.ColumnDefinitions.Add(new ColumnDefinition(){ Width=GridLength.Auto});
			actionsGrid.ColumnDefinitions.Add(new ColumnDefinition(){ Width=GridLength.Auto});
			actionsGrid.ColumnDefinitions.Add(new ColumnDefinition(){ Width=GridLength.Auto});
			actionsGrid.ColumnDefinitions.Add(new ColumnDefinition(){ Width=GridLength.Auto});
			actionsGrid.ColumnDefinitions.Add(new ColumnDefinition(){ Width=GridLength.Auto});
			actionsGrid.ColumnDefinitions.Add(new ColumnDefinition(){ Width=GridLength.Auto});

			List<SA_Action> list = new List<SA_Action>();

			foreach(SA_Action action in actions)
			{
				if (action.PrintbarNo==color)
				{
					list.Add(action);
				}
			}
			list.Sort((a1, a2) => string.Compare(a1.Name, a2.Name));
			foreach(SA_Action action in list)
			{
				_composeAction(actionsGrid, action);
			}
			return actionsGrid;
		}

		//--- _composePage ---------------------------------------
		private PageContent _composePage(Size pageSize, List<SA_Action>  actions, DateTime timePrinted)
		{
			FixedPage page = new FixedPage() { Margin=new Thickness()};
			page.Width  = pageSize.Width;
			page.Height = pageSize.Height;

			Grid pageGrid = new Grid(){ Margin=new Thickness(50,30,0,0)};

			pageGrid.ColumnDefinitions.Add(new ColumnDefinition(){Width=GridLength.Auto});
			pageGrid.ColumnDefinitions.Add(new ColumnDefinition(){Width=GridLength.Auto});

			pageGrid.RowDefinitions.Add(new RowDefinition(){ Height=GridLength.Auto});
			pageGrid.RowDefinitions.Add(new RowDefinition(){ Height=GridLength.Auto});
			pageGrid.RowDefinitions.Add(new RowDefinition(){ Height=GridLength.Auto});

			TextBlock title=new TextBlock() { };
			title.Text="Alignment Measurment of ";
			pageGrid.Children.Add(title);

			TextBlock machine=new TextBlock() { };
			machine.SetValue(TextBlock.FontWeightProperty, FontWeights.Bold);
			machine.Text=RxGlobals.PrintSystem.HostName;
			Grid.SetColumn(machine, 1);
			pageGrid.Children.Add(machine);

			TextBlock time=new TextBlock() { };
			time.Text="Printed ";
			Grid.SetRow(time, 1);
			pageGrid.Children.Add(time);

			TextBlock now=new TextBlock();
			now.SetValue(TextBlock.FontWeightProperty, FontWeights.Bold);
			now.Text=timePrinted.ToString();
			Grid.SetRow(now, 1);
			Grid.SetColumn(now,1);
			pageGrid.Children.Add(now);

			Grid resultsGrid = new Grid();
			Grid.SetRow(resultsGrid, 2);
			Grid.SetColumn(resultsGrid,0);
			Grid.SetColumnSpan(resultsGrid,3);

			for (int color=0; color<RxGlobals.PrintSystem.ColorCnt; color++)
			{
				resultsGrid.ColumnDefinitions.Add(new ColumnDefinition(){ Width=new GridLength(250)});
				Grid actionsGrid = _composeColor(actions, color);
				Grid.SetColumn(actionsGrid, color);
				resultsGrid.Children.Add(actionsGrid);
			}
			pageGrid.Children.Add(resultsGrid);
			page.Children.Add(pageGrid);

			//--- END ----------------------------
			PageContent pc = new PageContent();
			((IAddChild)pc).AddChild(page);
			return pc;
		}
	}
}
