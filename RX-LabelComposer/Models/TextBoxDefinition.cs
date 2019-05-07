using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using RX_JobComposer.Common;
using RX_JobComposer.Models;

namespace RX_JobComposer.Models
{
    public class TextBoxDefinition : BindableBase
    {
        private static int test = 0;

        public TextBoxDefinition(STextBox textDef)
        {
            TextDef = textDef;
            Size = ++test;
        }

        private STextBox _TextDef;
        public STextBox TextDef
        {
            get { return _TextDef; }
            set { SetProperty<STextBox>(ref _TextDef, value); }
        }
        
        public string Font
        {
            get { return _TextDef.font; }
            set { SetProperty<string>(ref _TextDef.font, value); }
        }

        public int Size
        {
            get { return _TextDef.size; }
            set { SetProperty<int>(ref _TextDef.size, value); }
        }

        public int LineSpace
        {
            get { return _TextDef.lineSpace; }
            set { SetProperty<int>(ref _TextDef.lineSpace, value); }
        }

        public int Alignment
        {
            get { return _TextDef.alignment; }
            set { SetProperty<int>(ref _TextDef.alignment, value); }
        }
    }
}
