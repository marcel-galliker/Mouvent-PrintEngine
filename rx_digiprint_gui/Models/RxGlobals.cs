﻿using RX_Common;
using RX_DigiPrint.Helpers;
using RX_DigiPrint.Services;
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
        public static PrinterProperties PrinterProperties = new PrinterProperties(){init=true};
        public static LogList           Log               = new LogList();
        public static EventList         Events            = new EventList();
        public static PrinterStatus     PrinterStatus     = new PrinterStatus();
        public static PrintQueue        PrintQueue        = new PrintQueue();
        public static Action            PrintQueueChanged = null;
        public static RxLanguage        Language          = new RxLanguage();
//      public static EUserType         User              = EUserType.usr_operator;
        public static User              User              = new User();
        public static RxInterface       RxInterface       = new RxInterface();
        public static RxBluetooth       Bluetooth         = new RxBluetooth();
        public static RxBtProdState     BtProdState       = new RxBtProdState();
        public static StepperCfg        Stepper           = new StepperCfg();
        public static PrintSystem       PrintSystem       = new PrintSystem();
        public static InkFamilyList     InkFamilies       = new InkFamilyList();
        public static InkTypeList       InkTypes          = new InkTypeList();
        public static InkSupplyList     InkSupply         = new InkSupplyList();
        public static HeadStatList      HeadStat          = new HeadStatList();
        public static TestTableStatus   TestTableStatus   = new TestTableStatus();
        public static Plc               Plc               = new Plc();
        public static Chiller           Chiller           = new Chiller();
        public static Encoder[]         Encoder           = new Encoder[2];
        public static NetworkList       Network           = new NetworkList();
        public static RxScreen          Screen            = new RxScreen();
        public static PreviewCash       PreviewCash       = new PreviewCash();
        public static HeadAdjustment    HeadAdjustment    = new HeadAdjustment();
        public static MaterialList      MaterialList      = new MaterialList();
        public static MaterialXml       MaterialXML       = new MaterialXml();
        public static RxTextPad         Keyboard          = new RxTextPad(null);
        public static RxTaskSwitch      TaskSwitch        = new RxTaskSwitch();
        public static RxPopup           Popup;
        public static UvLamp            UvLamp            = new UvLamp();
        public static Window            MainWindow;
        public static Window            BluetoothLoginWnd;
        public static License           License           = new License();
        public static CleafOrder        CleafOrder        = new CleafOrder();
    }
}
