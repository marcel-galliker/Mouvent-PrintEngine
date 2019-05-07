using System;
using System.Deployment.Application;
using System.Windows;

namespace RX_LabelComposer.Views
{
    /// <summary>
    /// Interaction logic for About.xaml
    /// </summary>
    public partial class AboutDlg : Window
    {
        public AboutDlg(System.Reflection.AssemblyName assembly)
        {
            InitializeComponent();
            DataContext=this;
            Version=assembly.Version.ToString();
 
            if (ApplicationDeployment.IsNetworkDeployed)
            {
                Version v=ApplicationDeployment.CurrentDeployment.CurrentVersion;
                PublishVersion = string.Format("{0}.{1}.{2}.{3}", v.Major, v.Minor, v.Build, v.Revision);
            }
            else PublishVersion="offine";
        }

        //--- Property Version ---------------------------------------
        private string _Version;
        public string Version
        {
            get { return _Version; }
            set { _Version= value; }
        }

        //--- Property PublishVersion ---------------------------------------
        private string _PublishVersion;
        public string PublishVersion
        {
            get { return _PublishVersion; }
            set { _PublishVersion= value; }
        }
        

    }
}
