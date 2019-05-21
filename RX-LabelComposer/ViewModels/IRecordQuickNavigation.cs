using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Input;

namespace RX_LabelComposer.ViewModels
{
    public interface IRecordQuickNavigation
    {
        ICommand ShowFirstRecordCommand { get; }
        ICommand ShowNextRecordCommand { get; }
        ICommand ShowPreviousRecordCommand { get; }

        String RecNoStr { get; }
    }
}
