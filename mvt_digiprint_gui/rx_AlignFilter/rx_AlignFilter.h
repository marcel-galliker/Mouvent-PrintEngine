#pragma once

using namespace std;

#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#include <CL/cl.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/ml.hpp>
using namespace cv::ml;
using namespace cv;

#ifndef UNICODE
#define UNICODE
#endif

#ifndef _UNICODE
#define _UNICODE
#endif

#ifndef __rx_AlignFilter__
#define __rx_AlignFilter__

#pragma region IDs der Filter

// {148BC1EB-2C83-418E-B9CD-E1F5BC9D1E38}
DEFINE_GUID(CLSID_rx_AlignFilter,
	0x148bc1eb, 0x2c83, 0x418e, 0xb9, 0xcd, 0xe1, 0xf5, 0xbc, 0x9d, 0x1e, 0x38);

// {C915723A-FE83-4914-AAFA-C7C486A41AAC}
DEFINE_GUID(IID_IFrx_AlignFilter,
	0xc915723a, 0xfe83, 0x4914, 0xaa, 0xfa, 0xc7, 0xc4, 0x86, 0xa4, 0x1a, 0xac);

#pragma endregion


//Host Interface
interface IFrx_AlignFilter : public IUnknown
{

public:

	#pragma region Enums & Structs

	enum BinarizeModeEnum
	{
		BinarizeMode_Off = 0,
		BinarizeMode_Manual = 1,
		BinarizeMode_Auto = 2,
		BinarizeMode_ColorAdaptive = 3,
		BinarizeMode_RGB = 4
	};

	enum MeasureModeEnum	//0: Off, 1: All Lines, 2: StartLines, 3:Angle, 4: Stitch, 5: Register
	{
		MeasureMode_Off = 0,
		MeasureMode_AllLines = 1,
		MeasureMode_StartLines = 2,
		MeasureMode_Angle = 3,
		MeasureMode_Stitch = 4,
		MeasureMode_Register = 5,
		MeasureMode_StartLinesCont = 6,
		MeasureMode_ColorStitch = 7,
		MeasureMode_OCR = 8,
	};

	enum DisplayModeEnum
	{
		Display_Off = 0,
		Display_AllLines = 1,
		Display_Correction = 2,
		Display_OCR = 3,
	};

	enum LineLayoutEnum
	{
		LineLayout_Undefined = 0,
		LineLayout_Covering = 1,
		LineLayout_FromTop = 2,
		LineLayout_FromBot = 3,
		LineLayout_FromLeft = 4,
		LineLayout_FromRight = 5
	};

	enum LineAttachEnum
    {
        LineAttach_Undefined = 0,
        LineAttach_None = 1,
        LineAttach_Top = 2,
        LineAttach_Bot = 3,
        LineAttach_Left = 4,
        LineAttach_Right = 5
    };

	struct MeasureDataStruct
	{
		int ErrorCode;
		float DPosX;			//Center of pattern offset X to center of camera
		float DPosY;			//Center of pattern offset Y to center of camera
		float Value_1;			//Angle, Stitch, Register: Correction Value in Rev or μm (Register), StartLines: number of lines
		LineLayoutEnum LineLayout;	//Angle, Stitch, Register: 0, StartLines: Lines layout (Top/Right/Covering/Bottom/Left)
        LineAttachEnum LineAttach; //Angle, Stitch, Register: 0, StartLines: Line attached to camera limits (Top/Right/None/Bottom/Left)
		BOOL micron;				//Measure is in μm
        int NumMeasures;			//Number of successful measures
		char OcrResult[4];			//OCR result in ascii or empty
        char Info[256];
	};

	enum OcrLearnCmdEnum
	{
		ReadImage = 0,
		Next = 1,
		Edit = 2,
		SaveAs = 3,
		Quit = 4,
		DeleteLearnedOCR = 5,
		ReloadOCR = 6,
	};

	#pragma endregion

	#pragma region General

	//Version Info
	STDMETHOD(GetVersion)(THIS_ wchar_t* VersionInfo, INT32* VersionInfoSize) PURE;
	//C++ Version Info
	STDMETHOD_(UINT32, GetCppVersion)(THIS) PURE;
	//WindowHandle of Host
	STDMETHOD(SetHostPointer)(THIS_ HWND HostHwnd) PURE;
	//Display Device Name
	STDMETHOD(GetDeviceName)(THIS_ LPCWSTR DeviceName) PURE;
	//Debug On
	STDMETHOD(SetDebug)(THIS_ BOOL DebugOn) PURE;
	//Callback debug
	STDMETHOD(SetCallbackDebug)(THIS_ BOOL CallbackDebug) PURE;
	//Debug Log To FIle
	STDMETHOD(SetDebugLogToFile)(THIS_ BOOL DebugToFile) PURE;
	//Display Frame Timing
	STDMETHOD(SetFrameTiming)(THIS_ BOOL DspFrameTime) PURE;
	//Take SnapShot and Save to
	STDMETHOD_(BOOL, TakeSnapShot)(THIS_ const wchar_t* SnapDirectory, const wchar_t* SnapFileName) PURE;

	//ShowOriginalImage
	STDMETHOD(SetShowOriginalImage)(THIS_ BOOL ShowOriginalImage) PURE;
	STDMETHOD_(BOOL, GetShowOriginalImage)(THIS) PURE;

