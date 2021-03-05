using RX_DigiPrint.Models;
using RX_DigiPrint.Services;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace RX_DigiPrint.Views.UserControls
{
    /// <summary>
    /// Interaction logic for PlcTiitle.xaml
    /// </summary>
    public partial class PlcTitle : UserControl
    {
        public PlcTitle()
        {
            InitializeComponent();
            DataContext=this;
        }

        //--- Property Header ---------------------------------------
        private string _Header;
        public string Header
        {
            get { return _Header; }
            set { _Header = value; }
        }

        public static readonly DependencyProperty HeaderProperty
        = DependencyProperty.Register(
            "Header",
            typeof(String),
            typeof(PlcTitle),
            new FrameworkPropertyMetadata("",
                OnHeaderChanged
                )
            );

        private static void OnHeaderChanged(DependencyObject source,
        DependencyPropertyChangedEventArgs e)
        {
            PlcTitle p = source as PlcTitle;
            p.Header = (String)e.NewValue;
        }


        //--- Property User ---------------------------------------
        private EUserType _User;
        public EUserType User
        {
            get { return _User; }
            set
            { 
                RxGlobals.User.PropertyChanged += User_PropertyChanged;
                _User = value;
                _setUserType();
            }
        }

		//--- Property Color ---------------------------------------
		public Color Color
		{
			set { Line.Fill=new SolidColorBrush(value); }
		}

		//--- _setUserType ---------------
		private void _setUserType()
        {
            Visibility = (RxGlobals.User.UserType>=User) ? Visibility.Visible : Visibility.Collapsed;
        }

        //--- User_PropertyChanged ----------------------------
        void User_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            _setUserType();            
        }

    }
}
