using RX_LabelComposer.Models;
using RX_LabelComposer.Views;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Input;

namespace RX_LabelComposer.ViewModels
{
    public interface IMainViewModel
    {
        ICommand NewJobCommand { get; }
        ICommand LoadJobCommand { get; }
        ICommand SaveJobCommand { get; }
        ICommand SaveAsJobCommand { get; }
        ICommand ExitCommand { get; }

        ICommand AnalyseCommand { get; }
        ICommand FieldNamesCommand { get; }

        ICommand DocCommand { get; }
        ICommand CheckForUpdateCommand { get; }
        ICommand DownloadSamplesCommand { get; }
        ICommand VisitRadexHomepageCommand { get; }
        ICommand AboutCommand { get; }

        ObservableCollection<Object> TabItems { get; }

        LayoutView LayoutPage { get; }
        FileView FileViewPage { get; }

        String Title { get; }
        RecordNo CurrentRecord { get; }

        #region Window Events

        ICommand WindowLoadedCommand { get; }
        ICommand WindowUnloadedCommand { get; }
        ICommand WindowClosingCommand { get; }
        ICommand WindowContentRenderedCommand { get; }

		#endregion
	}
}
