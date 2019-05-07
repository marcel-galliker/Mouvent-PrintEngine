using RX_Common;
using RX_LabelComposer.External;
using RX_LabelComposer.Models.Enums;
using RX_LabelComposer.Models.Models.Enums;
using System;
using System.Linq;
using System.Windows;
using System.Windows.Input;

namespace RX_LabelComposer.Models
{
    public class BarcodeProperties : RxBindable
    {
        private SBarcodeBox _BcBoxS;
        private int _barCnt=4;

        public BarcodeProperties(SBarcodeBox bcBoxS)
        {
            BarcodeBox = bcBoxS;
            
            BarDefaultsCommand = new RxCommand(OnBarDefaults, OnCanBarDefaults);
        }

        public SBarcodeBox BarcodeBox
        {
            get { return _BcBoxS; }
            set { 
                SetProperty(ref _BcBoxS, value);
                ShowControls();
            }
        }

        private void ShowControls()
        {
            foreach (BcProperties bcType in BarcodeTypeList)
            {
                if (_BcBoxS.bcType == bcType.Value)
                {
                    if (bcType.Font != null)
                    {
                        FontVisible = Visibility.Visible;
                        if (bcType.Above == BcFontAboveEnum.variable) FontAboveVisible = Visibility.Visible;
                        else
                        {
                            FontAboveVisible = Visibility.Collapsed;
                            if (bcType.Above == BcFontAboveEnum.fix_true) FontAbove = 1;
                            else FontAbove = 0;
                        }
                    }
                    else
                    {
                        FontVisible = Visibility.Collapsed;
                        FontAboveVisible = Visibility.Collapsed;
                    }

                    //--- bars -----------------------------------
                    int bars = TecIT.BCGetCountBars(_BcBoxS.bcType);
                    Visibility[] barVisible = new Visibility[_barCnt];
                    for (int i = 0; i < _barCnt; i++)
                    {
                        if (!bcType.Stretch && i < bars) barVisible[i] = Visibility.Visible;
                        else barVisible[i] = Visibility.Collapsed;
                    }
                    BarVisible = barVisible;

                    //--- spaces ----------------------------------------------
                    int spaces = TecIT.BCGetCountSpaces(_BcBoxS.bcType);
                    Visibility[] spaceVisible = new Visibility[_barCnt];
                    for (int i = 0; i < _barCnt; i++)
                    {
                        if (!bcType.Stretch && i < spaces) spaceVisible[i] = Visibility.Visible;
                        else spaceVisible[i] = Visibility.Collapsed;
                    }
                    SpaceVisible = spaceVisible;

                    //--- stretch ---------------------------------------
                    if (!bcType.Stretch) Stretch = 0;
                    else if (Stretch < 1) Stretch = 1;
                    StretchVisible = (bcType.Stretch)?Visibility.Visible:Visibility.Collapsed;

                    //--- filter ---------------------------------
                    BarcodeFilterList = new BarcodeFilterList(_BcBoxS.bcType);
                    FilterComboVisible = BarcodeFilterList.Visible();
                    FilterLabelVisible = (bcType.Stretch || BarcodeFilterList.Visible() == Visibility.Visible) ? Visibility.Visible : Visibility.Collapsed;
                    FilterVisible = (bcType.Stretch) ? Visibility.Visible : Visibility.Collapsed;

                    //--- size ---------------------------------
                    BarcodeSizeList = new BarcodeSizeList(_BcBoxS.bcType);
                    SizeVisible = BarcodeSizeList.Visible();

                    //--- format ---------------------------------
                    BarcodeFormatList = new BarcodeFormatList(_BcBoxS.bcType);
                    FormatVisible = BarcodeFormatList.Visible();

                    //--- check ---------------------------
                    BarcodeCheckList = new BarcodeCheckList(_BcBoxS.bcType);
                    CheckVisible = BarcodeCheckList.Visible();

                    //--- mask ------------------------------ 
                    BarcodeMaskList = new BarcodeMaskList(_BcBoxS.bcType);
                    MaskVisible = BarcodeMaskList.Visible();

                    //--- hex -----------------------------
                    HexVisible = (_BcBoxS.bcType == BcTypeEnum.eBC_DataMatrix) ? Visibility.Visible : Visibility.Collapsed;

                    //--- composite ----------------
                    BarcodeCompositeList = new BarcodeCompositeList(_BcBoxS.bcType);
                    CcTypeVisible = BarcodeCompositeList.Visible();

                    //--- PDF14- rows and cols ------
                    BarcodeRowsList = new BarcodeRowsList(_BcBoxS.bcType);
                    RowsVisible = BarcodeRowsList.Visible();
                    BarcodeColsList = new BarcodeColsList(_BcBoxS.bcType);
                    ColsVisible = BarcodeColsList.Visible();

                    BarcodeSegPerRowList = new BarcodeSegPerRowList(_BcBoxS.bcType);
                    SegPerRowVisible = BarcodeSegPerRowList.Visible();

                    CodePageVisible = Visibility.Collapsed;

                    int n = RX_Rip.rip_bc_states(_BcBoxS.bcType);
                    LongBarVisible  = (n==2 || n==4)? Visibility.Visible:Visibility.Collapsed;
    
                    return;
                }
            }

            //--- default values --------------------------------
            FontVisible     = Visibility.Collapsed;
            FontAboveVisible= Visibility.Collapsed;
            for (int i = 0; i < BarVisible.Count(); i++)   BarVisible[i]   = Visibility.Collapsed;
            for (int i = 0; i < SpaceVisible.Count(); i++) SpaceVisible[i] = Visibility.Collapsed;
            StretchVisible  = Visibility.Collapsed;
            FilterLabelVisible  = Visibility.Collapsed;
            FilterVisible       = Visibility.Collapsed;
            FilterComboVisible  = Visibility.Collapsed;
            SizeVisible     = Visibility.Collapsed;
            FormatVisible   = Visibility.Collapsed;
            CheckVisible    = Visibility.Collapsed;
            MaskVisible     = Visibility.Collapsed;
            HexVisible      = Visibility.Collapsed;
            CcTypeVisible   = Visibility.Collapsed;
            ColsVisible     = Visibility.Collapsed;
            RowsVisible     = Visibility.Collapsed;
            SegPerRowVisible= Visibility.Collapsed;
            CodePageVisible = Visibility.Collapsed;
            LongBarVisible  = Visibility.Collapsed;
        }