	//Overlay-Text
	STDMETHOD_(BOOL, SetOverlayTxt)(THIS_ const wchar_t* OverlayTxt) PURE;

	//BlobTextColor
	STDMETHOD(SetOverlayTextColor)(THIS_ COLORREF BlobTextColor) PURE;
	STDMETHOD_(COLORREF, GetOverlayTextColor)(THIS) PURE;

	//Font for Blob Values
	STDMETHOD(SetOverlayFont)(THIS_ void* pLogFontStruct) PURE;
	STDMETHOD(GetOverlayFont)(THIS_ void* pLogFontStruct, UINT32* LogFontSize) PURE;

	#pragma endregion

	#pragma region Binarization

	//Binarization
	STDMETHOD(SetBinarizeMode)(THIS_ UINT BinarizeMode) PURE;			//0: off, 1: manual, 2: Auto, 3:Adaptive
	STDMETHOD_(UINT, GetBinarizeMode)(THIS) PURE;

	//Threshold
	STDMETHOD(SetThreshold)(THIS_ UINT Threshold) PURE;		// Switch Threshold
	STDMETHOD_(UINT, GetThreshold)(THIS) PURE;		// Get Threshold

	//Show Histogram
	STDMETHOD(ShowHistogram)(THIS_ BOOL ShowHistogram) PURE;			// Switch ShowHistogram On/Off

	#pragma endregion

	#pragma region Dilate-Erode

	//Num Dilate-Erodes
	STDMETHOD(SetNumDilateErodes)(THIS_ UINT DilateErodes) PURE;
	STDMETHOD_(UINT, GetNumDilateErodes)(THIS) PURE;

	//Num Extra-Erodes
	STDMETHOD(SetNumExtraErodes)(THIS_ UINT ExtraErodes) PURE;
	STDMETHOD_(UINT, GetNumExtraErodes)(THIS) PURE;

	//Erode Seed
	STDMETHOD(SetErodeSeedX)(THIS_ UINT ErodeSeedX) PURE;
	STDMETHOD(SetErodeSeedY)(THIS_ UINT ErodeSeedY) PURE;
	STDMETHOD_(UINT, GetErodeSeedX)(THIS) PURE;
	STDMETHOD_(UINT, GetErodeSeedY)(THIS) PURE;

	#pragma endregion

	#pragma region Blob

	//Cross Color
	STDMETHOD(SetCrossColor)(THIS_ COLORREF CrossColor) PURE;
	STDMETHOD_(COLORREF, GetCrossColor)(THIS) PURE;

	//BlobOutlineColor
	STDMETHOD(SetBlobOutlineColor)(THIS_ COLORREF BlobOutlineColor) PURE;
	STDMETHOD_(COLORREF, GetBlobOutlineColor)(THIS) PURE;
	//BlobCrossColor
	STDMETHOD(SetBlobCrossColor)(THIS_ COLORREF BlobCrossColor) PURE;
	STDMETHOD_(COLORREF, GetBlobCrossColor)(THIS) PURE;
	//BlobTextColor
	STDMETHOD(SetBlobTextColor)(THIS_ COLORREF BlobTextColor) PURE;
	STDMETHOD_(COLORREF, GetBlobTextColor)(THIS) PURE;

	//BlobAspectLimit
	STDMETHOD(SetBlobAspectLimit)(THIS_ UINT32 BlobAspectLimit) PURE;
	STDMETHOD_(UINT32, GetBlobAspectLimit)(THIS) PURE;
	//BlobAreaDivisor
	STDMETHOD(SetBlobAreaDivisor)(THIS_ UINT32 BlobAreaDivisor) PURE;
	STDMETHOD_(UINT32, GetBlobAreaDivisor)(THIS) PURE;

	//ShowBlobOutlines
	STDMETHOD(SetShowBlobOutlines)(THIS_ BOOL ShowBlobOutlines) PURE;
	STDMETHOD_(BOOL, GetShowBlobOutlines)(THIS) PURE;
	//ShowBlobCenters
	STDMETHOD(SetShowBlobCenters)(THIS_ BOOL ShowBlobCenters) PURE;
	STDMETHOD_(BOOL, GetShowBlobCenters)(THIS) PURE;
	//ShowBlobValues
	STDMETHOD(SetShowBlobValues)(THIS_ BOOL ShowBlobValues) PURE;
	STDMETHOD_(BOOL, GetShowBlobValues)(THIS) PURE;

	//Font for Blob Values
	STDMETHOD(SetBlobFont)(THIS_ void* pLogFontStruct) PURE;
	STDMETHOD(GetBlobFont)(THIS_ void* pLogFontStruct, UINT32* LogFontSize) PURE;

	#pragma endregion

	#pragma region Measurement

	//Measure in Pixels
	STDMETHOD(SetMeasurePixels)(THIS_ BOOL MeasurePixels) PURE;
	STDMETHOD_(BOOL, GetMeasurePixels)(THIS) PURE;

	//Inverse Image for White
	STDMETHOD(SetInverse)(THIS_ BOOL Inverse) PURE;
	STDMETHOD_(BOOL, GetInverse)(THIS) PURE;

	//MeasureMode
	STDMETHOD(SetMeasureMode)(THIS_ MeasureModeEnum MeasureMode) PURE;
	STDMETHOD_(MeasureModeEnum, GetMeasureMode)(THIS) PURE;

