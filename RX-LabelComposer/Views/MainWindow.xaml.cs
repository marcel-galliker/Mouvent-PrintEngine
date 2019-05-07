using System.Windows;

namespace RX_LabelComposer.Views
{
    public partial class MainWindow : Window
    {
        //--- MainWindow --------------------------------------------------------------------------
        public MainWindow()
        {
            InitializeComponent();
            this.DataContext = new ViewModels.MainViewModel();
        }
    }

#if false
    public partial class MainWindow : Window
    {
        private string HomePage_URL = "http://www.radex-net.com";
        private string Samples_URL  = "http://www.radex-net.com/Downloads/RX-LabelComposer/Samples/Samples.zip";

        static FileDefinition       _FileDef    = FileDefinition.The_FileDefinition;
        static LayoutDefinition     _LayoutDef  = LayoutDefinition.The_LayoutDef;
        static CounterDefinition    _CounterDef = CounterDefinition.The_CounterDefinition;
        
        //--- MainWindow ---------------------------------------------------------------
        public MainWindow()
        {
            InitializeComponent();
            return;

            //--- Data Context -----------------------------------
            DataContext  = this;
            Record_Toolbar.DataContext     = RecordNo.The_RecordNo;
            FileViewPage = new FileView();
            LayoutPage   = new LayoutView();

            //--- menu commands -------------------------------
            NewJobCommand    = new RxCommand(OnNewJob);
            LoadJobCommand   = new RxCommand(OnLoadJob);
            SaveJobCommand   = new RxCommand(OnSaveJob, OnCanSaveJob);
            SaveAsJobCommand = new RxCommand(OnSaveAsJob);
            ExitCommand      = new RxCommand(OnExit);
       }
        
        //--- Property FileViewPage -----------------------------------------------------
        private FileView _FileViewPage;
        public FileView FileViewPage
        {
            get { return _FileViewPage; }
            set { _FileViewPage = value; }
        }
        
        //--- LayoutPage -----------------------------------------------------------------
        private LayoutView _LayoutPage;
        public LayoutView LayoutPage
        {
            get { return _LayoutPage; }
            set { _LayoutPage = value; }
        }

        //--- Window_Loaded ---------------------------------------------------------------
        private void Window_Loaded(object sender, System.EventArgs e)
        {
            FileViewPage.FileDef = _FileDef;
            LayoutPage.FileDef   = _FileDef;
            LayoutPage.LayoutDef = _LayoutDef;
            LayoutPage.CounterDef= _CounterDef;
            RecordNo.The_RecordNo.OnRecNoChanged += LayoutPage.ActiveRecNoChanged;

            //--- restore settings ---------------------------------------
            RxGlobals.Settings.Load(null);
            WindowState = (WindowState)RxGlobals.Settings.WindowState;
            Width       = RxGlobals.Settings.WindowWidth;
            Height      = RxGlobals.Settings.WindowHeight;

            RX_Rip.rip_init();
        }

        //--- Window_Unloaded --------------------------------------------------------------
        private void Window_Unloaded(object sender, RoutedEventArgs e)
        {
        }

        //--- Save_Changes -----------------------------------------------
        private MessageBoxResult Save_Changes(string title)
        {
            _LayoutDef.SaveBoxProperties();
            if (_FileDef.Changed || _LayoutDef.Changed || _CounterDef.Changed)
            {
                MessageBoxResult result = System.Windows.MessageBox.Show("Definiton has changed. Save it?", title, MessageBoxButton.YesNoCancel);
                switch(result)
                {
                    case MessageBoxResult.Yes: OnSaveJob(this);  break;
                    case MessageBoxResult.No: break;
                    case MessageBoxResult.Cancel: break;
                }
                return result;
            }
            return MessageBoxResult.No;
        }

        //--- Window_Closing -------------------------------------------------------------------
        private void Window_Closing(object sender, CancelEventArgs e)
        {
            if (Save_Changes("Closing")==MessageBoxResult.Cancel) e.Cancel = true;

            //--- save settings -----------------------------------------
            RxGlobals.Settings.WindowState  = this.WindowState;
            RxGlobals.Settings.WindowWidth  = (int)this.ActualWidth;
            RxGlobals.Settings.WindowHeight = (int)this.ActualHeight;
            RxGlobals.Settings.LastUsedFile = JobFilename;
            RxGlobals.Settings.Save(null);
        } 

        //--- Property JobFilename ---------------------------------------------------------
        private string _JobFilename;
        public string JobFilename
        {
            get { return _JobFilename; }
            set 
            { 
                _JobFilename = value; 
                Title = JobFilename;
            }
        }
                
        //--- MENU commands ---------------------------------
        public ICommand ExitCommand { get; private set; }
        void OnExit(object parameter)
        {
            this.Close();
        }

        //--- OnNewJob --------------------------------------------------------
        public ICommand NewJobCommand { get; private set; }
        void OnNewJob(object parameter)
        {
            if (Save_Changes("New File")==MessageBoxResult.Cancel) return;

            SaveFileDialog dlg = new SaveFileDialog();

            dlg.Filter = "label definition files (*.rxd)|*.rxd|All files (*.*)|*.*";
            dlg.RestoreDirectory = true;

            if (dlg.ShowDialog() == System.Windows.Forms.DialogResult.OK)
            {
                LoadJob(dlg.FileName);
            }
        }

        //--- OnLoadJob --------------------------------------------------------
        public ICommand LoadJobCommand { get; private set; }
        void OnLoadJob(object parameter)
        {
            if (Save_Changes("Load File")==MessageBoxResult.Cancel) return;

            OpenFileDialog dlg = new OpenFileDialog();

            dlg.Filter = "label definition files (*.rxd)|*.rxd|All files (*.*)|*.*";
            dlg.RestoreDirectory = true;
           
            if (dlg.ShowDialog() == System.Windows.Forms.DialogResult.OK)
            {
                LoadJob(dlg.FileName);
            }
        }