        public BarcodeTypeList BarcodeTypeList { get { return new BarcodeTypeList(); } }
        public BcTypeEnum BcType
        {
            get { return _BcBoxS.bcType; }
            set
            {
                if (SetProperty(ref _BcBoxS.bcType, value))
                    ShowControls();            
            }
        }

        private Visibility[] _BarVisible = new Visibility[4];
        public Visibility[] BarVisible
        {
            get { return _BarVisible;}
            set { SetProperty(ref _BarVisible, value); }
        }

        public short[] Bar
        {
            get { return _BcBoxS.bar; }
            set { SetProperty(ref _BcBoxS.bar, value); }
        }

        private Visibility[] _SpaceVisible = new Visibility[4];
        public Visibility[] SpaceVisible
        {
            get { return _SpaceVisible; }
            set { SetProperty(ref _SpaceVisible, value); }
        }
        public short[] Space
        {
            get { return _BcBoxS.space; }
            set { SetProperty(ref _BcBoxS.space, value); }
        }

        private Visibility _LongBarVisible = new Visibility();
        public Visibility LongBarVisible
        {
            get { return _LongBarVisible; }
            set { SetProperty(ref _LongBarVisible, value); }
        }

        public ICommand BarDefaultsCommand { get; private set; }
        private void OnBarDefaults(object parameter)
        {
            short i, module, height;
            int cnt = 8;
            short[] bar   = new short[cnt];
            short[] space = new short[cnt];
            try { module = Convert.ToInt16(Bar[0]);} catch(Exception) {module=10;}
            try { height = Convert.ToInt16(Bar[1]);} catch(Exception) {height=100;}
            switch(RX_Rip.rip_bc_states(_BcBoxS.bcType))
            {
            case 2:     space[0] = bar[0] = module;
                        bar[1] = height;
                        bar[2] = (short)(height*.385);
                        break;

            case 3:     space[0] = bar[0] = module;
                        bar[1] = height;
                        bar[2] = (short)(height*0.125);
                        break;

            case 4:     space[0] = bar[0] = module;
                        bar[1] = height;
                        bar[2] = (short)(height*0.125);
                        break;
                        
            default:    for (i=0; i<cnt; i++)
                        {
                            bar[i] = space[i] = (short)(module * (i + 1));
                        }
                        break;          
            }
            Bar = bar;
            Space = space;
        }
        bool OnCanBarDefaults(object parameter)
        {
            return true;
        }