	//DisplayMode
	STDMETHOD(SetDisplayMode)(THIS_ DisplayModeEnum DisplayMode) PURE;
	STDMETHOD_(DisplayModeEnum, GetDisplayMode)(THIS) PURE;

	//Minimum number of StartLines
    STDMETHOD(SetMinNumStartLines)(THIS_ UINT32 MinNumStartLines) PURE;
	//Distance between vertical StartLines
	STDMETHOD(SetStartLinesDistance)(THIS_ float StartLinesDistance) PURE;
	//Timeout for StartLines
	STDMETHOD(SetStartLinesTimeout)(THIS_ UINT32 StartLinesTimeout) PURE;

	//Mid/Outer ratio for Register
	STDMETHOD(SetRegisterMidOuterRatio)(THIS_ float MidOuterRatio) PURE;

	//Execute Measures
	STDMETHOD_(BOOL, DoMeasures)(THIS_ UINT32 NumMeasures, UINT32 TO_1st, UINT32 TO_End) PURE;

	//GetMeasure Results
	STDMETHOD_(BOOL, GetMeasureResults)(THIS_ void* pMeasureDataStructArray, UINT32* ListSize) PURE;

	#pragma endregion

	#pragma region Line-Direction

	//Line Direction
	STDMETHOD(SetLinesHorizontal)(THIS_ BOOL Horizontal) PURE;
	STDMETHOD_(BOOL, GetLinesHorizontal)(THIS) PURE;

	//Line Direction
	STDMETHOD(SetUpsideDown)(THIS_ BOOL UpsideDown) PURE;
	STDMETHOD_(BOOL, GetUpsideDown)(THIS)PURE;

#pragma endregion

	#pragma region OCR

	//Number of OCR Characters
	STDMETHOD(SetNumOcrChars)(THIS_ UINT32 NumChars) PURE;

	//Learn OCR Characters
	STDMETHOD(SetLearnOcr)(THIS_ OcrLearnCmdEnum LearnCommand, char OcrChar) PURE;

	#pragma endregion


};

//Static Reference
static HMODULE hMySelf;

//Class Prototypes
class C_rx_AlignFilter;
class C_rx_AlignFilter_OutputPin;
class C_rx_AlignFilter_InputPin;

struct QualifyStruct
{
public:
	UINT16 BlobNo;
	UINT16 Valid;	//0: Invalid, 1: Valid Line, 2: Valid OCR, 3: Valid Line and OCR
	float PosX;
	float PosY;
	float Angle;
	float Height;
	float Width;
	float DistToNext;
	float DistToLast;
	UINT16 LastUsed;
	UINT16 NextUsed;
	UINT16 Display;
	float AngleCorrection;
	float StitchCorr;
	float RegisterCorr;
	UINT16 OcrResult;
	UINT16 Deviated = 0;		//0: OK, 1: to left, 2: to right, 3: NotSpecified from ReEval, 4: missing, 5: too thin, 6: too thick, 7: ToNext > 2 Sigma off
	float Deviation = 0;
	float DevResolution = 0;
	UINT16 NextMissing = 0;
	UINT16 LastMissing = 0;
};

struct OcrResultStruct
{
	UINT16 BlobNo;
	UINT16 OcrResult;
	int XPos;
	int YPos;
};


//Input pin
class C_rx_AlignFilter_InputPin : public CBaseInputPin
{
	friend class C_rx_AlignFilter_OutputPin;
	C_rx_AlignFilter *m_prx_AlignFilter;		// Main filter object

public:

	// Constructor and destructor
	C_rx_AlignFilter_InputPin(TCHAR *pObjName,	//Pin Name
		C_rx_AlignFilter *p_C_rx_AlignFilter,	//Filter
		HRESULT *phr,		//H-Result
		LPCWSTR pPinName);	//Pin Name

	LPCWSTR PinName;
	BOOL m_bInsideCheckMediaType;  // Re-entrancy control

	// Used to check the input pin connection
	HRESULT CheckMediaType(const CMediaType *pmt);
	HRESULT SetMediaType(const CMediaType *pmt);
	HRESULT BreakConnect();

	// Reconnect outputs if necessary at end of completion
	virtual HRESULT CompleteConnect(IPin *pReceivePin);

	STDMETHODIMP NotifyAllocator(IMemAllocator *pAllocator, BOOL bReadOnly);

	// Pass through calls downstream
	STDMETHODIMP EndOfStream();
	STDMETHODIMP BeginFlush();
	STDMETHODIMP EndFlush();
	STDMETHODIMP NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate);

	// Handles the next block of data from the stream
	STDMETHODIMP Receive(IMediaSample *pSample);
};

//Output pin
class C_rx_AlignFilter_OutputPin : public CBaseOutputPin
{
	friend class C_rx_AlignFilter_InputPin;
	friend class C_rx_AlignFilter;

	C_rx_AlignFilter *m_prx_AlignFilter;		// Main filter object pointer
	IUnknown    *m_pPosition;						// Pass seek calls upstream
	BOOL m_bHoldsSeek;								// Is this the one seekable stream

	COutputQueue *m_pOutputQueue;					// Streams data to the peer pin
	BOOL m_bInsideCheckMediaType;					// Re-entrancy control
	LONG m_cOurRef;									// We maintain reference counting

public:

