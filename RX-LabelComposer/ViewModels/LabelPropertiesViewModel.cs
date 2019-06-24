using Microsoft.Win32;
using RX_Common;
using RX_LabelComposer.Models;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;

namespace RX_LabelComposer.ViewModels
{
    public interface ILabelPropertiesViewModel
    {
        ICommand SelectColorLayerCommand { get; }
        ICommand ShowLayersCommand { get; }

        double WebWidth   { get; set; }
        int    Columns    { get; set; }
        double ColumnDist { get; set; }

        #region Unknown

        bool IsLabelControlChecked { get; set; }
        bool IsLabelControlEnabled { get; }
        ICommand LabelCommand { get; }

        String Label { get; set; }

        bool IsColorControlChecked { get; set; }
        bool IsColorControlEnabled { get; }
        String ColorLayer { get; set; }

        bool IsVariableControlEnabled { get; }
        bool IsVariableControlChecked { get; set; }

        #endregion

        /// <summary>
        /// Determines if the Label width and height properties are enabled.
        /// </summary>
        bool LabelUnused { get; }

        double LabelWidth { get; set; }
        double LabelHeight { get; set; }
    }

    public class LabelPropertiesViewModel : RxBindable, ILabelPropertiesViewModel
    {
        private LayoutDefinition _LayoutDef;// => LayoutDefinition.The_LayoutDef;

        public ICommand SelectColorLayerCommand { get; private set; }
        public ICommand ShowLayersCommand { get; private set; }
        public ICommand LabelCommand { get; private set; }

        public LabelPropertiesViewModel(LayoutDefinition model)
        {
            _LayoutDef = model;
            _LayoutDef.PropertyChanged += _LayoutDef_PropertyChanged;

            this.SelectColorLayerCommand = new RxCommand(this.SelectColorLayerInteraction);
            this.ShowLayersCommand = new RxCommand(this.ShowLayersInteraction);
            this.LabelCommand = new RxCommand(this.LabelInteraction);
        }

        private void _LayoutDef_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            if (sender != _LayoutDef)
                throw new ArgumentException("The sender '{sender}' should be the same as the layout definition '{_LayoutDef}'.");

            switch (e.PropertyName)
            {
                case nameof(LayoutDefinition.WebWidth):
                    OnPropertyChanged(nameof(ILabelPropertiesViewModel.WebWidth));
                    break;
                case nameof(LayoutDefinition.Columns):
                    OnPropertyChanged(nameof(ILabelPropertiesViewModel.Columns));
                    break;
                case nameof(LayoutDefinition.ColumnDist):
                    OnPropertyChanged(nameof(ILabelPropertiesViewModel.ColumnDist));
                    break;
                case nameof(LayoutDefinition.LabelUnused):
                    OnPropertyChanged(nameof(ILabelPropertiesViewModel.LabelUnused));
                    break;
                case nameof(LayoutDefinition.LabelWidth):
                    OnPropertyChanged(nameof(ILabelPropertiesViewModel.LabelWidth));
                    break;
                case nameof(LayoutDefinition.LabelHeight):
                    OnPropertyChanged(nameof(ILabelPropertiesViewModel.LabelHeight));
                    break;
                case nameof(LayoutDefinition.LabelVisible):
                    OnPropertyChanged(nameof(ILabelPropertiesViewModel.IsLabelControlChecked));
                    break;
                case nameof(LayoutDefinition.ColorLayer):
                    OnPropertyChanged(nameof(ILabelPropertiesViewModel.ColorLayer));
                    break;
                case nameof(LayoutDefinition.Label):
                    OnPropertyChanged(nameof(ILabelPropertiesViewModel.Label));
                    break;
                default:
                    //Debug.WriteLine($"Property changed event for name '{e.PropertyName}' in '{sender}' not handled.");
                    break;
            }
        }

        public double WebWidth
        {
            get {return _LayoutDef.WebWidth;}
            set { _LayoutDef.WebWidth = value;}
        }

        public int Columns
        {
            get => _LayoutDef.Columns;
            set => _LayoutDef.Columns = value;
        }

        public double ColumnDist
        {
            get => _LayoutDef.ColumnDist;
            set => _LayoutDef.ColumnDist = value;
        }

        #region Unknown

        public bool IsLabelControlChecked
        {
            get => _LayoutDef.LabelVisible;
            set => _LayoutDef.LabelVisible = value;
        }

        private bool _IsLabelControlEnabled = false;
        public bool IsLabelControlEnabled
        {
            get => _IsLabelControlEnabled;
            set => SetProperty(ref _IsLabelControlEnabled, value);
        }

        public String Label
        {
            get => _LayoutDef.Label;
            set => _LayoutDef.Label = value;
        }

        private bool _IsColorControlChecked = false;
        public bool IsColorControlChecked
        {
            get => _IsColorControlChecked;
            set => SetProperty(ref _IsColorControlChecked, value);
        }

        private bool _IsColorControlEnabled = false;
        public bool IsColorControlEnabled
        {
            get => _IsColorControlEnabled;
            set => SetProperty(ref _IsColorControlEnabled, value);
        }

        public String ColorLayer
        {
            get => _LayoutDef.ColorLayer;
            set => _LayoutDef.ColorLayer = value;
        }

        private bool _IsVariableControlChecked = false;
        public bool IsVariableControlChecked
        {
            get => _IsVariableControlChecked;
            set => SetProperty(ref _IsVariableControlChecked, value);
        }

        private bool _IsVariableControlEnabled = false;
        public bool IsVariableControlEnabled
        {
            get => _IsVariableControlEnabled;
            set => SetProperty(ref _IsVariableControlEnabled, value);
        }

        #endregion

        public bool LabelUnused
        {
            get => _LayoutDef.LabelUnused;
            set => _LayoutDef.LabelUnused = value;
        }

        public double LabelWidth
        {
            get => _LayoutDef.LabelWidth;
            set => _LayoutDef.LabelWidth = value;
        }

        public double LabelHeight
        {
            get => _LayoutDef.LabelHeight;
            set => _LayoutDef.LabelHeight = value;
        }

        //--- Label_Clicked -----------------------------------------------------------------------
        private void LabelInteraction(object dummy)
        {
            OpenFileDialog dlg = new OpenFileDialog
            {
                Filter = "bmp files (*.bmp)|*.bmp|All files (*.*)|*.*",
                FilterIndex = 1,
                //RestoreDirectory = true
            };

            if (dlg.ShowDialog() == true)
                _LayoutDef.Label = dlg.FileName;
        }

        //--- ColorLayer_Clicked ------------------------------------------------------------------
        private void SelectColorLayerInteraction(object dummy)
        {
            OpenFileDialog dlg = new OpenFileDialog
            {
                Filter = "bmp files (*.bmp)|*.bmp|All files (*.*)|*.*",
                FilterIndex = 1,
                RestoreDirectory = true
            };

            if (dlg.ShowDialog() == true)
                 _LayoutDef.ColorLayer = dlg.FileName;
        }

        //--- show_layers -------------------------------------------------------------------------
        private void ShowLayersInteraction(object dummy)
        {
            this.ShowLayersAction(null, null);
        }

        public Action<object, RoutedEventArgs> ShowLayersAction { get; set; }
    }
}