        public void OnBarChanged(string obj, string value)
        {
            string arr;
            int idx;
            Rx.SplitArrayName(obj, out arr, out idx);
            try
            {
                Int16[] bar = new Int16[Bar.Count()];
                Bar.CopyTo(bar, 0);
                bar[idx] = Convert.ToInt16(value);
                Bar = bar;
            }
            catch { }
        }

        public void OnSpaceChanged(string obj, string value)
        {
            string arr;
            int idx;
            Rx.SplitArrayName(obj, out arr, out idx);
            try
            {
                Int16[] bar = new Int16[Space.Count()];
                Space.CopyTo(bar, 0);
                bar[idx] = Convert.ToInt16(value);
                Space = bar;
            }
            catch { }
        }

        private Visibility _StretchVisible;
        public Visibility StretchVisible
        {
            get { return _StretchVisible; }
            set { SetProperty(ref _StretchVisible, value); }
        }
        public short Stretch
        {
            get { return _BcBoxS.stretch; }
            set {
                if (StretchVisible==Visibility.Visible && value < 1) value = 1;
                SetProperty(ref _BcBoxS.stretch, value); 
            }
        }

        private Visibility _FilterLabelVisibleVisible;
        public Visibility FilterLabelVisible
        {
            get { return _FilterLabelVisibleVisible; }
            set { SetProperty(ref _FilterLabelVisibleVisible, value); }
        }
        private Visibility _FilterVisible;
        public Visibility FilterVisible
        {
            get { return _FilterVisible; }
            set { SetProperty(ref _FilterVisible, value); }
        }
        private Visibility _FilterComboVisibleVisible;
        public Visibility FilterComboVisible
        {
            get { return _FilterComboVisibleVisible; }
            set { SetProperty(ref _FilterComboVisibleVisible, value); }
        }
        private BarcodeFilterList _BarcodeFilterList;
        public BarcodeFilterList BarcodeFilterList
        {
            get { return _BarcodeFilterList; }
            private set { SetProperty(ref _BarcodeFilterList, value); }
        }
        public short Filter
        {
            get { return _BcBoxS.filter; }
            set { SetProperty(ref _BcBoxS.filter, value); }
        }

        private Visibility _CodePageVisible;
        public Visibility CodePageVisible
        {
            get { return _CodePageVisible; }
            set { SetProperty(ref _CodePageVisible, value); }
        }
        public short CodePage
        {
            get { return _BcBoxS.codePage; }
            set { SetProperty(ref _BcBoxS.codePage, value); }
        }

        private BarcodeCheckList _BarcodeCheckList;
        public BarcodeCheckList BarcodeCheckList
        {
            get { return _BarcodeCheckList; }
            private set { SetProperty(ref _BarcodeCheckList, value); }
        }
        private Visibility _CheckVisible;
        public Visibility CheckVisible
        {
            get { return _CheckVisible; }
            set { SetProperty(ref _CheckVisible, value); }
        }
        public short Check
        {
            get { return _BcBoxS.check; }
            set { SetProperty(ref _BcBoxS.check, value); }
        }

        private Visibility _SizeVisible;
        public Visibility SizeVisible
        {
            get { return _SizeVisible; }
            set { SetProperty(ref _SizeVisible, value); }
        }
        private BarcodeSizeList _BarcodeSizeList;
        public BarcodeSizeList BarcodeSizeList
        {
            get { return _BarcodeSizeList; }
            private set{SetProperty(ref _BarcodeSizeList, value);}
        }
        public short Size
        {
            get { return _BcBoxS.size; }
            set { SetProperty(ref _BcBoxS.size, value); }
        }

        private Visibility _FormatVisible;
        public Visibility FormatVisible
        {
            get { return _FormatVisible; }
            set { SetProperty(ref _FormatVisible, value); }
        }
        private BarcodeFormatList _BarcodeFormatList;
        public BarcodeFormatList BarcodeFormatList
        {
            get { return _BarcodeFormatList; }
            private set { SetProperty<BarcodeFormatList>(ref _BarcodeFormatList, value); }
        }
        public short Format
        {
            get { return _BcBoxS.format; }
            set { SetProperty(ref _BcBoxS.format, value); }
        }