        //--- LoadJob -----------------------------------
        private void LoadJob(string path)
        {
            if (path!=null && !path.Equals(""))
            {
                System.Windows.Forms.Cursor.Current = System.Windows.Forms.Cursors.WaitCursor;

                JobFilename = path;
                RxGlobals.Dir = System.IO.Path.GetDirectoryName(path)+"\\";

                // int doc = RX_Doc.rx_xml_load(JobFilename);
                IntPtr doc = RX_Rip.rip_open_xml_local(path, System.IO.Path.GetTempPath());
                RX_Rip.rip_load_files(doc, System.IO.Path.GetTempPath(), null);                
                _FileDef.Load(doc);
                _CounterDef.Load(doc);
                LayoutPage.FileDef = _FileDef;
                _LayoutDef.Load(doc);
                LayoutPage.CounterDef = null; 
                LayoutPage.CounterDef = _CounterDef;
                RX_Doc.rx_xml_free(doc);
                RecordNo.The_RecordNo.RecNo=0;

                System.Windows.Forms.Cursor.Current = System.Windows.Forms.Cursors.Default;
            }
        }

        //--- SaveJob -----------------------------------
        private void SaveJob(string path)
        {
            if (path!=null && !path.Equals(""))
            {
                System.Windows.Forms.Cursor.Current = System.Windows.Forms.Cursors.WaitCursor;
                
                StringBuilder dstPath=new StringBuilder(path, 256);
                IntPtr doc = RX_Doc.rx_xml_new();
                _LayoutDef.Save(dstPath, doc);
                _FileDef.Save(dstPath, doc);
                _CounterDef.Save(doc);
                RX_Doc.rx_xml_save(dstPath.ToString(), doc);
                RX_Doc.rx_xml_free(doc);
                
                System.Windows.Forms.Cursor.Current = System.Windows.Forms.Cursors.Default;
                
                RxGlobals.Dir = System.IO.Path.GetDirectoryName(path)+"\\";
            }
        }

        //--- SaveJobCommand ------------------------------------------------------------------------
        public ICommand SaveJobCommand { get; private set; }
        void OnSaveJob(object parameter)
        {
            SaveJob(JobFilename);
        }

        //--- OnCanSaveJob ------------------------------------------------------------------
        bool OnCanSaveJob(object parameter)
        {
            return JobFilename != null && JobFilename != "";
        }

        //--- OnSaveAsJob -------------------------------------------------------------------
        public ICommand SaveAsJobCommand { get; private set; }
        void OnSaveAsJob(object parameter)
        {
            SaveFileDialog dlg = new SaveFileDialog();

            dlg.Filter = "label definition files (*.rxd)|*.rxd|All files (*.*)|*.*";
            dlg.RestoreDirectory = true;

            if (dlg.ShowDialog() == System.Windows.Forms.DialogResult.OK)
            {
                JobFilename = dlg.FileName;
                SaveJob(JobFilename);
            }
        }

        //--- MainFrame_Navigated -------------------------------------------------------------
        private void MainFrame_Navigated(object sender, System.Windows.Navigation.NavigationEventArgs e)
        {
        }
        
        //--- Analyse_Clicked --------------------------------------------------------- 
        private void Analyse_Clicked(object sender, RoutedEventArgs e)
        {
            _FileDef.Analyse();
        }

        //--- FieldNames_Clicked --------------------------------------------------------- 
        private void FieldNames_Clicked(object sender, RoutedEventArgs e)
        {
            _FileDef.RecToFieldNames();
        }
        
        //--- Update_Clicked --------------------------------------------------------- 
        private void Update_Clicked(object sender, RoutedEventArgs e)
        {
            RxUpdate.DoUpdate();
        }

        //--- Doc_Clicked --------------------------------------------------------- 
        private void Doc_Clicked(object sender, RoutedEventArgs e)
        {
            try { System.Diagnostics.Process.Start(@".\RX-LabelComposer.pdf");}
            catch { };
        }

        //--- Samples_Clicked --------------------------------------------------------- 
        private void Samples_Clicked(object sender, RoutedEventArgs e)
        {
            try 
            {  
                System.Diagnostics.Process.Start(Samples_URL);
            }
            catch { System.Windows.MessageBox.Show("No internet access."); };
        }

        //--- Radex_Clicked --------------------------------------------------------- 
        private void Radex_Clicked(object sender, RoutedEventArgs e)
        {
            try {  System.Diagnostics.Process.Start(HomePage_URL);}
            catch { System.Windows.MessageBox.Show("No internet access."); };
        }
       
        //--- About_Clicked --------------------------------------------------------- 
        private void About_Clicked(object sender, RoutedEventArgs e)
        {
            AboutDlg dlg = new AboutDlg(typeof(MainWindow).Assembly.GetName());
            dlg.ShowDialog();
        }

        //--- Window_ContentRendered ---------------------------------------------------------------------
        private static bool first=true;
        private void Window_ContentRendered(object sender, EventArgs e)
        {
            if (first)
            {
                if (RxGlobals.Settings.LastUsedFile!=null && !RxGlobals.Settings.LastUsedFile.Equals("")) LoadJob(RxGlobals.Settings.LastUsedFile);

                //--- save settings -----------------------------------------
#if (RELEASE)
                    RxGlobals.Settings.LastUsedFile = "";
                    RxGlobals.Settings.Save();
#endif

                first=false;
            }
        }

        //--- XamTabControl_SelectionChanged -------------------------------------------------
        private void XamTabControl_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            MenuData.IsEnabled = (ViewTab.SelectedIndex==1);
        }
    }
#endif
}
