using System;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Text;
using System.Windows;
using System.Windows.Input;
using Microsoft.Win32;
using RX_Common;
using RX_LabelComposer.External;
using RX_LabelComposer.Models;
using RX_LabelComposer.Views;

namespace RX_LabelComposer.ViewModels
{
	public class MainViewModel : RxBindable, IMainViewModel
	{
		//private string HomePage_URL = "http://www.radex-net.com";
        //private string Samples_URL  = "http://www.radex-net.com/Downloads/RX-LabelComposer/Samples/Samples.zip";

        static FileDefinition       _FileDef    = FileDefinition.The_FileDefinition;
        static LayoutDefinition     _LayoutDef  = LayoutDefinition.The_LayoutDef;
        static CounterDefinition    _CounterDef = CounterDefinition.The_CounterDefinition;

        public MainViewModel()
        {
            //--- menu commands -------------------------------
            // File
            this.NewJobCommand    = new RxCommand(OnNewJob);
            this.LoadJobCommand   = new RxCommand(OnLoadJob);
            this.SaveJobCommand   = new RxCommand(OnSaveJob, OnCanSaveJob);
            this.SaveAsJobCommand = new RxCommand(OnSaveAsJob);
            this.ExitCommand      = new RxCommand(OnExit);
            // Data
            this.AnalyseCommand    = new RxCommand((dummy) => { });
            this.FieldNamesCommand = new RxCommand((dummy) => { }); 
            // Help
            this.DocCommand                = new RxCommand((dummy) => { });
            this.CheckForUpdateCommand     = new RxCommand((dummy) => { });
            this.DownloadSamplesCommand    = new RxCommand((dummy) => { });
            this.VisitRadexHomepageCommand = new RxCommand((dummy) => { });
            this.AboutCommand              = new RxCommand((dummy) => { });

			//--- window events commands ----------------------------------------------------------
			this.WindowLoadedCommand   = new RxCommand(this.WindowLoadedInteraction);
            this.WindowUnloadedCommand = new RxCommand(this.WindowUnloadedInteraction);
            this.WindowClosingCommand  = new RxCommand(this.WindowClosingInteraction);
            this.WindowContentRenderedCommand 
                                       = new RxCommand(this.WindowContentRenderedInteraction);

			//--- tab items -----------------------------------
			this.LayoutPage = new LayoutView();
            this.FileViewPage = new FileView();
            //this.TabItems.Add(this.LayoutPage);
            //this.TabItems.Add(this.FileViewPage);
        }

        #region Menu

		public ICommand NewJobCommand { get; private set; }

		public ICommand LoadJobCommand { get; private set; }

		public ICommand SaveJobCommand { get; private set; }

		public ICommand SaveAsJobCommand { get; private set; }

		public ICommand ExitCommand { get; private set; }

		public ICommand AnalyseCommand { get; private set; }

		public ICommand FieldNamesCommand { get; private set; }

		public ICommand DocCommand { get; private set; }

		public ICommand CheckForUpdateCommand { get; private set; }

		public ICommand DownloadSamplesCommand { get; private set; }

		public ICommand VisitRadexHomepageCommand { get; private set; }

		public ICommand AboutCommand { get; private set; }

        //--- OnNewJob --------------------------------------------------------
		void OnNewJob(object parameter)
        {
            if (Save_Changes("New File")==MessageBoxResult.Cancel) return;

            SaveFileDialog dlg = new SaveFileDialog
            {
                Filter           = "label definition files (*.rxd)|*.rxd|All files (*.*)|*.*",
                RestoreDirectory = true
            };

            if (dlg.ShowDialog() == true)
            {
                LoadJob(dlg.FileName);
            }
        }

        //--- OnLoadJob --------------------------------------------------------
        void OnLoadJob(object parameter)
        {
            if (Save_Changes("Load File")==MessageBoxResult.Cancel) return;

            OpenFileDialog dlg = new OpenFileDialog
            {
                Filter           = "label definition files (*.rxd)|*.rxd|All files (*.*)|*.*",
                RestoreDirectory = true
            };

            if (dlg.ShowDialog() == true)
            {
                LoadJob(dlg.FileName); 
            }
        }

        //--- SaveJobCommand ------------------------------------------------------------------------
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
        void OnSaveAsJob(object parameter)
        {
            SaveFileDialog dlg = new SaveFileDialog
            {
                Filter           = "label definition files (*.rxd)|*.rxd|All files (*.*)|*.*",
                RestoreDirectory = true
            };

            if (dlg.ShowDialog() == true)
            {
                JobFilename = dlg.FileName;
                SaveJob(JobFilename);
            }
        }

		void OnExit(object parameter)
        {
			if (parameter is Window window)
			{
				window.Close();
			}
        }

        #endregion

        #region Record Navigation ToolBar