	// Constructor and destructor

	C_rx_AlignFilter_OutputPin(TCHAR *pObjName,
		C_rx_AlignFilter *pdsFilterTime,
		HRESULT *phr,
		LPCWSTR pPinName);

	LPCWSTR PinName;

	// Override to enumerate media types
	STDMETHODIMP EnumMediaTypes(IEnumMediaTypes **ppEnum);

	// Check that we can support an output type
	HRESULT CheckMediaType(const CMediaType *pmt);
	HRESULT SetMediaType(const CMediaType *pmt);

	// Negotiation to use our input pins allocator
	HRESULT DecideAllocator(IMemInputPin *pPin, IMemAllocator **ppAlloc);
	HRESULT DecideBufferSize(IMemAllocator *pMemAllocator, ALLOCATOR_PROPERTIES * ppropInputRequest);

	// Used to create output queue objects
	HRESULT Active();
	HRESULT Inactive();

	// Overriden to create and destroy output pins
	HRESULT CompleteConnect(IPin *pReceivePin);

	// Overriden to pass data to the output queues
	HRESULT Deliver(IMediaSample *pMediaSample);
	HRESULT DeliverEndOfStream();
	HRESULT DeliverBeginFlush();
	HRESULT DeliverEndFlush();
	HRESULT DeliverNewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate);

};

//rx_AlignFilter filter
class C_rx_AlignFilter
	: public CCritSec, public CBaseFilter, public IFrx_AlignFilter, public CPersistStream
{
	// Let the pins access our internal state
	friend class C_rx_AlignFilter_InputPin;
	friend class C_rx_AlignFilter_OutputPin;

	// Declare 1 Input Pins
	C_rx_AlignFilter_InputPin m_Input;

	// Declare 1 Output Pins
	C_rx_AlignFilter_OutputPin m_Output;

	IMemAllocator *m_pAllocator;    // Allocator from input pin

public:

	DECLARE_IUNKNOWN;

	//Constructor
	C_rx_AlignFilter(TCHAR *pName, LPUNKNOWN pUnk, HRESULT *hr);
	//Destructor
	~C_rx_AlignFilter();

	CBasePin *GetPin(int n);
	int GetPinCount();

	CRITICAL_SECTION m_MeasureLock;
	CRITICAL_SECTION m_OvTextLock;
	CRITICAL_SECTION m_SnapShotLock;
	CRITICAL_SECTION m_MeasureStartLock;

	// Function needed for the class factory
	static CUnknown * WINAPI CreateInstance(LPUNKNOWN pUnk, HRESULT *phr);

	// Send EndOfStream if no input connection
	STDMETHODIMP Run(REFERENCE_TIME tStart);
	STDMETHODIMP Pause();
	STDMETHODIMP Stop();

	// Reveals Filter and ISpecifyPropertyPages
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);
	// CPersistStream override
	STDMETHODIMP GetClassID(CLSID *pClsid);



	#pragma region Host Interface

	#pragma region General

	STDMETHODIMP GetVersion(wchar_t* VersionInfo, INT32* VersionInfoSize);
	STDMETHODIMP_(UINT32) GetCppVersion();
	STDMETHODIMP SetHostPointer(HWND PropHwnd);
	STDMETHODIMP GetDeviceName(LPCWSTR DeviceName);
	STDMETHODIMP SetDebug(BOOL DebugOn);
	STDMETHODIMP SetCallbackDebug(BOOL CallbackDebug);
	STDMETHODIMP SetDebugLogToFile(BOOL DebugToFile);
	STDMETHODIMP SetFrameTiming(BOOL DspFrameTime);
	STDMETHODIMP_(BOOL) TakeSnapShot(const wchar_t* SnapDirectory, const wchar_t* SnapFileName);
	STDMETHODIMP SetShowOriginalImage(BOOL ShowOriginalImage);
	STDMETHODIMP_(BOOL) GetShowOriginalImage();
	STDMETHODIMP_(BOOL) SetOverlayTxt(const wchar_t* OverlayTxt);
	STDMETHODIMP SetOverlayTextColor(COLORREF BlobTextColor);
	STDMETHODIMP_(COLORREF) GetOverlayTextColor();
	STDMETHODIMP SetOverlayFont(void* pLogFontStruct);
	STDMETHODIMP GetOverlayFont(void* pLogFontStruct, UINT32* LogFontSize);

	#pragma endregion

	#pragma region Binarize

	STDMETHODIMP SetBinarizeMode(UINT BinarizeMode);
	STDMETHODIMP_(UINT) GetBinarizeMode();

	STDMETHODIMP SetThreshold(UINT Threshold);
	STDMETHODIMP_(UINT) GetThreshold();

	STDMETHODIMP ShowHistogram(BOOL ShowHistogram);

	#pragma endregion

	#pragma region Dilate-Erode

	//Num Dilate-Erodes
	STDMETHODIMP SetNumDilateErodes(UINT DilateErodes);
	STDMETHODIMP_(UINT) GetNumDilateErodes();
	//Num Extra-Erodes
	STDMETHODIMP SetNumExtraErodes(UINT ExtraErodes);
	STDMETHODIMP_(UINT) GetNumExtraErodes();
	//Erode Seed
	STDMETHODIMP SetErodeSeedX(UINT ErodeSeedX);
	STDMETHODIMP SetErodeSeedY(UINT ErodeSeedY);
	STDMETHODIMP_(UINT) GetErodeSeedX();
	STDMETHODIMP_(UINT) GetErodeSeedY();

	#pragma endregion

	#pragma region Blob

	STDMETHODIMP SetCrossColor(COLORREF CrossColor);
	STDMETHODIMP_(COLORREF) GetCrossColor();

	//BlobOutlineColor
	STDMETHODIMP SetBlobOutlineColor(COLORREF BlobOutlineColor);
	STDMETHODIMP_(COLORREF) GetBlobOutlineColor();
	//BlobCrossColor
	STDMETHODIMP SetBlobCrossColor(COLORREF BlobCrossColor);
	STDMETHODIMP_(COLORREF) GetBlobCrossColor();
	//BlobTextColor
	STDMETHODIMP SetBlobTextColor(COLORREF BlobTextColor);
	STDMETHODIMP_(COLORREF) GetBlobTextColor();

	//BlobAspectLimit
	STDMETHODIMP SetBlobAspectLimit(UINT32 BlobAspectLimit);
	STDMETHODIMP_(UINT32) GetBlobAspectLimit();
	//BlobAreaDivisor
	STDMETHODIMP SetBlobAreaDivisor(UINT32 BlobAreaDivisor);
	STDMETHODIMP_(UINT32) GetBlobAreaDivisor();

	//ShowBlobOutlines
	STDMETHODIMP SetShowBlobOutlines(BOOL ShowBlobOutlines);
	STDMETHODIMP_(BOOL) GetShowBlobOutlines();
	//ShowBlobCenters
	STDMETHODIMP SetShowBlobCenters(BOOL ShowBlobCenters);
	STDMETHODIMP_(BOOL) GetShowBlobCenters();
	//ShowBlobValues
	STDMETHODIMP SetShowBlobValues(BOOL ShowBlobValues);
	STDMETHODIMP_(BOOL) GetShowBlobValues();
	//Font for Blob Values
	STDMETHODIMP SetBlobFont(void* pLogFontStruct);
	STDMETHODIMP GetBlobFont(void* pLogFontStruct, UINT32* LogFontSize);

	#pragma endregion

	#pragma region Measurement

	//Measure in Pixels
	STDMETHODIMP SetMeasurePixels(BOOL MeasurePixels);
	STDMETHODIMP_(BOOL) GetMeasurePixels();

	//Inverse Image for White
	STDMETHODIMP SetInverse(BOOL Inverse);
	STDMETHODIMP_(BOOL) GetInverse();

	//MeasureMode
	STDMETHODIMP SetMeasureMode(MeasureModeEnum MeasureMode);
	STDMETHODIMP_(MeasureModeEnum) GetMeasureMode();

	//DisplayMode
	STDMETHODIMP SetDisplayMode(DisplayModeEnum DisplayMode);
	STDMETHODIMP_(DisplayModeEnum) GetDisplayMode();

	//Minimum number of StartLines
    STDMETHODIMP SetMinNumStartLines(UINT32 MinNumStartLines);
	//Distance between vertical StartLines
	STDMETHODIMP SetStartLinesDistance(float StartLinesDistance);
	//Timeout for StartLines
	STDMETHODIMP SetStartLinesTimeout (UINT32 StartLinesTimeout);

	//Mid/Outer ratio for Register
	STDMETHODIMP SetRegisterMidOuterRatio(float MidOuterRatio);

	//Execute Measures
	STDMETHODIMP_(BOOL) DoMeasures(UINT32 NumMeasures, UINT32 TO_1st, UINT32 TO_End);

	//GetMeasure Results
	STDMETHODIMP_(BOOL) GetMeasureResults(void* pMeasureDataStructArray, UINT32* ListSize);

