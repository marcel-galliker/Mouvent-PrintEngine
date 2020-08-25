using RX_Common;
using RX_DigiPrint.Helpers;
using RX_DigiPrint.Services;
using RX_DigiPrint.Views.Density;
using RX_DigiPrint.Views.LB702WBView;
using RX_DigiPrint.Views.LH702View;
using RX_DigiPrint.Views.PrintQueueView;
using RX_DigiPrint.Views.UserControls;
using System;
using System.Collections.Generic;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Threading;

namespace RX_DigiPrint.Models
{
    public class RxGlobals
    {
        public static RxTimer           Timer             = new RxTimer(500);
        public static Settings          Settings          = new Settings();
        public static PrinterProperties PrinterProperties = new PrinterProperties(){init=true};
        public static LogList           Log               = new LogList();
        public static EventList         Events            = new EventList();
        public static PrinterStatus     PrinterStatus     = new PrinterStatus();
        public static PrintQueue        PrintQueue        = new PrintQueue();
        public static PrintQueueView    PrintQueueView    = null;  
        public static RxLanguage        Language          = new RxLanguage();
        public static User              User              = new User();
        public static RxInterface       RxInterface       = new RxInterface();
        public static RxBluetooth       Bluetooth         = new RxBluetooth();
        public static RxBtProdState     BtProdState       = new RxBtProdState();
        public static StepperCfg        Stepper           = new StepperCfg();
        public static Chiller Chiller = new Chiller();
        public static PrintSystem       PrintSystem       = new PrintSystem();
        public static StepperStatus[] StepperStatus = new StepperStatus[4] {
            new StepperStatus(0), new StepperStatus(1), new StepperStatus(2), new StepperStatus(3) 
        };
        public static InkFamilyList     InkFamilies       = new InkFamilyList();
        public static InkTypeList       InkTypes          = new InkTypeList();
        public static InkSupplyList     InkSupply         = new InkSupplyList();
        public static HeadStatList      HeadStat          = new HeadStatList();
        public static ClusterStatList   ClusterStat       = new ClusterStatList();

        public static Plc               Plc               = new Plc();
        
        public static Encoder[] Encoder = new Encoder[2] { new Encoder(0), new Encoder(1) };
        public static NetworkList       Network           = new NetworkList();
        public static RxScreen          Screen            = new RxScreen();
        public static PreviewCash       PreviewCash       = new PreviewCash();
        public static HeadAdjustment    HeadAdjustment    = new HeadAdjustment();
        public static MaterialList      MaterialList      = new MaterialList();
        public static MaterialXml       MaterialXML       = new MaterialXml();
        public static MvtTextPad        Keyboard          = new MvtTextPad(null);
        public static RxTaskSwitch      TaskSwitch        = new RxTaskSwitch();
        public static RxPopup           Popup;
        public static UvLamp            UvLamp            = new UvLamp();
        public static Window            MainWindow;
        public static Window            BluetoothLoginWnd;
        public static License           License           = new License();
        public static CleafOrder        CleafOrder        = new CleafOrder();
        public static Alignment             Alignment = new Alignment();
        public static AlignmentResources    AlignmentResources = new AlignmentResources();
        public static LB702WB_Machine       LB702WB_Machine;
        public static LH702_View            LH702_View;
        public static LH702_Preview         LH702_Preview;
        public static DisabledJets          DisabledJets = new DisabledJets();
        public static Density               Density = new Density();
        
    }
}
