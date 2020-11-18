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

	enum MeasureModeEnum
	{
		MeasureMode_Off = 0,
		MeasureMode_AllLines = 1
	};

	enum DisplayModeEnum
	{
		Display_Off = 0,
		Display_AllLines = 1
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
	//Display Frame Timing
	STDMETHOD(SetFrameTiming)(THIS_ BOOL DspFrameTime) PURE;
	//Take SnapShot and Save to
	STDMETHOD_(BOOL, TakeSnapShot)(THIS_ const wchar_t* SnapDirectory, const wchar_t* SnapFileName) PURE;

	//ShowOriginalImage
	STDMETHOD(SetShowOriginalImage)(THIS_ BOOL ShowOriginalImage) PURE;
	STDMETHOD_(BOOL, GetShowOriginalImage)(THIS) PURE;

	//Overlay-Text
	STDMETHOD_(BOOL, SetOverlayTxt)(THIS_ const wchar_t* OverlayTxt, UINT32 OverlayTxtColor) PURE;

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
	STDMETHOD(SetBlobOutlineColor)(THIS_ UINT32 BlobOutlineColor) PURE;
	STDMETHOD_(UINT32, GetBlobOutlineColor)(THIS) PURE;
	//BlobCrossColor
	STDMETHOD(SetBlobCrossColor)(THIS_ UINT32 BlobCrossColor) PURE;
	STDMETHOD_(UINT32, GetBlobCrossColor)(THIS) PURE;
	//BlobTextColor
	STDMETHOD(SetBlobTextColor)(THIS_ UINT32 BlobTextColor) PURE;
	STDMETHOD_(UINT32, GetBlobTextColor)(THIS) PURE;

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

	#pragma endregion

	#pragma region Line-Direction

	//Line Direction
	STDMETHOD(SetLinesHorizontal)(THIS_ BOOL Horizontal) PURE;
	STDMETHOD_(BOOL, GetLinesHorizontal)(THIS) PURE;

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
	UINT16 Deviated = 0;		//0: OK, 1: to left, 2: to right, 3: NotSpecified from ReEval, 4: missing, 5: too thin, 6: too thick, 7: ToNext > 2 Sigma off
	float Deviation = 0;
	float DevResolution = 0;
	UINT16 NextMissing = 0;
	UINT16 LastMissing = 0;
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
	STDMETHODIMP SetFrameTiming(BOOL DspFrameTime);
	STDMETHODIMP_(BOOL) TakeSnapShot(const wchar_t* SnapDirectory, const wchar_t* SnapFileName);
	STDMETHODIMP SetShowOriginalImage(BOOL ShowOriginalImage);
	STDMETHODIMP_(BOOL) GetShowOriginalImage();
	STDMETHODIMP_(BOOL) SetOverlayTxt(const wchar_t* OverlayTxt, UINT32 OverlayTxtColor);

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
	STDMETHODIMP SetBlobOutlineColor(UINT32 BlobOutlineColor);
	STDMETHODIMP_(UINT32) GetBlobOutlineColor();
	//BlobCrossColor
	STDMETHODIMP SetBlobCrossColor(UINT32 BlobCrossColor);
	STDMETHODIMP_(UINT32) GetBlobCrossColor();
	//BlobTextColor
	STDMETHODIMP SetBlobTextColor(UINT32 BlobTextColor);
	STDMETHODIMP_(UINT32) GetBlobTextColor();

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

	#pragma endregion

	#pragma region Line Direction

	//Line Direction
	STDMETHODIMP SetLinesHorizontal(BOOL LinesHorizontal);
	STDMETHODIMP_(BOOL) GetLinesHorizontal();

	#pragma endregion

	#pragma endregion

private:
	
	//Connection
	HRESULT FrameFinished(void);
	CMediaType m_mt1;

	//Video Size
	UINT m_ImageWidth = 0;
	UINT m_ImageHeight = 0;

	//Host and Property Communication
	HWND m_HostHwnd = NULL;

	//OpenCl
	cl_context context = NULL;
	cl_command_queue queue = NULL;
	cl_program Program = NULL;

	//Binarize
	UINT m_BinarizeMode = 0;	//0: off, 1: manual, 2: Auto, 3:Adaptive
	UINT Histogram[256];
	UINT m_Threshold = 127;
	BOOL m_ShowHistogram = false;
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

	//Dilate/Erode
	UINT m_DilateErodes = 2;			//Dilate-Erode Iterations
	UINT m_ExtraErodes = 1;				//Additional Erodes
	UINT m_ErodeSeedX = 10;				//Erode Seed size		
	UINT m_ErodeSeedY = 1;

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

	//Measurement
	BOOL m_ShowOriginal = true;
	BOOL m_OverlayBlobs = true;
	BOOL m_OverlayCenters = true;
	BOOL m_OverlayValues = true;
	BOOL m_MeasurePixels = true;
	UINT m_BlobAreaDivisor = 70;
	UINT m_BlobAspectLimit = 5;
	COLORREF m_BlobColor = RGB(127, 127, 127);
	COLORREF m_CrossColor = RGB(255, 255, 255);
	COLORREF m_TextColor = RGB(0, 255, 0);
	HFONT m_TextFont = NULL;
	long m_FontHeight = 0;
	MeasureModeEnum m_MeasureMode = MeasureModeEnum::MeasureMode_Off;		//0: Off, 1: All Lines
	BOOL m_ValidMeasure = false;
	UINT m_QualifyListSize = 0;
	vector<QualifyStruct> m_vQualifyList;
	vector<QualifyStruct> m_vQualifyListToHost;
	BOOL m_PrepareQualifyListForHost = false;
	BOOL m_QualifyListForHostReady = false;
	const int m_MaxBlobAngle = 20;
	const int m_MaxNumBlobs = 50;
	DisplayModeEnum m_DisplayMode = DisplayModeEnum::Display_Off;	//0: Off, 1: All Lines

	//Line Direction
	bool m_LinesHorizontal = false;

	//Debug On
	BOOL m_DebugPrepare = false;
	BOOL m_DebugOn = false;
	BOOL m_ConsoleAllocated = false;
	UINT m_DebugCounter = 0;

	bool m_InverseImage = false;

	//TextBitmap
	bool OverlayBitmapReady = FALSE;
	HBITMAP TextBitmap = NULL;
	BYTE* pText = NULL;
	HDC MemDC = NULL;

	//SnapShot
	wchar_t* m_SnapDir[MAX_PATH];
	wchar_t m_SavePathName[MAX_PATH] = L"";
	bool m_TakeSnapShot = false;

	//Timing Display
	BOOL m_DspTiming = false;
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
	UINT32 m_OverlayTxtColor = 0x00000000;


	//Prototypes

	//OpenCl
	bool FindPlatformDevice(cl_platform_id* ClPlatform, int* PlatformNum, cl_device_id* ClDevice, int* DeviceNum, char* DeviceName);
	cl_context CreateContext(cl_platform_id ClPlatformId, int PlatformNum, cl_device_id ClDeviceId, int DeviceNum);
	cl_command_queue CreateCommandQueue(cl_context context, cl_device_id ClDeviceId, int DeviceNum);
	cl_program MakeProgram(int KernelSourceID, cl_context context, cl_device_id DeviceId, int DeviceNum);
	bool CreateKernels(cl_program Program);

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


	//Binarize
	HRESULT GetHistogram(IMediaSample* pSampleIn, UINT* Histogram, UINT BinarizationMode);
	HRESULT ShowHistogram(IMediaSample* pSampleIn, UINT* Histogram);
	HRESULT Binarize(IMediaSample* pSampleIn, IMediaSample* pSampleOut);
	int CalcThreshold(UINT* Histogram, UINT* Peak_1_Val, UINT * Peak_1_Pos, UINT* Peak_2_Val, UINT* Peak_2_Pos, UINT* Threshold);

	//Multi Color Histogram
	HRESULT GetColorHistogram(IMediaSample* pSampleIn, BOOL Vertical);
	HRESULT ShowColorHistogram(IMediaSample* pSampleIn, UINT* Histogram, BOOL Vertical);
	HRESULT C_rx_AlignFilter::BinarizeMultiColor(IMediaSample* pSampleIn, IMediaSample* pSampleOut, UINT BinarizationMode);


	//Dilate/Erode
	HRESULT Erode(IMediaSample* pSampleIn, BOOL Vertical);
	void 	CalculateErodeSeed();

	//Blob
	HRESULT Blob(IMediaSample* pSampleIn);

	//Measurement
	HRESULT BlobQualifyer(IMediaSample* pSampleIn, BOOL Vertical, IMediaSample* pProcessSample);
	HRESULT CalculateDistances(BOOL Vertical, Mat SourceMat);
	HRESULT OverlayBlobs(IMediaSample* pSampleIn);
	HRESULT OverlayCenters(IMediaSample* pSampleIn, BOOL Vertical);
	HRESULT OverlayValues(IMediaSample* pSampleIn, BOOL Vertical);
	HRESULT OverlayDistanceLines(IMediaSample* pSampleIn, BOOL Vertical);

	//SnapShot
	HRESULT TakeSnapShot(IMediaSample* pSampleIn);

};


#endif __rx_AlignFilter__