#pragma endregion

	#pragma region Line Direction

	//Line Direction
	STDMETHODIMP SetLinesHorizontal(BOOL LinesHorizontal);
	STDMETHODIMP_(BOOL) GetLinesHorizontal();

	//Line Direction
	STDMETHODIMP SetUpsideDown(BOOL UpsideDown);
	STDMETHODIMP_(BOOL) GetUpsideDown();

	#pragma endregion

	#pragma region OCR

	//Number of OCR Characters
	STDMETHODIMP SetNumOcrChars(UINT32 NumOcrChars);

	//Learn OCR Characters
	STDMETHODIMP SetLearnOcr(OcrLearnCmdEnum LearnCommand, char OcrChar);

	#pragma endregion

	#pragma endregion

private:
	
	wchar_t AppLocalPath[MAX_PATH];
	wchar_t AppTempPath[MAX_PATH];
	wchar_t DebugLogPath[MAX_PATH];

	//Connection
	HRESULT FrameFinished(void);
	CMediaType m_mt1;

	//Video Size
	UINT m_ImageWidth = 0;
	UINT m_ImageHeight = 0;

	//Host Communication
	HWND m_PresetHostHwnd = NULL;
	HWND m_HostHwnd = NULL;

	//OpenCl
	cl_context context = NULL;
	cl_command_queue queue = NULL;
	cl_program Program = NULL;

	//Binarize
	BinarizeModeEnum m_BinarizeMode = BinarizeMode_Off;	//0: off, 1: manual, 2: Auto, 3: Adaptive, 4: RGB
	BinarizeModeEnum m_PresetBinarizeMode = BinarizeMode_Off;
	UINT Histogram[256];
	UINT m_Threshold = 127;
	UINT m_PresetThreshold = 127;
	BOOL m_ShowHistogram = false;
	BOOL m_PresetShowHistogram = false;
	LPCWSTR m_DeviceName = L"none";
	UINT m_Peak_1_Val = 0;
	UINT m_Peak_2_Val = 0;
	UINT m_Peak_1_Pos = 0;
	UINT m_Peak_2_Pos = 0;
	UINT ImageCounter = 0;
	UINT HistoRepeat = 2;

	//Multi Color Histogram
	uchar* ThresholdArray = NULL;
	UINT* HistogramArray = NULL;
	uchar* ThresholdLine = NULL;

	//RGB Histogram
	UINT RGBHistogram[3 * 256];
	UINT m_RGBThreshold[3];
	UINT m_RGBPeak_1_Val[3] = { 0, 0, 0 };
	UINT m_RGBPeak_2_Val[3] = { 0, 0, 0 };
	UINT m_RGBPeak_1_Pos[3] = { 0, 0, 0 };
	UINT m_RGBPeak_2_Pos[3] = { 0, 0, 0 };

	//Dilate/Erode
	UINT m_DilateErodes = 3;			//Dilate-Erode Iterations
	UINT m_PresetDilateErodes = 3;
	UINT m_ExtraErodes = 2;				//Additional Erodes
	UINT m_PresetExtraErodes = 2;
	UINT m_ErodeSeedX = 13;				//Erode Seed size
	UINT m_PresetErodeSeedX = 13;
	UINT m_ErodeSeedY = 1;
	UINT m_PresetErodeSeedY = 1;

	Mat m_ElementE_Hori;
	Mat m_ElementE_Verti;
	Mat m_ElementD_Hori;
	Mat m_ElementD_Verti;
	Mat DilShape;
	Mat EroShape;

	//Blob
	vector<vector<Point>> m_vContours;
	vector<Vec4i> m_vHierarchy;
	vector<RotatedRect> m_vAngularRect;
	vector<Point2f> m_vCenters;
	vector<Moments> m_vMoments;
	vector<Rect> m_vBoundRect;
	COLORREF m_CrossColorBlob = RGB(255, 255, 255);
	COLORREF m_PresetCrossColorBlob = RGB(255, 255, 255);

	//Measurement
	BOOL m_ShowOriginal = true;
	BOOL m_PresetShowOriginal = true;
	BOOL m_OverlayBlobs = true;
	BOOL m_PresetOverlayBlobs = true;
	BOOL m_OverlayCenters = true;
	BOOL m_PresetOverlayCenters = true;
	BOOL m_OverlayValues = true;
	BOOL m_PresetOverlayValues = true;
	BOOL m_MeasurePixels = true;
	UINT m_BlobAreaDivisor = 70;
	UINT m_PresetBlobAreaDivisor = 70;
	UINT m_BlobAspectLimit = 5;
	UINT m_PresetBlobAspectLimit = 5;
	COLORREF m_BlobColor = RGB(192, 192, 192);
	COLORREF m_PresetBlobColor = RGB(192, 192, 192);
	COLORREF m_CrossColor = RGB(255, 255, 255);
	COLORREF m_PresetCrossColor = RGB(255, 255, 255);

	COLORREF m_BlobTextColor = RGB(0, 255, 0);
	COLORREF m_PresetBlobTextColor = RGB(0, 255, 0);
	HFONT m_BlobTextFont = NULL;
	long m_BlobFontHeight = 0;

	MeasureModeEnum m_MeasureMode = IFrx_AlignFilter::MeasureModeEnum::MeasureMode_Off;	//0: Off, 1: All Lines, 2: StartLines, 3:Angle, 4: Stitch, 5: Register, 6: StartLinesCont
	MeasureModeEnum m_PresetMeasureMode = IFrx_AlignFilter::MeasureModeEnum::MeasureMode_Off;
	BOOL m_ValidMeasure = false;
	vector<QualifyStruct> m_vQualifyList;

	BOOL m_StartMeasure = false;
	BOOL m_MeasureRunning = false;
	BOOL m_NumMeasures = 0;
	MeasureDataStruct CorrectionForHost;
	vector<MeasureDataStruct> m_vMeasureDataList;
	UINT32 m_DataListSize = 0;
	BOOL m_DataListforHostReady = false;
	BOOL m_measureDone = false;
	UINT32 m_Timeout1st = 0;
	UINT32 m_TimeoutEnd = 0;
	UINT32 m_TimeoutCounter = 0;

	float m_FindLine_umPpx = 1;

	const int m_MaxBlobAngle = 20;
	const int m_MaxNumBlobs = 50;
	DisplayModeEnum m_DisplayMode = IFrx_AlignFilter::DisplayModeEnum::Display_Off;	//0: Off, 1: All Lines, 2: Correction Values
	DisplayModeEnum m_PresetDisplayMode = IFrx_AlignFilter::DisplayModeEnum::Display_Off;
	UINT m_MinNumStartLines = 3;
	UINT m_PresetMinNumStartLines = 3;
	#define WM_APP_ALIGNEV WM_APP + 2025
	#define WP_StartLines 100
	#define WP_Angle 101
	#define WP_Stitch 102
	#define WP_Register 103
	#define WP_StartLinesCont 104
	#define WP_MeasureTimeout 105
	#define WP_StartLinesTimeout 106
	#define WP_CallBackDebug 107
	#define WP_ReadOCR 108
	#define WP_ColorStitch 109

	//Line Direction
	BOOL m_LinesHorizontal = false;
	BOOL m_PresetLinesHorizontal = false;
	BOOL m_UpsideDown = false;
	BOOL m_PresetUpsideDown = false;
	BOOL m_InverseImage = false;
	BOOL m_PresetInverseImage = false;

	//Debug On
	BOOL m_DebugPrepare = false;
	BOOL m_DebugOn = false;
	BOOL m_ConsoleAllocated = false;
	UINT m_DebugCounter = 0;
	FILE* pStdIn;
	FILE* pStdOut;
	FILE* pStdErr;
	BOOL m_CallbackDebug = false;
	BOOL m_LogToFile = false;


	//Pattern parameters
	//Angle
	double m_AngleSideA = 541.00739366481859570264341545511;
	double m_AngleSideB = 64.031242374328486864882176746218;
	double m_AngleSideC = 542.88580751388224147838611840257;
	double m_AngleAlpha = 1.4824231678730821214311580232518;
	double m_AngleBeta1 = 0.05713332549154622154645192349483;
	double m_AngleBeta2 = 0.06103520658968898926969889927577;
	double m_AngleDegreesPerRev = -0.000970835240444;
	double m_AngleOuterDistance = 1354.6666666666667;
	//Stitch
	float m_StitchOuterDistance = 1143;
	float m_StitchTargetDistance = 571.5;
	float m_StitchMicronsPerRev = 11.347f;
	//Register
	float m_RegisterOuterDistance = 1143;
	float m_RegisterTargetDistance = 571.5;
	float m_RegisterMidOuterRatio = 1.5f;
	//FindLines
	float m_FindLine_Distance = (float)677.333333333333;
	float m_PresetFindLine_Distance = (float)677.333333333333;
	UINT32 m_StartLinesTimeout = 0;
	UINT32 m_StartLinesTimeoutCounter = 0;

	//TextBitmap
	BOOL OverlayBitmapReady = FALSE;
	HBITMAP TextBitmap = NULL;
	BYTE* pText = NULL;
	HDC MemDC = NULL;

	//SnapShot
	wchar_t* m_SnapDir[MAX_PATH];
	wchar_t m_SavePathName[MAX_PATH] = L"";
	BOOL m_TakeSnapShot = false;

	//Timing Display
	BOOL m_DspTiming = false;
	BOOL m_PresetDspTiming = false;
	UINT NumFrames = 0;
	std::chrono::steady_clock::time_point m_FrameStartTime;
	std::chrono::steady_clock::time_point m_FrameEndTime;
	LONGLONG m_FrameRepeat = 0;
	LONGLONG m_FrameTime = 0;;
	LONGLONG m_AvgFrameTime = 0;

	std::chrono::steady_clock::time_point m_DebugStartTime;
	std::chrono::steady_clock::time_point m_MeasureTime;
	LONGLONG m_TimeStamp = 0;

	//Overlay
	wchar_t m_OverlayTxt[MAX_PATH] = L"";
	BOOL m_OverlayTxtReady = false;
	COLORREF m_OverlayTextColor = RGB(0, 255, 0);
	COLORREF m_PresetOverlayTextColor = RGB(0, 255, 0);
	HFONT m_OverlayTextFont = NULL;
	long m_OverlayFontHeight = 0;

	//OCR
	vector<OcrResultStruct> OcrList;	//Will hold {BlobNo, OcrValue, X- and Y-Position}
	BOOL m_OcrTrained = false;
	BOOL m_OCR_ValueReady = false;
	char m_OCR_Value[256] = "";
	UINT16 m_OcrAvgX = 0;
	UINT16 m_OcrAvgY = 0;
	Ptr<KNearest> KNearOcr = NULL;
	UINT32 m_PresetNumOcrChars = 2;
	UINT32 m_NumOcrChars = 2;
	wchar_t LearnedOcrPath[MAX_PATH];
	std::string RoiBmpOcrPath;
	Mat Learn_Samples;
	Mat_<int> Learn_Responses;
	Mat FrozenImage;
	BOOL m_LearningOCR = false;
	BOOL m_LearnImageTaken = false;
	Mat OcrRoi[4];
	UINT32 m_NumOcrRois = 0;
	UINT32 m_CurrentOcrBlob = 0;
	BOOL m_EditOcrImage = false;
	BOOL m_DestroyOcrImage = false;
	cv::Point3i MousePos;
	cv::Point2i LastBase;
	BOOL ocrwin0open = false;
	UINT32 m_OcrBgCounter = 0;
	BOOL m_DeleteLearnedOcr = false;
	BOOL m_SaveOCRChar = false;
	BOOL m_ReloadOcr = false;
	char m_OcrChar = '\0';

	BOOL ocrwin1open = false;
	BOOL ocrwin2open = false;
	BOOL ocrwin3open = false;
	BOOL ocrwin4open = false;

	//Prototypes

	//OpenCl
	BOOL FindPlatformDevice(cl_platform_id* ClPlatform, int* PlatformNum, cl_device_id* ClDevice, int* DeviceNum, char* DeviceName);
	cl_context CreateContext(cl_platform_id ClPlatformId, int PlatformNum, cl_device_id ClDeviceId, int DeviceNum);
	cl_command_queue CreateCommandQueue(cl_context context, cl_device_id ClDeviceId, int DeviceNum);
	cl_program MakeProgram(int KernelSourceID, cl_context context, cl_device_id DeviceId, int DeviceNum);
	BOOL CreateKernels(cl_program Program);

	//OpenCl Kernels
	cl_kernel ClFullHistogramKernel = NULL;
	cl_kernel ClBlockHistogramKernel = NULL;
	cl_kernel ClRowHistogramKernel = NULL;
	cl_kernel ClJoinHistogramKernel = NULL;
	cl_kernel ClSmoothenHistogramKernel = NULL;
	cl_kernel ClShowHistogramKernel = NULL;
	cl_kernel ClBinarizeKernel = NULL;
	cl_kernel ClColorHistogramKernel = NULL;
	cl_kernel ClColorBinarizeKernel = NULL;
	cl_kernel ClColorThresholdLinesKernel = NULL;
	cl_kernel ClColorThresholdAverageKernel = NULL;
	cl_kernel ClShowColorHistogramKernel = NULL;
	cl_kernel ClRGBBlockHistogramKernel = NULL;
	cl_kernel ClShowRGBHistogramKernel = NULL;
	cl_kernel ClJoinRGBHistogramKernel = NULL;
	cl_kernel ClSmoothenRGBHistogramKernel = NULL;
	cl_kernel ClColorizeRGBKernel = NULL;

	//Debug
	void CallbackDebug(const char* format, ...);
	void DebugLog(const char* MessageFormat, ...);
	void LogToFile(char* MessageFormat, ...);
	void LogToFile(wchar_t* MessageFormat, ...);

	//Change Settings and Modes
	HRESULT ChangeModes();

	//Binarize
	HRESULT GetHistogram(IMediaSample* pSampleIn, UINT* Histogram, UINT BinarizationMode);
	HRESULT ShowHistogram(IMediaSample* pSampleIn, UINT* Histogram);
	HRESULT Binarize(IMediaSample* pSampleIn, IMediaSample* pSampleOut);
	int CalcThreshold(UINT* Histogram, UINT* Peak_1_Val, UINT * Peak_1_Pos, UINT* Peak_2_Val, UINT* Peak_2_Pos, UINT* Threshold);

	//Multi Color Histogram
	HRESULT GetColorHistogram(IMediaSample* pSampleIn, BOOL Vertical);
	HRESULT ShowColorHistogram(IMediaSample* pSampleIn, UINT* Histogram, BOOL Vertical);
	HRESULT BinarizeMultiColor(IMediaSample* pSampleIn, IMediaSample* pSampleOut);

	//RGB Histogram
	HRESULT GetRGBHistogram(IMediaSample* pSampleIn, UINT* RGBHistogram);
	HRESULT ShowRGBHistogram(IMediaSample* pSampleIn, UINT* RGBHistogram);
	HRESULT ColorizeRGB(IMediaSample* pSampleIn, UINT* RGBHistogram, IMediaSample* pSampleOut, bool FixInverse = true);
	int CalcRGBThreshold(UINT* RGBHistogram, UINT* Peak_1_Val, UINT* Peak_1_Pos, UINT* Peak_2_Val, UINT* Peak_2_Pos, UINT* Threshold);

	//Dilate/Erode
	HRESULT Erode(IMediaSample* pSampleIn, BOOL Vertical);
	void CalculateErodeSeed();

	//Blob
	HRESULT Blob(IMediaSample* pSampleIn);

	//Measurement
	HRESULT BlobQualifyer(IMediaSample* pSampleIn, BOOL Vertical, IMediaSample* pProcessSample);
	HRESULT CalculateDistances(BOOL Vertical, Mat SourceMat);
	HRESULT FindStartLines(BOOL Vertical, BOOL UpsideDown, BOOL Continuous = false);
	HRESULT MeasureAngle(BOOL Vertical, BOOL UpsideDown);
	HRESULT MeasureStitch(BOOL Vertical, BOOL UpsideDown, BOOL InRevolutions);
	HRESULT MeasureRegister(BOOL Vertical, BOOL UpsideDown, float MidOuterRatio = 1.0f);
	HRESULT ReadOcr(IMediaSample* pSampleIn, BOOL Vertical, BOOL UpsideDown, BOOL Continuous);

	//Display
	HRESULT OverlayBlobs(IMediaSample* pSampleIn);
	HRESULT OverlayCenters(IMediaSample* pSampleIn, BOOL Vertical);
	HRESULT OverlayValues(IMediaSample* pSampleIn, BOOL Vertical);
	HRESULT OverlayDistanceLines(IMediaSample* pSampleIn, BOOL Vertical);

	//SnapShot
	HRESULT TakeSnapShot(IMediaSample* pSampleIn);

	//ocr
	BOOL TrainOCR();
	HRESULT LearnOcr(IMediaSample* pSampleIn, BOOL Vertical, BOOL UpsideDown, UINT32 OcrBlobNo);

};


#endif __rx_AlignFilter__

