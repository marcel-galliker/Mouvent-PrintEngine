using Infragistics.Controls.Grids;
using RX_DigiPrint.Models;
using System.IO;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Markup;
using System.Xml;

namespace RX_DigiPrint.Views.ConditionerTextCenterView
{
	/// <summary>
	/// Interaction logic for CTC_TestView.xaml
	/// </summary>
	public partial class CTC_TestView : UserControl
	{
		private bool _init=false;

		//--- constructor ------------------------------------------
		public CTC_TestView()
		{
			InitializeComponent();

		}

		//--- UserControl_IsVisibleChanged ------------------------------
		private void UserControl_IsVisibleChanged(object sender, DependencyPropertyChangedEventArgs e)
		{
			if ((bool)e.NewValue && !_init)
			{
				_init = true;
				RxGlobals.CTC_TestView = this;
				TestsGrid.ItemsSource = RxGlobals.CTC_Operation.Tests;
				for (int head=0; head<CTC_Test.HEADS; head++)
				{
					TestsGrid.Columns.Insert(TestsGrid.Columns.Count - 1, CreateTemplateColumn(head));
				}
			}
		}

		//--- CreateTemplateColumn -------------------------------------
		private TemplateColumn CreateTemplateColumn(int head)
		{
			string key = @"State[" + head.ToString() + @"]";
			DataTemplate template= CreateDataTemplate(key);
			TemplateColumn col = new TemplateColumn() { HeaderText = (head+1).ToString(), Key = key, ItemTemplate = template };
			return col;
		}

		//--- CreateDataTemplate --------------------
		private DataTemplate CreateDataTemplate(string key)
		{
			StringReader stringReader = new StringReader(
			@"<DataTemplate 
				xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
				xmlns:iconPacks=""http://metro.mahapps.com/winfx/xaml/iconpacks""> 
				<iconPacks:PackIconMaterial" + @" 
					Kind      =""{Binding " + key + @", Converter={StaticResource CTC_StateImage_Converter}}""
					Foreground=""{Binding " + key + @", Converter={StaticResource CTC_StateColor_Converter}}""
				/> 
			</DataTemplate>");
			XmlReader xmlReader = XmlReader.Create(stringReader);
			return XamlReader.Load(xmlReader) as DataTemplate;
		}

	}
}
