using DirectShowLib;
using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;

namespace rx_CamLib.Models
{
    #region Interface to rx_AlignFilter

    //Keep in order as defined in dll !!!

    //Expose Interface of Align Filter
    [ComImport,
    System.Security.SuppressUnmanagedCodeSecurity,
    Guid("C915723A-FE83-4914-AAFA-C7C486A41AAC"),           //Filter Interface IFrx_AlignFilter
    InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IFrx_AlignFilter : IBaseFilter
    {
        #region General

        //Version Info
        [PreserveSig]
        void GetVersion(IntPtr VersionInfo, out UInt32 VersionInfoSize);
        //C++ Version Info
        [PreserveSig]
        UInt32 GetCppVersion();
        //WindowHandle of Hoste
        [PreserveSig]
        void SetHostPointer(IntPtr HostHwnd);
        //Graphics Device Name
        [PreserveSig]
        void GetDeviceName(IntPtr DeviceName);
        //Debug On
        [PreserveSig]
        void SetDebug(bool DebugOn);
        //Callback debug
        [PreserveSig]
        void SetCallbackDebug(bool CallbackDebug);
        [PreserveSig]
        //Debug Log To FIle
        void SetDebugLogToFile(bool DebugToFile);
        //Display Frame Timing
        [PreserveSig]
        void SetFrameTiming(bool DspFrameTime);
        //Take SnapShot and Save to
        [PreserveSig]
        bool TakeSnapShot(string SnapDirectory, string SnapFileName);

        //ShowOriginalImage
        [PreserveSig]
        void SetShowOriginalImage(bool ShowOriginalImage);
        [PreserveSig]
        bool GetShowOriginalImage();

        //Overlay-Text
        [PreserveSig]
        bool SetOverlayTxt(string OverlayTxt);
        //BlobTextColor
        [PreserveSig]
        void SetOverlayTextColor(UInt32 OverlayTextColor);
        [PreserveSig]
        UInt32 GetOverlayTextColor();
        //Font for Overlay Text
        [PreserveSig]
        void SetOverlayFont(IntPtr pLogFontStruct);
        [PreserveSig]
        void GetOverlayFont(IntPtr pLogFontStruct, out UInt32 LogFontSize);

        #endregion

        #region Binarization

        //Binarization
        [PreserveSig]
        void SetBinarizeMode(UInt32 BinarizeMode);			// //0: off, 1: Manual, 2: Auto, 3: ColorAdaptive
        [PreserveSig]
        UInt32 GetBinarizeMode();

        //Threshold
        [PreserveSig]
        void SetThreshold(UInt32 Threshold);
        [PreserveSig]
        UInt32 GetThreshold();

        //Show Histogram
        [PreserveSig]
        void ShowHistogram(bool ShowHistogram);         // Switch ShowHistogram On/Off

        #endregion

        #region Dilate-Erode

        //Num Dilate-Erodes
        [PreserveSig]
        void SetNumDilateErodes(UInt32 DilateErodes);
        [PreserveSig]
        UInt32 GetNumDilateErodes();

        //Num Extra-Erodes
        [PreserveSig]
        void SetNumExtraErodes(UInt32 ExtraErodes);
        [PreserveSig]
        UInt32 GetNumExtraErodes();

        //Erode Seed
        [PreserveSig]
        void SetErodeSeedX(UInt32 ErodeSeedX);
        [PreserveSig]
        void SetErodeSeedY(UInt32 ErodeSeedY);
        [PreserveSig]
        UInt32 GetErodeSeedX();
        [PreserveSig]
        UInt32 GetErodeSeedY();

        #endregion

        #region Blob

        //Cross Color
        [PreserveSig]
        void SetCrossColor(UInt32 CrossColor);
        [PreserveSig]
        UInt32 GetCrossColor();

        //BlobOutlineColor
        [PreserveSig]
        void SetBlobOutlineColor(UInt32 BlobOutlineColor);
        [PreserveSig]
        UInt32 GetBlobOutlineColor();
        //BlobCrossColor
        [PreserveSig]
        void SetBlobCrossColor(UInt32 BlobCrossColor);
        [PreserveSig]
        UInt32 GetBlobCrossColor();
        //BlobTextColor
        [PreserveSig]
        void SetBlobTextColor(UInt32 BlobTextColor);
        [PreserveSig]
        UInt32 GetBlobTextColor();

        //BlobAspectLimit
        [PreserveSig]
        void SetBlobAspectLimit(UInt32 BlobAspectLimit);
        [PreserveSig]
        UInt32 GetBlobAspectLimit();
        //BlobAreaDivisor
        [PreserveSig]
        void SetBlobAreaDivisor(UInt32 BlobAreaDivisor);
        [PreserveSig]
        UInt32 GetBlobAreaDivisor();

        //ShowBlobOutlines
        [PreserveSig]
        void SetShowBlobOutlines(bool ShowBlobOutlines);
        [PreserveSig]
        bool GetShowBlobOutlines();
        //ShowBlobCenters
        [PreserveSig]
        void SetShowBlobCenters(bool ShowBlobCenters);
        [PreserveSig]
        bool GetShowBlobCenters();
        //ShowBlobValues
        [PreserveSig]
        void SetShowBlobValues(bool ShowBlobValues);
        [PreserveSig]
        bool GetShowBlobValues();

        //Font for Blob Values
        [PreserveSig]
        void SetBlobFont(IntPtr pLogFontStruct);
        [PreserveSig]
        void GetBlobFont(IntPtr pLogFontStruct, out UInt32 LogFontSize);

        #endregion

        #region Measurement

        //Measure in Pixels
        [PreserveSig]
        void SetMeasurePixels(bool MeasurePixels);
        [PreserveSig]
        bool GetMeasurePixels();

        //Inverse Image for White
        [PreserveSig]
        void SetInverse(bool Inverse);
        [PreserveSig]
        bool GetInverse();

        //MeasureMode
        [PreserveSig]
        void SetMeasureMode(UInt32 MeasureMode);
        [PreserveSig]
        UInt32 GetMeasureMode();

        //DisplayMode
        [PreserveSig]
        void SetDisplayMode(UInt32 DisplayMode);
        [PreserveSig]
        UInt32 GetDisplayMode();

        //Minimum number of StartLines
        [PreserveSig]
        void SetMinNumStartLines(UInt32 MinNumStartLines);
        //Distance between vertical StartLines
        [PreserveSig]
        void SetStartLinesDistance(float StartLinesDistance);
        //Timeout for StartLines
        [PreserveSig]
        void SetStartLinesTimeout (UInt32 StartLinesTimeout);

        //Execute Measures
        [PreserveSig]
        bool DoMeasures(UInt32 NumMeasures, UInt32 TO_1st, UInt32 TO_End);

        //Read Correction List
        [PreserveSig]
        bool GetMeasureResults(IntPtr pMeasureDataStructArray, out UInt32 ListSize);

        #endregion

        #region Line-Direction

        //Line Direction
        [PreserveSig]
        void SetLinesHorizontal(bool Horizontal);
        [PreserveSig]
        bool GetLinesHorizontal();

        //Camera Direction
        [PreserveSig]
        void SetUpsideDown(bool UpsideDown);
        [PreserveSig]
        bool GetUpsideDown();

        #endregion

        #region OCR

        //Number of OCR Characters
        [PreserveSig]
        void SetNumOcrChars(UInt32 NumOcrChars);

        //Learn OCR Characters
        [PreserveSig]
        void SetLearnOcr(UInt32 LearnCommand, char OcrChar);

        #endregion

    }

    #endregion
}