        public RecordNo CurrentRecord => RecordNo.The_RecordNo;

        #endregion

        #region Tab Items

        public ObservableCollection<object> TabItems { get; private set; } = new ObservableCollection<object>();

        public LayoutView LayoutPage { get; private set; }
        public FileView FileViewPage { get; private set; }

        #endregion

        #region Windows Properties

		private String _Title = null;
		public String Title
		{
			get => _Title;
			private set => SetProperty(ref _Title, value);
		}

		#endregion

        #region Windows Events

        public ICommand WindowLoadedCommand { get; private set; }
        public ICommand WindowUnloadedCommand { get; private set; }
        public ICommand WindowClosingCommand { get; private set; }
        public ICommand WindowContentRenderedCommand { get; private set; }

        //--- Window Loaded -----------------------------------------------------------------------
        private void WindowLoadedInteraction(object param)
        {
            Window window = param as Window;

            FileViewPage.FileDef = _FileDef;
            LayoutPage.FileDef   = _FileDef;
            LayoutPage.LayoutDef = _LayoutDef;
            LayoutPage.CounterDef= _CounterDef;
            RecordNo.The_RecordNo.OnRecNoChanged += LayoutPage.ActiveRecNoChanged;

            //--- restore settings ---------------------------------------
            RxGlobals.Settings.Load(null);
            window.WindowState = (WindowState)RxGlobals.Settings.WindowState;
            window.Width       = RxGlobals.Settings.WindowWidth;
            window.Height      = RxGlobals.Settings.WindowHeight;
            RX_Rip.rip_init();
        }

        //--- Window Unloaded ---------------------------------------------------------------------
        private void WindowUnloadedInteraction(object param)
        {

        }

        //--- Window Closing ----------------------------------------------------------------------
        private void WindowClosingInteraction(object param)
        {
            // TODO: FIX
            CancelEventArgs e = param as CancelEventArgs;
            if (Save_Changes("Closing")==MessageBoxResult.Cancel) e.Cancel = true;

            //--- save settings -----------------------------------------
            // TODO: FIX
            /*
            RxGlobals.Settings.WindowState  = this.WindowState;
            RxGlobals.Settings.WindowWidth  = (int)this.ActualWidth;
            RxGlobals.Settings.WindowHeight = (int)this.ActualHeight;
            */
            RxGlobals.Settings.LastUsedFile = JobFilename;
            RxGlobals.Settings.Save(null);
        }

        //--- Window Content Rendered -------------------------------------------------------------
        private static bool first = true;
        private void WindowContentRenderedInteraction(object param)
        {
            if (first)
            {
                if (!String.IsNullOrEmpty(RxGlobals.Settings.LastUsedFile))
                {
                    LoadJob(RxGlobals.Settings.LastUsedFile);
                }

                //--- save settings -----------------------------------------
                #if (RELEASE)
                    RxGlobals.Settings.LastUsedFile = "";
                    RxGlobals.Settings.Save();
                #endif

                first=false;
            }
        }

		#endregion

		#region Helpers

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

		 //--- LoadJob -----------------------------------
        private void LoadJob(string path)
        {
            if (!string.IsNullOrEmpty(path) && System.IO.File.Exists(path))
            {
                Mouse.OverrideCursor = Cursors.Wait;
                //System.Windows.Forms.Cursor.Current = System.Windows.Forms.Cursors.WaitCursor;

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

                Mouse.OverrideCursor = Cursors.Arrow;
                //System.Windows.Forms.Cursor.Current = System.Windows.Forms.Cursors.Default;
            }
        }

        //--- SaveJob -----------------------------------
        private void SaveJob(string path)
        {
            if (path!=null && !path.Equals(""))
            {
                Mouse.OverrideCursor = Cursors.Wait;
                //System.Windows.Forms.Cursor.Current = System.Windows.Forms.Cursors.WaitCursor;
                
                StringBuilder dstPath=new StringBuilder(path, 256);
                IntPtr doc = RX_Doc.rx_xml_new();
                _LayoutDef.Save(dstPath, doc);
                _FileDef.Save(dstPath, doc);
                _CounterDef.Save(doc);
                RX_Doc.rx_xml_save(dstPath.ToString(), doc);
                RX_Doc.rx_xml_free(doc);
                
                Mouse.OverrideCursor = Cursors.Arrow;
                //System.Windows.Forms.Cursor.Current = System.Windows.Forms.Cursors.Default;
                
                RxGlobals.Dir = System.IO.Path.GetDirectoryName(path)+"\\";
            }
        }

        //--- Property JobFilename ---------------------------------------------------------
        private string _JobFilename;
        public string JobFilename
        {
            get { return _JobFilename; }
            set 
            { 
                _JobFilename = value; 
                this.Title = JobFilename;
            }
        }

		#endregion
	}
}