        private Visibility _MaskVisible;
        public Visibility MaskVisible
        {
            get { return _MaskVisible; }
            set { SetProperty(ref _MaskVisible, value); }
        }
        private BarcodeMaskList _BarcodeMaskList;
        public BarcodeMaskList BarcodeMaskList
        {
            get { return _BarcodeMaskList; }
            private set { SetProperty(ref _BarcodeMaskList, value); }
        }
        public short Mask
        {
            get { return _BcBoxS.mask; }
            set { SetProperty(ref _BcBoxS.mask, value); }
        }

        private Visibility _RowsVisible;
        public Visibility RowsVisible
        {
            get { return _RowsVisible; }
            set { SetProperty(ref _RowsVisible, value); }
        }
        private BarcodeRowsList _BarcodeRowsList;
        public BarcodeRowsList BarcodeRowsList
        {
            get { return _BarcodeRowsList; }
            private set { SetProperty(ref _BarcodeRowsList, value); }
        }
        public short Rows
        {
            get { return _BcBoxS.rows; }
            set { SetProperty(ref _BcBoxS.rows, value); }
        }

        private Visibility _ColsVisible;
        public Visibility ColsVisible
        {
            get { return _ColsVisible; }
            set { SetProperty(ref _ColsVisible, value); }
        }
        private BarcodeColsList _BarcodeColsList;
        public BarcodeColsList BarcodeColsList
        {
            get { return _BarcodeColsList; }
            private set { SetProperty(ref _BarcodeColsList, value); }
        }
        public short Cols
        {
            get { return _BcBoxS.cols; }
            set { SetProperty(ref _BcBoxS.cols, value); }
        }

        //--- Composit list ---------------------------------
        private Visibility _CcTypeVisible;
        public Visibility CcTypeVisible
        {
            get { return _CcTypeVisible; }
            set { SetProperty(ref _CcTypeVisible, value); }
        }        
        private BarcodeCompositeList _BarcodeCompositeList;
        public BarcodeCompositeList BarcodeCompositeList
        {
            get { return _BarcodeCompositeList; }
            private set { SetProperty(ref _BarcodeCompositeList, value); }
        }
        public short CcType
        {
            get { return _BcBoxS.ccType; }
            set { SetProperty(ref _BcBoxS.ccType, value); }
        }

        private Visibility _SegPerRowVisible;
        public Visibility SegPerRowVisible
        {
            get { return _SegPerRowVisible; }
            set { SetProperty(ref _SegPerRowVisible, value); }
        }
        private BarcodeSegPerRowList _BarcodeSegPerRowList;
        public BarcodeSegPerRowList BarcodeSegPerRowList
        {
            get { return _BarcodeSegPerRowList; }
            private set { SetProperty(ref _BarcodeSegPerRowList, value); }
        }
        public short SegPerRow
        {
            get { return _BcBoxS.segPerRow; }
            set { SetProperty(ref _BcBoxS.segPerRow, value); }
        }

        private Visibility _HexVisible;
        public Visibility HexVisible
        {
            get { return _HexVisible; }
            set { SetProperty(ref _HexVisible, value); }
        }
        public short Hex
        {
            get { return _BcBoxS.hex; }
            set { SetProperty<short>(ref _BcBoxS.hex, value); }
        }

        private Visibility _FontVisible;
        public Visibility FontVisible
        {
            get { return _FontVisible; }
            set { SetProperty(ref _FontVisible, value); }
        }

        public BcFontList FontList 
        {
            get { return new BcFontList(); } 
        }
        public string Font
        {
            get { return _BcBoxS.font; }
            set { SetProperty(ref _BcBoxS.font, value); }
        }

        public short FontSize
        {
            get { return _BcBoxS.fontSize; }
            set { SetProperty(ref _BcBoxS.fontSize, value); }
        }
        private Visibility _FontAboveVisible;
        public Visibility FontAboveVisible
        {
            get { return _FontAboveVisible; }
            set { SetProperty(ref _FontAboveVisible, value); }
        }
        public short FontAbove
        {
            get { return _BcBoxS.fontAbove; }
            set { SetProperty(ref _BcBoxS.fontAbove, value); }
        }
        public short FontDist
        {
            get { return _BcBoxS.fontDist; }
            set { SetProperty(ref _BcBoxS.fontDist, value); }
        }
    }
}
