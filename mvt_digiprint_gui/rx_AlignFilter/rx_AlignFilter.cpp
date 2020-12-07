﻿#pragma once

#define _USE_MATH_DEFINES

#include "stdafx.h"

using namespace std;

#include <windows.h>
#include <tchar.h>		//Generic-Text Mappings (TCHAR)
#include <Dshow.h>		//All DirectShow applications
#include <D3d9.h>
#include <streams.h>
#include <initguid.h>
#include <stdio.h>
#include <time.h>
#include <chrono>
#include <ctime>    
#include <conio.h>
#include <iostream>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <algorithm>
#include "resource.h"
#include <algorithm>
#include <math.h>
#include <Shlobj.h>
#include <KnownFolders.h>

#define CL_USE_DEPRECATED_OPENCL_1_2_APIS

#include <CL/cl.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/ml.hpp>

using namespace cv;

#include "rx_AlignFilter.h"
#include "rx_AlignFilter_Functions.h"
#include "Version.h"

#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <windows.h>


#pragma region Setup data

TCHAR rx_AlignFilter_KName[STR_MAX_LENGTH] = TEXT("rx_AlignFilter");

// DllEntryPoint
extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);

BOOL APIENTRY DllMain(HANDLE hModule, DWORD  dwReason, LPVOID lpReserved)
{
	//Module-Handle to myself for accessing Resources
	hMySelf = (HMODULE)hModule;
	return DllEntryPoint((HINSTANCE)(hModule), dwReason, lpReserved);
}

const AMOVIESETUP_MEDIATYPE sudPinTypes =
{
	&MEDIATYPE_Video,       // Major type
	&MEDIASUBTYPE_RGB32		//  MEDIASUBTYPE_NULL      // Minor type
};

const AMOVIESETUP_PIN psudPins[] =
{
	{ (LPWSTR)"Input",    // Pin's string name
	  FALSE,                // Is it rendered
	  FALSE,                // Is it an output
	  FALSE,                // Allowed none
	  FALSE,                // Allowed many
	  &CLSID_NULL,          // Connects to filter
	  TEXT("Output"),       // Connects to pin
	  1,                    // Number of types
	  &sudPinTypes },       // Pin information

	{ (LPWSTR)"Output",   // Pin's string name
	  FALSE,                // Is it rendered
	  FALSE,                // Is it an output
	  FALSE,                // Allowed none
	  FALSE,                // Allowed many
	  &CLSID_NULL,          // Connects to filter
	  TEXT("Input"),        // Connects to pin
	  1,                    // Number of types
	  &sudPinTypes },       // Pin information
};

const AMOVIESETUP_FILTER sud_rx_AlignFilter =
{
	&CLSID_rx_AlignFilter,	// CLSID of filter
	rx_AlignFilter_KName,	// Filter's name
	MERIT_DO_NOT_USE,			// Filter merit
	2,							// Number of pins
	psudPins					// Pin information
};

CFactoryTemplate g_Templates[] =
{
	{
		rx_AlignFilter_KName,
		&CLSID_rx_AlignFilter,
		C_rx_AlignFilter::CreateInstance,
		NULL,
		&sud_rx_AlignFilter
	}
};

int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);

#pragma endregion

#pragma region C_rx_AlignFilter

// CreateInstance
CUnknown *C_rx_AlignFilter::CreateInstance(LPUNKNOWN pUnk, HRESULT *phr)
{
	ASSERT(phr);

	C_rx_AlignFilter *pNewObject = new C_rx_AlignFilter(rx_AlignFilter_KName, pUnk, phr);

	if (pNewObject == NULL)
	{
		if (phr) *phr = E_OUTOFMEMORY;
	}

	return pNewObject;
}

// Constructor
C_rx_AlignFilter::C_rx_AlignFilter(TCHAR *pName, LPUNKNOWN pUnk, HRESULT *phr) :
	CBaseFilter(rx_AlignFilter_KName, pUnk, this, CLSID_rx_AlignFilter),
	CPersistStream(pUnk, phr),
	m_Input(L"Input", this, phr, L"Input"),
	m_Output(L"Output", this, phr, L"Output"),
	m_pAllocator(NULL),
	m_OverlayTxt(L"")
{

	if (m_DebugOn)
	{
		m_DebugStartTime = std::chrono::steady_clock::now();
		printf("rx_AlignFilter Constructor\n");
	}

	if (!InitializeCriticalSectionAndSpinCount(&m_MeasureLock, 0x00400000))
	{
		if (m_DebugOn)
		{
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tInitializeCritical MeasureLock failed\n", (float)m_TimeStamp / 1000000.0f);
			return;
		}
	}
	if (!InitializeCriticalSectionAndSpinCount(&m_OvTextLock, 0x00400000))
	{
		if (m_DebugOn)
		{
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tInitializeCritical OvTextLock failed\n", (float)m_TimeStamp / 1000000.0f);
			return;
		}
	}
	if (!InitializeCriticalSectionAndSpinCount(&m_SnapShotLock, 0x00400000))
	{
		if (m_DebugOn)
		{
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tInitializeCritical SnapShotLock failed\n", (float)m_TimeStamp / 1000000.0f);
			return;
		}
	}
}

// Destructor
C_rx_AlignFilter::~C_rx_AlignFilter()
{
	if (m_DebugOn)
	{
		m_MeasureTime = std::chrono::steady_clock::now();
		m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
		printf("%6.6f\tClosing DLL\n", (float)m_TimeStamp / 1000000.0f);

		SetDebug(FALSE);
	}

    if (OverlayBitmapReady)
	{
		DeleteObject(TextBitmap);
		DeleteDC(MemDC);
	}

	//Cleanup OpenCl
	if (ClBlockHistogramKernel != NULL) clReleaseKernel(ClBlockHistogramKernel);
	if (ClRowHistogramKernel != NULL) clReleaseKernel(ClRowHistogramKernel);
	if (ClJoinHistogramKernel != NULL) clReleaseKernel(ClJoinHistogramKernel);
	if (ClSmoothenHistogramKernel != NULL) clReleaseKernel(ClSmoothenHistogramKernel);
	if (ClShowHistogramKernel != NULL) clReleaseKernel(ClShowHistogramKernel);
	if (ClBinarizeKernel != NULL) clReleaseKernel(ClBinarizeKernel);

	if (ThresholdArray != NULL)
    {
		delete[] ThresholdArray;
		ThresholdArray = NULL;
    }
    if (HistogramArray != NULL)
    {
        delete[] HistogramArray;
        HistogramArray = NULL;
    }
    if (ThresholdLine != NULL)
    {
        delete[] ThresholdLine;
        ThresholdLine = NULL;
    }

	DeleteCriticalSection(&m_MeasureLock);
	DeleteCriticalSection(&m_OvTextLock);
	DeleteCriticalSection(&m_SnapShotLock);
}

// GetPinCount
int C_rx_AlignFilter::GetPinCount()
{
	return (2);
}

// GetPin
CBasePin *C_rx_AlignFilter::GetPin(int n)
{
	//We have exactly 2 Pins
	if (n >= 0 || n <= 1)
	{
		// 0 is Input
		if (n == 0) { return &m_Input; }
		// 1 is Output
		if (n == 1) { return &m_Output; }
	}

	return NULL;
}

// Stop, Overriden to handle no input connections
STDMETHODIMP C_rx_AlignFilter::Stop()
{
	if (m_DebugOn)
	{
		m_MeasureTime = std::chrono::steady_clock::now();
		m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
		printf("%6.6f\tStop Filter\n", (float)m_TimeStamp / 1000000.0f);
	}

	CAutoLock cObjectLock(m_pLock);

	CBaseFilter::Stop();

    if (ThresholdArray != NULL)
    {
        delete[] ThresholdArray;
        ThresholdArray = NULL;
    }
    if (HistogramArray != NULL)
    {
        delete[] HistogramArray;
        HistogramArray = NULL;
    }
    if (ThresholdLine != NULL)
    {
        delete[] ThresholdLine;
        ThresholdLine = NULL;
    }

    if (m_DebugOn)
    {
        m_MeasureTime = std::chrono::steady_clock::now();
        m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
        printf("%6.6f\tCBaseFilter::IsStopped: %d\n", (float)m_TimeStamp / 1000000.0f, CBaseFilter::IsStopped());
    }

    m_State = FILTER_STATE::State_Stopped;

	return NOERROR;
}

// Pause, Overriden to handle no input connections
STDMETHODIMP C_rx_AlignFilter::Pause()
{
	CAutoLock cObjectLock(m_pLock);
	if (m_DebugOn)
	{
		m_MeasureTime = std::chrono::steady_clock::now();
		m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
		printf("%6.6f\tPause Filter\n", (float)m_TimeStamp / 1000000.0f);
	}

	HRESULT hr = CBaseFilter::Pause();

	if (m_Input.IsConnected() == FALSE)
	{
		m_Input.EndOfStream();
	}

	m_State = FILTER_STATE::State_Paused;
	return hr;
}

// Run, Overriden to handle no input connections
STDMETHODIMP C_rx_AlignFilter::Run(REFERENCE_TIME tStart)
{
	CAutoLock cObjectLock(m_pLock);
	m_DebugOn = m_DebugPrepare;
	if (m_DebugOn)
	{
		m_MeasureTime = std::chrono::steady_clock::now();
		m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
		printf("%6.6f\tRun Filter\n", (float)m_TimeStamp / 1000000.0f);
	}

	//Prepare Dilate/Erode Seed
	CalculateErodeSeed();

	//Prepare OpenCl
	//	find Platform and Devices
	cl_device_id ClDeviceId = NULL;
	cl_platform_id ClPlatformId = NULL;
	int PlatformNum = NULL;
	int DeviceNum = NULL;
	char DeviceName[256];

	if (!FindPlatformDevice(&ClPlatformId, &PlatformNum, &ClDeviceId, &DeviceNum, DeviceName))
	{
		//No useful platform found
		if (m_DebugOn)
		{
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tCould not find a parallel processing device\n", (float)m_TimeStamp / 1000000.0f);
		}
		exit(-1);
	}

	//Set Devicename to be displayed
	int NameLen = (int)strlen(DeviceName);
	wchar_t* wDeviceName = new wchar_t[256];
	MultiByteToWideChar(CP_ACP, 0, DeviceName, NameLen + 1, wDeviceName, NameLen + 1);
	m_DeviceName = _wcsdup(wDeviceName);
	delete[] wDeviceName;

	//start OpenCl
	context = CreateContext(ClPlatformId, PlatformNum, ClDeviceId, DeviceNum);
	queue = CreateCommandQueue(context, ClDeviceId, DeviceNum);
	Program = MakeProgram(IDR_OPENCL1, context, ClDeviceId, DeviceNum);
	if (!CreateKernels(Program))
	{
		exit(-1);
	}


	HRESULT hr = CBaseFilter::Run(tStart);
    if (m_DebugOn)
    {
        DWORD dwTime;
        FILTER_STATE fState;
        CBaseFilter::GetState(dwTime, &fState);

        m_MeasureTime = std::chrono::steady_clock::now();
        m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
        printf("%6.6f\tCBaseFilter::Run: %d\n", (float)m_TimeStamp / 1000000.0f, fState);
    }


	if (m_Input.IsConnected() == FALSE)
	{
		m_Input.EndOfStream();
        if (m_DebugOn)
        {
            m_MeasureTime = std::chrono::steady_clock::now();
            m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
            printf("%6.6f\tm_Input.IsConnected() == FALSE\n", (float)m_TimeStamp / 1000000.0f);
        }
    }

	//Bitmapinfo
	VIDEOINFOHEADER* pVih = (VIDEOINFOHEADER*)m_mt1.Format();
	ASSERT(pVih);
	//Bitmaps Size
	m_ImageWidth = (UINT)pVih->bmiHeader.biWidth; // Width
	m_ImageHeight = (UINT)pVih->bmiHeader.biHeight; // Height

	//Create arrays for Multi Color Histogram and Threshold, filled with default Threshold
	ThresholdArray = new uchar[(size_t)m_ImageHeight * (size_t)m_ImageWidth];
	HistogramArray = new UINT[(size_t)m_ImageHeight * (size_t)m_ImageWidth];
	for (uint i = 0; i < m_ImageHeight * m_ImageWidth; i++)
	{
		ThresholdArray[i] = m_Threshold;
		HistogramArray[i] = 0;
	}

	ThresholdLine = new uchar[(size_t)m_ImageWidth];
	for (uint i = 0; i < m_ImageWidth; i++)
	{
		ThresholdLine[i] = m_Threshold;
	}

	if (m_DebugOn)
	{
		m_MeasureTime = std::chrono::steady_clock::now();
		m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
		printf("%6.6f\tRun Filter Done\n", (float)m_TimeStamp / 1000000.0f);
	}

	return hr;
}

HRESULT C_rx_AlignFilter::FrameFinished()
{
	return NOERROR;
}

// NonDelegatingQueryInterface, Reveals IIPEffect and ISpecifyPropertyPages
STDMETHODIMP C_rx_AlignFilter::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
	CheckPointer(ppv, E_POINTER);

if (riid == IID_IFrx_AlignFilter)
	{//Property Page abrufen
		return GetInterface(static_cast<IFrx_AlignFilter*>(this), ppv);
	}
	else
	{//Nicht vorhanden
		return CBaseFilter::NonDelegatingQueryInterface(riid, ppv);
	}
}

// GetClassID, This is the only method of IPersist
STDMETHODIMP C_rx_AlignFilter::GetClassID(CLSID* pClsid)
{
	return CBaseFilter::GetClassID(pClsid);
}


//Change Settings and Modes
HRESULT C_rx_AlignFilter::ChangeModes()
{
	if (m_MeasureMode != m_PresetMeasureMode)
	{
		m_MeasureRunning = false;
		m_measureDone = false;
		m_MeasureMode = m_PresetMeasureMode;
	}
	m_DisplayMode = m_PresetDisplayMode;

	m_ShowOriginal = m_PresetShowOriginal;
	m_BinarizeMode = m_PresetBinarizeMode;
	m_Threshold = m_PresetThreshold;
	m_ShowHistogram = m_PresetShowHistogram;
	m_DilateErodes = m_PresetDilateErodes;
	m_ExtraErodes = m_PresetExtraErodes;

	if (m_ErodeSeedX != m_PresetErodeSeedX || m_ErodeSeedY != m_PresetErodeSeedY)
	{
		m_ErodeSeedX = m_PresetErodeSeedX;
		m_ErodeSeedY = m_PresetErodeSeedY;
		CalculateErodeSeed();
	}

	m_BlobAspectLimit = m_PresetBlobAspectLimit;
	m_BlobAreaDivisor = m_PresetBlobAreaDivisor;

	//Start Measure
	if (m_StartMeasure)
	{
		m_MeasureRunning = true;
	}
	m_StartMeasure = false;

	SetDirty(TRUE);
	return NOERROR;
}

//Binarize

HRESULT C_rx_AlignFilter::GetHistogram(IMediaSample* pSampleIn, UINT* Histogram, UINT BinarizationMode)
{
	//Adaptive binarization Get Histogram

	CheckPointer(pSampleIn, E_POINTER);
	HRESULT hr = NOERROR;

	//OpenCl Return Value
	cl_int cl_Error = CL_SUCCESS;

	#pragma region create CL Buffers

	//Input Image Pointer for OpenCL
	BYTE *pInBuffer;
	hr = pSampleIn->GetPointer(&pInBuffer);
	if (FAILED(hr))
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s(hr, MsgMsg, 10, 16);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tGetHistogram: Get Pointer InBuffer Error: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		return E_POINTER;
	}
	cl_uchar4 *pClSourceImage = (cl_uchar4*)pInBuffer;
	
	//Input Image Buffer for OpenCL
	cl_mem ClSourceImageBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, pSampleIn->GetActualDataLength(), (void*)pInBuffer, &cl_Error);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tGetHistogram: Could not create OpenCL Input Image Buffer: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}

	//Histogram Buffer
	cl_mem ClHistogramBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(cl_uint) * 256, (void*)Histogram, &cl_Error);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tGetHistogram: Could not create  OpenCL Histogram Buffer: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}

	//Histogram Row Buffer
	UINT* RowHistogram;
	RowHistogram = (UINT*)malloc((sizeof UINT) * 256 * m_ImageWidth);
	if (RowHistogram == NULL)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tGetHistogram: Could not allocate Histogram Buffer\n", (float)m_TimeStamp / 1000000.0f);
		}
		exit(-1);
	}
	memset(RowHistogram, 0, (sizeof UINT) * 256 * m_ImageWidth);
	cl_mem ClRowHistogramBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(cl_uint) * 256 * m_ImageWidth, (void*)RowHistogram, &cl_Error);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tGetHistogram: Could not create OpenCL Raw-Histogram Buffer: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}

	//Raw Histogram Buffer
	UINT* RawHistogram;
	RawHistogram = (UINT*)malloc((sizeof UINT) * 256);
	if (RawHistogram == NULL)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tGetHistogram: Could not allocate Histogram Buffer\n", (float)m_TimeStamp / 1000000.0f);
		}
		exit(-1);
	}
	memset(RawHistogram, 0, (sizeof UINT) * 256);
	cl_mem ClRawHistogramBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(cl_uint) * 256, (void*)RawHistogram, &cl_Error);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tGetHistogram: Could not create OpenCL Row-Histogram Buffer: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}

	#pragma endregion

	#pragma region Block Histogram

	//Set Arguments for ClBlockHistogramKernel
	cl_Error = clSetKernelArg(ClBlockHistogramKernel, 0, sizeof(cl_mem), &ClSourceImageBuffer);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tGetHistogram: Could not set Argument 0 of OpenCL Kernel HistogramBlock: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}
	cl_Error = clSetKernelArg(ClBlockHistogramKernel, 1, sizeof(cl_mem), &ClRowHistogramBuffer);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tGetHistogram: Could not set Argument 1 of OpenCL Kernel HistogramBlock: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}
	cl_Error = clSetKernelArg(ClBlockHistogramKernel, 2, sizeof(cl_uint), &m_ImageWidth);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tGetHistogram: Could not set Argument 2 of OpenCL Kernel HistogramBlock: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}

	//Run ClBlockHistogramKernel
	size_t GlobalWorkSize1[] = { (size_t)(m_ImageHeight / 8), (size_t)1 };
	cl_Error = clEnqueueNDRangeKernel(queue, ClBlockHistogramKernel, 1, NULL, GlobalWorkSize1, NULL, 0, NULL, NULL);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tGetHistogram: Could not run OpenCL Kernel HistogramBlock: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}
	cl_Error = clFinish(queue);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tGetHistogram: Could not finish OpenCL Kernel HistogramBlock: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}

	#pragma endregion

	#pragma region Join Histogram

	//Set Arguments for ClJoinHistogramKernel
	cl_Error = clSetKernelArg(ClJoinHistogramKernel, 0, sizeof(cl_mem), &ClRowHistogramBuffer);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tGetHistogram: Could not set Argument 0 of OpenCL Kernel JoinHistogram: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}
	cl_Error = clSetKernelArg(ClJoinHistogramKernel, 1, sizeof(cl_mem), &ClRawHistogramBuffer);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tGetHistogram: Could not set Argument 1 of OpenCL Kernel JoinHistogram: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}
	cl_Error = clSetKernelArg(ClJoinHistogramKernel, 2, sizeof(cl_uint), &m_ImageHeight);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tGetHistogram: Could not set Argument 2 of OpenCL Kernel JoinHistogram: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}

	//Run ClJoinHistogramKernel
	size_t GlobalWorkSize2[] = { (size_t)(256), (size_t)1 };
	cl_Error = clEnqueueNDRangeKernel(queue, ClJoinHistogramKernel, 1, NULL, GlobalWorkSize2, NULL, 0, NULL, NULL);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tGetHistogram: Could not run OpenCL Kernel JoinHistogram: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}
	cl_Error = clFinish(queue);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tGetHistogram: Could not finish OpenCL Kernel JoinHistogram: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}

	#pragma endregion

	#pragma region Smoothen Histogram

	UINT SmoothenWidth = 15;

	//Set Arguments for ClJoinHistogramKernel
	cl_Error = clSetKernelArg(ClSmoothenHistogramKernel, 0, sizeof(cl_mem), &ClRawHistogramBuffer);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tGetHistogram: Could not set Argument 0 of OpenCL Kernel SmoothenHistogram: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}
	cl_Error = clSetKernelArg(ClSmoothenHistogramKernel, 1, sizeof(cl_mem), &ClHistogramBuffer);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tGetHistogram: Could not set Argument 1 of OpenCL Kernel SmoothenHistogram: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}
	cl_Error = clSetKernelArg(ClSmoothenHistogramKernel, 2, sizeof(cl_uint), &SmoothenWidth);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tGetHistogram: Could not set Argument 2 of OpenCL Kernel SmoothenHistogram: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}

	//Run ClSmoothenHistogramKernel
	size_t GlobalWorkSize3[] = { (size_t)(256), (size_t)1 };
	cl_Error = clEnqueueNDRangeKernel(queue, ClSmoothenHistogramKernel, 1, NULL, GlobalWorkSize3, NULL, 0, NULL, NULL);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tGetHistogram: Could not run OpenCL Kernel SmoothenHistogram: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}
	cl_Error = clFinish(queue);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tGetHistogram: Could not finish OpenCL Kernel SmoothenHistogram: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}

	#pragma endregion

	#pragma region Retrieve Buffers

	//Retrieve Buffers
	cl_Error = clEnqueueReadBuffer(queue, ClHistogramBuffer, CL_TRUE, 0, sizeof(cl_uint) * 256, (void*)Histogram, 0, NULL, NULL);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tGetHistogram: Could not read OpenCL Histogram Buffer: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}

	#pragma endregion

	#pragma region Cleanup CL

	//Cleanup
	cl_Error = clReleaseMemObject(ClSourceImageBuffer);
	cl_Error = clReleaseMemObject(ClHistogramBuffer);
	cl_Error = clReleaseMemObject(ClRowHistogramBuffer);
	cl_Error = clReleaseMemObject(ClRawHistogramBuffer);
	free(RowHistogram);
	free(RawHistogram);

	#pragma endregion

	if (BinarizationMode > 1)
	{
		UINT NewThreshold = m_Threshold;
		int CalcResult = CalcThreshold(Histogram, &m_Peak_1_Val, &m_Peak_1_Pos, &m_Peak_2_Val, &m_Peak_2_Pos, &NewThreshold);

		m_Threshold = (m_Threshold + NewThreshold) / 2;
		if (m_Threshold <= 0) m_Threshold = 127;
	}

	return NOERROR;
}

HRESULT C_rx_AlignFilter::ShowHistogram(IMediaSample* pSampleIn, UINT* Histogram)
{
	//Adaptive binarization Show Histogram

	CheckPointer(pSampleIn, E_POINTER);
	HRESULT hr = NOERROR;

	//OpenCl Return Value
	cl_int cl_Error;

#pragma region create CL Buffers

	//Input Image Pointer for OpenCL
	BYTE* pInBuffer;
	hr = pSampleIn->GetPointer(&pInBuffer);
	if (FAILED(hr))
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s(hr, MsgMsg, 10, 16);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tShowHistogram: Get Pointer InBuffer Error: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		return E_POINTER;
	}
	//Input Image Buffer for OpenCL
	cl_mem ClSourceImageBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, pSampleIn->GetActualDataLength(), (void*)pInBuffer, &cl_Error);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tShowHistogram: Could not create OpenCL Input Image Buffer: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}

	//Histogram Buffer
	cl_mem ClHistogramBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(cl_uint) * 256, (void*)Histogram, &cl_Error);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tShowHistogram: Could not create OpenCL Histogram Buffer: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}

#pragma endregion

	//Max Peak in Histogram
	UINT MaxHistoVal = *max_element(&Histogram[0], &Histogram[256]);

#pragma region run kernel

	//Set Arguments
	cl_Error = clSetKernelArg(ClShowHistogramKernel, 0, sizeof(cl_mem), &ClSourceImageBuffer);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tShowHistogram: Could not set Argument 0 of OpenCL Kernel ShowHistogram: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}
	cl_Error = clSetKernelArg(ClShowHistogramKernel, 1, sizeof(cl_mem), &ClHistogramBuffer);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tShowHistogram: Could not set Argument 1 of OpenCL Kernel ShowHistogram: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}
	cl_Error = clSetKernelArg(ClShowHistogramKernel, 2, sizeof(cl_uint), (cl_uint*)&MaxHistoVal);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tShowHistogram: Could not set Argument 2 of OpenCL Kernel ShowHistogram: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}
	cl_Error = clSetKernelArg(ClShowHistogramKernel, 3, sizeof(cl_uint), (cl_uint*)&m_Peak_1_Val);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tShowHistogram: Could not set Argument 3 of OpenCL Kernel ShowHistogram: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}
	cl_Error = clSetKernelArg(ClShowHistogramKernel, 4, sizeof(cl_uint), (cl_uint*)&m_Peak_1_Pos);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tShowHistogram: Could not set Argument 4 of OpenCL Kernel ShowHistogram: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}
	cl_Error = clSetKernelArg(ClShowHistogramKernel, 5, sizeof(cl_uint), (cl_uint*)&m_Peak_2_Val);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tShowHistogram: Could not set Argument 5 of OpenCL Kernel ShowHistogram: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}
	cl_Error = clSetKernelArg(ClShowHistogramKernel, 6, sizeof(cl_uint), (cl_uint*)&m_Peak_2_Pos);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tShowHistogram: Could not set Argument 6 of OpenCL Kernel ShowHistogram: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}
	cl_Error = clSetKernelArg(ClShowHistogramKernel, 7, sizeof(cl_uint), (cl_uint*)&m_Threshold);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tShowHistogram: Could not set Argument 7 of OpenCL Kernel ShowHistogram: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}
	cl_Error = clSetKernelArg(ClShowHistogramKernel, 8, sizeof(cl_uint), (cl_uint*)&m_ImageWidth);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tShowHistogram: Could not set Argument 8 of OpenCL Kernel ShowHistogram: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}
	cl_Error = clSetKernelArg(ClShowHistogramKernel, 9, sizeof(cl_uint), (cl_uint*)&m_ImageHeight);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tShowHistogram: Could not set Argument 9 of OpenCL Kernel ShowHistogram: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}


	//Run ClShowHistogramKernel
	size_t GlobalWorkSize1[] = { (size_t)256, (size_t)1 };
	cl_Error = clEnqueueNDRangeKernel(queue, ClShowHistogramKernel, 1, NULL, GlobalWorkSize1, NULL, 0, NULL, NULL);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tShowHistogram: Could not run OpenCL Kernel ShowHistogram: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}
	cl_Error = clFinish(queue);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tShowHistogram: Could not finish OpenCL Kernel ShowHistogram: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}

#pragma endregion

#pragma region Retrieve Buffers

	//Retrieve Buffers
	cl_Error = clEnqueueReadBuffer(queue, ClSourceImageBuffer, CL_TRUE, 0, pSampleIn->GetActualDataLength(), (void*)pInBuffer, 0, NULL, NULL);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tShowHistogram: Could not read OpenCL Histogram overlay Buffer: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}

#pragma endregion

#pragma region Cleanup CL

	//Cleanup
	cl_Error = clReleaseMemObject(ClSourceImageBuffer);
	cl_Error = clReleaseMemObject(ClHistogramBuffer);

#pragma endregion

	return NOERROR;
}

HRESULT C_rx_AlignFilter::Binarize(IMediaSample* pSampleIn, IMediaSample* pSampleOut)
{
	//Adaptive binarization algorithm

	CheckPointer(pSampleIn, E_POINTER);
	CheckPointer(pSampleOut, E_POINTER);
	HRESULT hr = NOERROR;

	//OpenCl Return Value
	cl_int cl_Error;

	#pragma region create CL Buffers

	//Input Image Pointer for OpenCL
	BYTE *pInBuffer;
	hr = pSampleIn->GetPointer(&pInBuffer);
	if (FAILED(hr))
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s(hr, MsgMsg, 10, 16);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tBinarize: Get Pointer InBuffer Error: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		return E_POINTER;
	}
	//Input Image Buffer for OpenCL
	cl_mem ClSourceImageBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, pSampleIn->GetActualDataLength(), (void*)pInBuffer, &cl_Error);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tBinarize: Could not create OpenCL Input Image Buffer: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}

	//Output Image Pointer for OpenCL
	BYTE *pOutBuffer;
	hr = pSampleOut->GetPointer(&pOutBuffer);
	if (FAILED(hr))
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s(hr, MsgMsg, 10, 16);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tBinarize: Get Pointer OutBuffer Error: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		return E_POINTER;
	}
	//Output Image Buffer for OpenCL
	cl_mem ClDestImageBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, pSampleOut->GetActualDataLength(), (void*)pOutBuffer, &cl_Error);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tBinarize: Could not create OpenCL Output Image Buffer: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}


	#pragma endregion


	if (m_BinarizeMode > 0)
	{
		//Set Arguments
		cl_Error = clSetKernelArg(ClBinarizeKernel, 0, sizeof(cl_mem), &ClSourceImageBuffer);
		if (cl_Error != CL_SUCCESS)
		{
			if (m_DebugOn)
			{
				TCHAR MsgMsg[10];
				_itow_s((int)cl_Error, MsgMsg, 10, 10);
				m_MeasureTime = std::chrono::steady_clock::now();
				m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
				printf("%6.6f\tBinarize: Could not set Argument 0 of OpenCL Kernel Binarize: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
			}
			exit(-1);
		}
		cl_Error = clSetKernelArg(ClBinarizeKernel, 1, sizeof(cl_mem), &ClDestImageBuffer);
		if (cl_Error != CL_SUCCESS)
		{
			if (m_DebugOn)
			{
				TCHAR MsgMsg[10];
				_itow_s((int)cl_Error, MsgMsg, 10, 10);
				m_MeasureTime = std::chrono::steady_clock::now();
				m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
				printf("%6.6f\tBinarize: Could not set Argument 1 of OpenCL Kernel Binarize: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
			}
			exit(-1);
		}
		cl_Error = clSetKernelArg(ClBinarizeKernel, 2, sizeof(cl_uint), (cl_uint*)&m_Threshold);
		if (cl_Error != CL_SUCCESS)
		{
			if (m_DebugOn)
			{
				TCHAR MsgMsg[10];
				_itow_s((int)cl_Error, MsgMsg, 10, 10);
				m_MeasureTime = std::chrono::steady_clock::now();
				m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
				printf("%6.6f\tBinarize: Could not set Argument 2 of OpenCL Kernel Binarize: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
			}
			exit(-1);
		}

		cl_uint InverseIt = 0;
		if (m_InverseImage)
		{
			InverseIt = 1;
		}
		cl_Error = clSetKernelArg(ClBinarizeKernel, 3, sizeof(cl_uint), (cl_uint*) &InverseIt);
		if (cl_Error != CL_SUCCESS)
		{
			if (m_DebugOn)
			{
				TCHAR MsgMsg[10];
				_itow_s((int)cl_Error, MsgMsg, 10, 10);
				m_MeasureTime = std::chrono::steady_clock::now();
				m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
				printf("%6.6f\tBinarize: Could not set Argument 3 of OpenCL Kernel Binarize: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
			}
			exit(-1);
		}


		//Run the Kernel
		size_t GlobalWorkSize2[] = { (size_t)((size_t)m_ImageWidth * (size_t)m_ImageHeight), (size_t)1 };
		cl_Error = clEnqueueNDRangeKernel(queue, ClBinarizeKernel, 1, NULL, GlobalWorkSize2, NULL, 0, NULL, NULL);
		if (cl_Error != CL_SUCCESS)
		{
			if (m_DebugOn)
			{
				TCHAR MsgMsg[10];
				_itow_s((int)cl_Error, MsgMsg, 10, 10);
				m_MeasureTime = std::chrono::steady_clock::now();
				m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
				printf("%6.6f\tBinarize: Could not run OpenCL Kernel Binarize: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
			}
			exit(-1);
		}
		cl_Error = clFinish(queue);
		if (cl_Error != CL_SUCCESS)
		{
			if (m_DebugOn)
			{
				TCHAR MsgMsg[10];
				_itow_s((int)cl_Error, MsgMsg, 10, 10);
				m_MeasureTime = std::chrono::steady_clock::now();
				m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
				printf("%6.6f\tBinarize: Could not finish OpenCL Kernel Binarize: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
			}
			exit(-1);
		}
	}

	#pragma region Retrieve Buffers

	//Retrieve Buffers
	cl_Error = clEnqueueReadBuffer(queue, ClDestImageBuffer, CL_TRUE, 0, pSampleOut->GetActualDataLength(), (void*)pOutBuffer, 0, NULL, NULL);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tBinarize: Could not read OpenCL Binarized Buffer: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}

	#pragma endregion


	#pragma region Cleanup CL

	//Cleanup Buffers
	cl_Error = clReleaseMemObject(ClSourceImageBuffer);
	cl_Error = clReleaseMemObject(ClDestImageBuffer);

	#pragma endregion

	return hr;
}

//Binarize Multi Color

HRESULT C_rx_AlignFilter::GetColorHistogram(IMediaSample* pSampleIn, BOOL Vertical)
{
	//Adaptive binarization get Multi Color Histogram

	CheckPointer(pSampleIn, E_POINTER);
	HRESULT hr = NOERROR;

	//OpenCl Return Value
	cl_int cl_Error = CL_SUCCESS;

	//Histogram for each column if vertical lines or each row if horizontal lines

	#pragma region create CL Buffers for ColorHistogram Kernel

	//Input Image Pointer for OpenCL
	BYTE* pInBuffer;
	hr = pSampleIn->GetPointer(&pInBuffer);
	if (FAILED(hr))
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s(hr, MsgMsg, 10, 16);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tGetColorHistogram: Get Pointer InBuffer Error: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		return E_POINTER;
	}

	//Input Image Buffer for OpenCL
	cl_mem ClSourceImageBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, pSampleIn->GetActualDataLength(), (void*)pInBuffer, &cl_Error);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tGetColorHistogram: Could not create OpenCL Input Image Buffer: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}

	//Histogram array Buffer for OpenCl
	memset(HistogramArray, 0, (sizeof UINT) * m_ImageHeight * m_ImageWidth);
	cl_mem ClHistogramArrayBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(cl_uint) * m_ImageHeight * m_ImageWidth, (void*)HistogramArray, &cl_Error);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tGetColorHistogram: Could not create OpenCL Color-Histogram Buffer: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}

	#pragma endregion

	#pragma region set Arguments and run ColorHistogram Kernel

	//Set Arguments
	cl_Error = clSetKernelArg(ClColorHistogramKernel, 0, sizeof(cl_mem), &ClSourceImageBuffer);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tGetColorHistogram: Could not set Argument 0 of OpenCL Kernel Color-Histogram: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}
	cl_Error = clSetKernelArg(ClColorHistogramKernel, 1, sizeof(cl_mem), &ClHistogramArrayBuffer);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tGetColorHistogram: Could not set Argument 1 of OpenCL Kernel Color-Histogram: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}
	cl_Error = clSetKernelArg(ClColorHistogramKernel, 2, sizeof(cl_uint), &m_ImageHeight);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tGetColorHistogram: Could not set Argument 2 of OpenCL Kernel Color-Histogram: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}
	cl_Error = clSetKernelArg(ClColorHistogramKernel, 3, sizeof(cl_uint), &m_ImageWidth);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tGetColorHistogram: Could not set Argument 3 of OpenCL Kernel Color-Histogram: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}
	UINT Horizontal = 1;
	if(Vertical)Horizontal = 0;
	cl_Error = clSetKernelArg(ClColorHistogramKernel, 4, sizeof(cl_uint), &Horizontal);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tGetColorHistogram: Could not set Argument 4 of OpenCL Kernel Color-Histogram: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}


	//Run the Kernel
	size_t GlobalWorkSize1[] = { (size_t)(Vertical ? m_ImageWidth : m_ImageHeight), (size_t)1 };
	cl_Error = clEnqueueNDRangeKernel(queue, ClColorHistogramKernel, 1, NULL, GlobalWorkSize1, NULL, 0, NULL, NULL);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tGetColorHistogram: Could not run OpenCL Kernel Color-Histogram: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}
	cl_Error = clFinish(queue);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tGetColorHistogram: Could not finish OpenCL Kernel Color-Histogram: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}

	#pragma endregion

	//Calculate Threshold for each of these Lines

	#pragma region  create CL Buffers for ColorThresholdLines Kernel

	//ThresholdLineBuffer
	cl_mem ClThresholdLineBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(cl_uchar) * m_ImageWidth, (void*)ThresholdLine, &cl_Error);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tGetColorHistogram: Could not create OpenCL Color-ThresholdLine Buffer: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}

	#pragma endregion

	#pragma region set Arguments and run ColorThresholdLines Kernel

	//Set Arguments
	cl_Error = clSetKernelArg(ClColorThresholdLinesKernel, 0, sizeof(cl_mem), &ClHistogramArrayBuffer);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tGetColorHistogram: Could not set Argument 0 of OpenCL Kernel ColorThresholdLines: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}
	cl_Error = clSetKernelArg(ClColorThresholdLinesKernel, 1, sizeof(cl_mem), &ClThresholdLineBuffer);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tGetColorHistogram: Could not set Argument 1 of OpenCL Kernel ColorThresholdLines: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}
	cl_Error = clSetKernelArg(ClColorThresholdLinesKernel, 2, sizeof(cl_uint), Vertical ? &m_ImageWidth : &m_ImageHeight);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tGetColorHistogram: Could not set Argument 2 of OpenCL Kernel ColorThresholdLines: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}


	//Run the Kernel
	GlobalWorkSize1[0] = (size_t)(Vertical ? m_ImageWidth : m_ImageHeight);
	cl_Error = clEnqueueNDRangeKernel(queue, ClColorThresholdLinesKernel, 1, NULL, GlobalWorkSize1, NULL, 0, NULL, NULL);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tGetColorHistogram: Could not run OpenCL Kernel ColorThresholdLines: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}
	cl_Error = clFinish(queue);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tGetColorHistogram: Could not finish OpenCL Kernel ColorThresholdLines: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}



	#pragma endregion

	//Average Threshold for each Pixel

	#pragma region Create CL Buffers for Color ThresholdAverage Kernel

	//Threshold array Buffer for OpenCl
	cl_mem ClThresholdArrayBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(cl_uchar) * m_ImageHeight * m_ImageWidth, (void*)ThresholdArray, &cl_Error);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tGetColorHistogram: Could not create OpenCL Threshold Array Buffer: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}

	#pragma endregion

	#pragma region set Arguments and run ThresholdAverage Kernel

	//Set Arguments
	cl_Error = clSetKernelArg(ClColorThresholdAverageKernel, 0, sizeof(cl_mem), &ClThresholdLineBuffer);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tGetColorHistogram: Could not set Argument 0 of OpenCL Kernel Color-ThresholdAverage: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}
	cl_Error = clSetKernelArg(ClColorThresholdAverageKernel, 1, sizeof(cl_mem), &ClThresholdArrayBuffer);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tGetColorHistogram: Could not set Argument 1 of OpenCL Kernel Color-ThresholdAverage: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}
	cl_Error = clSetKernelArg(ClColorThresholdAverageKernel, 2, sizeof(cl_uint), &m_ImageHeight);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tGetColorHistogram: Could not set Argument 2 of OpenCL Kernel Color-ThresholdAverage: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}
	cl_Error = clSetKernelArg(ClColorThresholdAverageKernel, 3, sizeof(cl_uint), &m_ImageWidth);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tGetColorHistogram: Could not set Argument 3 of OpenCL Kernel Color-ThresholdAverage: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}
	Horizontal = 1;
	if (Vertical)Horizontal = 0;
	cl_Error = clSetKernelArg(ClColorThresholdAverageKernel, 4, sizeof(cl_uint), &Horizontal);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tGetColorHistogram: Could not set Argument 4 of OpenCL Kernel Color-ThresholdAverage: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}

	int AvgSize = (Vertical ? m_ImageWidth : m_ImageHeight) / 20;
	cl_Error = clSetKernelArg(ClColorThresholdAverageKernel, 5, sizeof(cl_int), &AvgSize);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tGetColorHistogram: Could not set Argument 5 of OpenCL Kernel Color-ThresholdAverage: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}

	//Run the Kernel
	GlobalWorkSize1[0] = (size_t)(Vertical ? m_ImageWidth : m_ImageHeight);
	cl_Error = clEnqueueNDRangeKernel(queue, ClColorThresholdAverageKernel, 1, NULL, GlobalWorkSize1, NULL, 0, NULL, NULL);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tGetColorHistogram: Could not run OpenCL Kernel Color-ThresholdAverage: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}
	cl_Error = clFinish(queue);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tGetColorHistogram: Could not finish OpenCL Kernel Color-ThresholdAverage: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}

	#pragma endregion

	#pragma region Retrieve Buffers

	//Retrieve Buffers
	cl_Error = clEnqueueReadBuffer(queue, ClHistogramArrayBuffer, CL_TRUE, 0, sizeof(cl_uint) * m_ImageHeight * m_ImageWidth, (void*)HistogramArray, 0, NULL, NULL);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tGetColorHistogram: Could not read OpenCL Histogram Array Buffer: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}

	cl_Error = clEnqueueReadBuffer(queue, ClThresholdLineBuffer, CL_TRUE, 0, sizeof(cl_uchar) * m_ImageWidth, (void*)ThresholdLine, 0, NULL, NULL);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tGetColorHistogram: Could not read OpenCL Threshold Line Buffer: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}

	cl_Error = clEnqueueReadBuffer(queue, ClThresholdArrayBuffer, CL_TRUE, 0, sizeof(cl_uchar) * m_ImageHeight * m_ImageWidth, (void*)ThresholdArray, 0, NULL, NULL);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tGetColorHistogram: Could not read OpenCL Threshold Array Buffer: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}

	#pragma endregion

	#pragma region Cleanup CL

	//Cleanup Buffers
	cl_Error = clReleaseMemObject(ClSourceImageBuffer);
	cl_Error = clReleaseMemObject(ClHistogramArrayBuffer);
	cl_Error = clReleaseMemObject(ClThresholdLineBuffer);
	cl_Error = clReleaseMemObject(ClThresholdArrayBuffer);

	#pragma endregion

	return NOERROR;
}

HRESULT C_rx_AlignFilter::ShowColorHistogram(IMediaSample* pSampleIn, UINT* Histogram, BOOL Vertical)
{
	//Adaptive binarization Show Multi Color  Histogram

	CheckPointer(pSampleIn, E_POINTER);
	HRESULT hr = NOERROR;

	//OpenCl Return Value
	cl_int cl_Error;

	#pragma region create CL Buffers

	//Input Image Pointer for OpenCL
	BYTE* pInBuffer;
	hr = pSampleIn->GetPointer(&pInBuffer);
	if (FAILED(hr))
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s(hr, MsgMsg, 10, 16);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tShowColorHistogram: Get Pointer InBuffer Error: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		return E_POINTER;
	}
	//Input Image Buffer for OpenCL
	cl_mem ClSourceImageBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, pSampleIn->GetActualDataLength(), (void*)pInBuffer, &cl_Error);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tShowColorHistogram: Could not create OpenCL Input Image Buffer: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}

	//ThresholdLineBuffer
	cl_mem ClThresholdLineBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(cl_uchar) * m_ImageWidth, (void*)ThresholdLine, &cl_Error);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tShowColorHistogram: Could not create OpenCL Color-ThresholdLine Buffer: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}

	//Threshold array Buffer for OpenCl
	cl_mem ClThresholdArrayBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(cl_uchar) * m_ImageHeight * m_ImageWidth, (void*)ThresholdArray, &cl_Error);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tShowColorHistogram: Could not create OpenCL Threshold Array Buffer: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}

	#pragma endregion

	#pragma region set Arguments and run ShowColorHistogram Kernel

	//Set Arguments
	cl_Error = clSetKernelArg(ClShowColorHistogramKernel, 0, sizeof(cl_mem), &ClSourceImageBuffer);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tShowColorHistogram: Could not set Argument 0 of OpenCL Kernel ShowColorHistogram: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}
	cl_Error = clSetKernelArg(ClShowColorHistogramKernel, 1, sizeof(cl_mem), &ClThresholdLineBuffer);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tShowColorHistogram: Could not set Argument 1 of OpenCL Kernel ShowColorHistogram: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}
	cl_Error = clSetKernelArg(ClShowColorHistogramKernel, 2, sizeof(cl_mem), &ClThresholdArrayBuffer);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tShowColorHistogram: Could not set Argument 2 of OpenCL Kernel ShowColorHistogram: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}
	cl_Error = clSetKernelArg(ClShowColorHistogramKernel, 3, sizeof(cl_uint), &m_ImageHeight);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tShowColorHistogram: Could not set Argument 3 of OpenCL Kernel ShowColorHistogram: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}
	cl_Error = clSetKernelArg(ClShowColorHistogramKernel, 4, sizeof(cl_uint), &m_ImageWidth);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tShowColorHistogram: Could not set Argument 4 of OpenCL Kernel ShowColorHistogram: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}
	cl_Error = clSetKernelArg(ClShowColorHistogramKernel, 5, sizeof(cl_uint), (Vertical ? (new UINT{ 0 }) : (new UINT{ 1 })));
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tShowColorHistogram: Could not set Argument 5 of OpenCL Kernel ShowColorHistogram: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}

	//Run the Kernel
	size_t GlobalWorkSize1[] = { (size_t)(Vertical ? m_ImageWidth : m_ImageHeight), (size_t)1 };
	cl_Error = clEnqueueNDRangeKernel(queue, ClShowColorHistogramKernel, 1, NULL, GlobalWorkSize1, NULL, 0, NULL, NULL);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tShowColorHistogram: Could not run OpenCL Kernel ShowColorHistogram: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}
	cl_Error = clFinish(queue);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tShowColorHistogram: Could not finish OpenCL Kernel ShowColorHistogram: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}

	#pragma endregion

	#pragma region Retrieve Buffers

	//Retrieve Buffers
	cl_Error = clEnqueueReadBuffer(queue, ClSourceImageBuffer, CL_TRUE, 0, pSampleIn->GetActualDataLength(), (void*)pInBuffer, 0, NULL, NULL);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tShowColorHistogram: Could not read OpenCL Image Buffer: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}

	#pragma endregion

	#pragma region Cleanup CL

	//Cleanup Buffers
	cl_Error = clReleaseMemObject(ClSourceImageBuffer);
	cl_Error = clReleaseMemObject(ClThresholdLineBuffer);
	cl_Error = clReleaseMemObject(ClThresholdArrayBuffer);

	#pragma endregion

	return NOERROR;
}

HRESULT C_rx_AlignFilter::BinarizeMultiColor(IMediaSample* pSampleIn, IMediaSample* pSampleOut, UINT BinarizationMode)
{
	//Binarize each Pixel with its own Threshold

	CheckPointer(pSampleIn, E_POINTER);
	CheckPointer(pSampleOut, E_POINTER);
	HRESULT hr = NOERROR;

	//OpenCl Return Value
	cl_int cl_Error;

	#pragma region create CL Buffers

	//Input Image Pointer for OpenCL
	BYTE* pInBuffer;
	hr = pSampleIn->GetPointer(&pInBuffer);
	if (FAILED(hr))
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s(hr, MsgMsg, 10, 16);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tBinarizeMultiColor: Get Pointer InBuffer Error: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		return E_POINTER;
	}

	//Input Image Buffer for OpenCL
	cl_mem ClSourceImageBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, pSampleIn->GetActualDataLength(), (void*)pInBuffer, &cl_Error);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tBinarizeMultiColor: Could not create OpenCL Input Image Buffer: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}

	//Output Image Pointer for OpenCL
	BYTE* pOutBuffer;
	hr = pSampleOut->GetPointer(&pOutBuffer);
	if (FAILED(hr))
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s(hr, MsgMsg, 10, 16);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tBinarizeMultiColor: Get Pointer OutBuffer Error: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		return E_POINTER;
	}

	//Output Image Buffer for OpenCL
	cl_mem ClDestImageBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, pSampleOut->GetActualDataLength(), (void*)pOutBuffer, &cl_Error);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tBinarizeMultiColor: Could not create OpenCL Output Image Buffer: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}

	//Threshold array Buffer for OpenCl
	cl_mem ClThresholdArrayBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(cl_uchar) * m_ImageHeight * m_ImageWidth, (void*)ThresholdArray, &cl_Error);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tBinarizeMultiColor: Could not create OpenCL Threshold Array Buffer: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}

	#pragma endregion

	#pragma region set Arguments and run Kernel

	//Set Arguments
	cl_Error = clSetKernelArg(ClColorBinarizeKernel, 0, sizeof(cl_mem), &ClSourceImageBuffer);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tBinarizeMultiColor: Could not set Argument 0 of OpenCL Kernel Color-Binarize: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}
	cl_Error = clSetKernelArg(ClColorBinarizeKernel, 1, sizeof(cl_mem), &ClDestImageBuffer);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tBinarizeMultiColor: Could not set Argument 1 of OpenCL Kernel Color-Binarize: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}
	cl_Error = clSetKernelArg(ClColorBinarizeKernel, 2, sizeof(cl_mem), &ClThresholdArrayBuffer);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tBinarizeMultiColor: Could not set Argument 2 of OpenCL Kernel Color-Binarize: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}
	cl_Error = clSetKernelArg(ClColorBinarizeKernel, 3, sizeof(cl_uint), (cl_uint*)(BinarizationMode ? (new UINT { 0 }) : &m_Threshold));
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tBinarizeMultiColor: Could not set Argument 3 of OpenCL Kernel Color-Binarize: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}

	//Run the Kernel
	size_t GlobalWorkSize1[] = { (size_t)((size_t)m_ImageWidth * (size_t)m_ImageHeight)};
	cl_Error = clEnqueueNDRangeKernel(queue, ClColorBinarizeKernel, 1, NULL, GlobalWorkSize1, NULL, 0, NULL, NULL);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tBinarizeMultiColor: Could not run OpenCL Kernel Color-Binarize: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}
	cl_Error = clFinish(queue);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tBinarizeMultiColor: Could not finish OpenCL Kernel Color-Binarize: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}

	#pragma endregion

	#pragma region Retrieve Buffers

	//Retrieve Buffers
	cl_Error = clEnqueueReadBuffer(queue, ClDestImageBuffer, CL_TRUE, 0, pSampleOut->GetActualDataLength(), (void*)pOutBuffer, 0, NULL, NULL);
	if (cl_Error != CL_SUCCESS)
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s((int)cl_Error, MsgMsg, 10, 10);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tBinarizeMultiColor: Could not read OpenCL Image Buffer: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		exit(-1);
	}

	#pragma endregion


	#pragma region Cleanup CL

	//Cleanup Buffers
	cl_Error = clReleaseMemObject(ClSourceImageBuffer);
	cl_Error = clReleaseMemObject(ClDestImageBuffer);
	cl_Error = clReleaseMemObject(ClThresholdArrayBuffer);

	#pragma endregion

	return NOERROR;
}


//Dilate/Erode

void C_rx_AlignFilter::CalculateErodeSeed()
{
	m_ElementE_Hori = getStructuringElement(MORPH_RECT,
		Size(m_ErodeSeedX, m_ErodeSeedY),
		Point(-1, -1));
	m_ElementE_Verti = getStructuringElement(MORPH_RECT,
		Size(m_ErodeSeedY, m_ErodeSeedX),
		Point(-1, -1));
}

HRESULT C_rx_AlignFilter::Erode(IMediaSample* pSampleIn, BOOL Vertical)
{
	//Erode algorithm

	CheckPointer(pSampleIn, E_POINTER);
	HRESULT hr = NOERROR;

	//Databuffer
	BYTE *pBufferIn;
	hr = pSampleIn->GetPointer(&pBufferIn);
	if (FAILED(hr))
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s(hr, MsgMsg, 10, 16);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tErode: Get Pointer Error: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		return E_POINTER;
	}


	#pragma region Dilate-Erode

	Mat m_SourceImage(m_ImageHeight, m_ImageWidth, CV_8UC4, pBufferIn);

	if (Vertical)
	{
		//Vertical Lines
		morphologyEx(m_SourceImage, m_SourceImage, MORPH_CLOSE, m_ElementE_Verti, Point(-1, -1), m_DilateErodes, BORDER_CONSTANT, morphologyDefaultBorderValue());
		erode(m_SourceImage, m_SourceImage, m_ElementE_Verti, Point(-1, -1), m_ExtraErodes, BORDER_CONSTANT, morphologyDefaultBorderValue());
	}
	else
	{
		//Horizontal Lines
		morphologyEx(m_SourceImage, m_SourceImage, MORPH_CLOSE, m_ElementE_Hori, Point(-1, -1), m_DilateErodes, BORDER_CONSTANT, morphologyDefaultBorderValue());
		erode(m_SourceImage, m_SourceImage, m_ElementE_Hori, Point(-1, -1), m_ExtraErodes, BORDER_CONSTANT, morphologyDefaultBorderValue());
	}

	m_SourceImage.release();

	#pragma endregion


	return hr;
}


//Blob

HRESULT C_rx_AlignFilter::Blob(IMediaSample* pSampleIn)
{
	//Blob algorithm

	CheckPointer(pSampleIn, E_POINTER);
	//CheckPointer(pSampleOut, E_POINTER);
	HRESULT hr = NOERROR;

	//Databuffer
	BYTE *pBufferIn;
	hr = pSampleIn->GetPointer(&pBufferIn);
	if (FAILED(hr))
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s(hr, MsgMsg, 10, 16);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tBlob: Get Pointer Error: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		return E_POINTER;
	}


	#pragma region contourArea

	Mat SourceMat(m_ImageHeight, m_ImageWidth, CV_8UC4, pBufferIn);
	Mat BlobMat;

	//Find Blobs
	cvtColor(SourceMat, BlobMat, COLOR_RGBA2GRAY);
	findContours(BlobMat, m_vContours, m_vHierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);

	//find the mass centers and bounding rectangles of each Blob
	m_vMoments.resize(m_vContours.size());
	m_vCenters.resize(m_vContours.size());
	m_vBoundRect.resize(m_vContours.size());
	m_vAngularRect.resize(m_vContours.size());
	vector<vector<Point>> contours_poly(m_vContours.size());

	for (int i = 0; i < m_vContours.size(); i++)
	{
		//Mass Centers
		m_vMoments[i] = moments(m_vContours[i]);
		m_vCenters[i] = Point((int)(m_vMoments[i].m10 / m_vMoments[i].m00), (int)(m_vMoments[i].m01 / m_vMoments[i].m00));
		//Bounding rectangles
		approxPolyDP(m_vContours[i], contours_poly[i], 3, true);
		m_vBoundRect[i] = boundingRect(contours_poly[i]);
		m_vAngularRect[i] = minAreaRect(contours_poly[i]);
	}

	//fill blobs with various colors, mark Mass centers and center line
	if (!m_ShowOriginal)
	{
		Scalar color(128, 0, 128);
		for (int i = 0; i < m_vContours.size(); i++)
		{
			//Fill Blob
			drawContours(SourceMat, m_vContours, i, color, FILLED, 8, m_vHierarchy, 0);
			color[0] = ((int)color[0] + 64) & 255;
			if (color[0] == 0) { color[1] = ((int)color[1] + 64) & 255; }
			if (color[1] == 0) { color[2] = ((int)color[2] + 64) & 255; }
			if (color[0] == 0 && color[1] == 0 && color[2] == 0) { color[0] = 64; }

			//Point2f Center = Point(Mom.m10 / Mom.m00, Mom.m01 / Mom.m00);
			drawMarker(SourceMat, m_vCenters[i], Scalar(GetBValue(m_CrossColorBlob), GetGValue(m_CrossColorBlob), GetRValue(m_CrossColorBlob)), 0, 20, 2, 8);

			//Calculate center-line
			Point2f Delta = m_vCenters[i] - m_vAngularRect[i].center;
			Point2f vertices[4];
			m_vAngularRect[i].points(vertices);

			//Mark center-line
			float blob_angle_deg = m_vAngularRect[i].angle;
			if (m_vAngularRect[i].size.width < m_vAngularRect[i].size.height)
			{
				blob_angle_deg = 90 + blob_angle_deg;
			}
			if ((blob_angle_deg > 0 && blob_angle_deg < 90) || (blob_angle_deg > 180 && blob_angle_deg < 270))
			{
				LineIterator LineIt(SourceMat, ((vertices[1] + vertices[2]) / 2) + Delta, ((vertices[3] + vertices[0]) / 2) + Delta, 8);
				for (int j = 0; j < LineIt.count; j++, LineIt++) if (j % 3 != 0) { (*LineIt)[0] = GetBValue(m_CrossColorBlob); (*LineIt)[1] = GetGValue(m_CrossColorBlob); (*LineIt)[2] = GetRValue(m_CrossColorBlob); }
			}
			else
			{
				LineIterator LineIt(SourceMat, ((vertices[0] + vertices[1]) / 2) + Delta, ((vertices[2] + vertices[3]) / 2) + Delta, 8);
				for (int j = 0; j < LineIt.count; j++, LineIt++) if (j % 3 != 0) { (*LineIt)[0] = GetBValue(m_CrossColorBlob); (*LineIt)[1] = GetGValue(m_CrossColorBlob); (*LineIt)[2] = GetRValue(m_CrossColorBlob); }
			}
		}
	}

	BlobMat.release();
	SourceMat.release();

	#pragma endregion

	return hr;
}


//Measurement

HRESULT C_rx_AlignFilter::BlobQualifyer(IMediaSample* pSampleIn, BOOL Vertical, IMediaSample* pProcessSample)
{
	CheckPointer(pSampleIn, E_POINTER);
	HRESULT hr = NOERROR;

	//Databuffer In
	BYTE* pBufferIn;
	hr = pProcessSample->GetPointer(&pBufferIn);
	if (FAILED(hr))
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s(hr, MsgMsg, 10, 16);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tBlobQualifyer: Get Pointer Error: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		return E_POINTER;
	}

	Mat SourceMat(m_ImageHeight, m_ImageWidth, CV_8UC4, pBufferIn);

	m_vQualifyList.clear();
	m_ValidMeasure = false;
	QualifyStruct QualifyerBlob;

	//Lines
	for (int BlobNo = 0; BlobNo < m_vContours.size(); BlobNo++)
	{
		QualifyerBlob.BlobNo = BlobNo;
		int Area = m_vBoundRect[BlobNo].width * m_vBoundRect[BlobNo].height;

		//Minimum size of Blob
		if ((uint)(Area / (m_ImageWidth * m_ImageHeight / 8000)) < m_BlobAreaDivisor)
		{
			//too small
			QualifyerBlob.Valid = 0;
		}
		else
		{
			//Rectify Angle and Dimensions
			if ((Vertical & (m_vAngularRect[BlobNo].size.width > m_vAngularRect[BlobNo].size.height)) |
				((!Vertical) & (m_vAngularRect[BlobNo].size.width < m_vAngularRect[BlobNo].size.height)))
			{
				//Angle
				QualifyerBlob.Angle = m_vAngularRect[BlobNo].angle + 90;
				//Dimension
				QualifyerBlob.Height = m_vAngularRect[BlobNo].size.width;
				QualifyerBlob.Width = m_vAngularRect[BlobNo].size.height;
			}
			else
			{
				//Angle
				QualifyerBlob.Angle = m_vAngularRect[BlobNo].angle;
				//Dimension
				QualifyerBlob.Height = m_vAngularRect[BlobNo].size.height;
				QualifyerBlob.Width = m_vAngularRect[BlobNo].size.width;
			}


			//Assume it's ok, then disqualify
			QualifyerBlob.Valid = 1;

			//Max / Min Angle
			if (QualifyerBlob.Angle > m_MaxBlobAngle || QualifyerBlob.Angle < -m_MaxBlobAngle)
			{
				//too skewed
				QualifyerBlob.Valid = 0;
			}
			else
			{
				//Minimum Aspect Ratio
				if ((Vertical & ((QualifyerBlob.Height / QualifyerBlob.Width) < m_BlobAspectLimit)) |
					((!Vertical) & ((QualifyerBlob.Width / QualifyerBlob.Height) < m_BlobAspectLimit)))
				{
					//not oblong enough
					QualifyerBlob.Valid = 0;
				}
				else
				{
					//Mass Center
					QualifyerBlob.PosX = m_vCenters[BlobNo].x;
					QualifyerBlob.PosY = m_vCenters[BlobNo].y;
				}
			}
		}

		m_vQualifyList.push_back(QualifyerBlob);
	}

	//Sort Blobs (LastUsed/NextUsed)
	if (Vertical)
	{
		//Sort from left to right
		std::sort(m_vQualifyList.begin(), m_vQualifyList.end(), CompareX);
	}
	else
	{
		//Sort from top to bottom
		std::sort(m_vQualifyList.begin(), m_vQualifyList.end(), CompareY);
	}

	//insert last/next used numbers
	UINT16 LastUsedBlob = 65535;
	for (int BlobNo = 0; BlobNo < m_vQualifyList.size(); BlobNo++)
	{
		m_vQualifyList[BlobNo].NextUsed = 65535;

		if (m_vQualifyList[BlobNo].Valid == 1)
		{
			m_vQualifyList[BlobNo].LastUsed = LastUsedBlob;
			if (LastUsedBlob < 65535)m_vQualifyList[LastUsedBlob].NextUsed = BlobNo;
			LastUsedBlob = BlobNo;
		}
		else
		{
			m_vQualifyList[BlobNo].LastUsed = 65535;
		}
	}

	CalculateDistances(Vertical, SourceMat);

	SourceMat.release();
	return NOERROR;
}

HRESULT C_rx_AlignFilter::CalculateDistances(BOOL Vertical, Mat SourceMat)
{
	//Calculate Distances between all Lines
	int NumBlobsvalid = 0;
	for (int BlobNo = 0; BlobNo < (int)m_vQualifyList.size(); BlobNo++)
	{
		m_vQualifyList[BlobNo].DistToLast = 0;
		m_vQualifyList[BlobNo].DistToNext = 0;

		//Only valid Blobs
		if (m_vQualifyList[BlobNo].Valid == 1)
		{
			NumBlobsvalid++;
			if (m_vQualifyList[BlobNo].LastUsed == 65535)
			{
				m_vQualifyList[BlobNo].DistToLast = 0;
			}
			else
			{
				//Distance perpendicular between two lines
				if (Vertical)
				{
					//Right Line
					double Angle_C = m_vQualifyList[BlobNo].Angle * (M_PI / 180); //rad, Angle of center line
					double Angle_T = m_vQualifyList[m_vQualifyList[BlobNo].LastUsed].Angle * (M_PI / 180); //rad, Angle of target line
					double dX = (double)m_vQualifyList[m_vQualifyList[BlobNo].LastUsed].PosX - (double)m_vQualifyList[BlobNo].PosX;
					double dY = (double)m_vQualifyList[BlobNo].PosY - (double)m_vQualifyList[m_vQualifyList[BlobNo].LastUsed].PosY;
					double Hypo = sqrt((dX * dX) + (dY * dY));	//Direct line between center points

					//perpendicular line from center to left
					double Gamma = atan(dY / dX);
					double Beta = (M_PI / 2) - Angle_T - Gamma;
					double Delta = M_PI - Beta - Gamma - Angle_C;
					double PerpenLine = (Hypo * sin(Beta)) / sin(Delta); //Line-Length Perpendicular from center to right

					m_vQualifyList[m_vQualifyList[BlobNo].LastUsed].DistToNext = (float)PerpenLine;
					m_vQualifyList[BlobNo].DistToLast = (float)PerpenLine;
				}
				else //Horizontal
				{
					//Top Line
					double Angle_C = m_vQualifyList[BlobNo].Angle * (M_PI / 180); //rad, Angle of center line
					double Angle_T = m_vQualifyList[m_vQualifyList[BlobNo].LastUsed].Angle * (M_PI / 180); //rad, Angle of target line
					double dX = (double)m_vQualifyList[m_vQualifyList[BlobNo].LastUsed].PosX - (double)m_vQualifyList[BlobNo].PosX;
					double dY = (double)m_vQualifyList[m_vQualifyList[BlobNo].LastUsed].PosY - (double)m_vQualifyList[BlobNo].PosY;
					double Hypo = sqrt((dX * dX) + (dY * dY));	//Direct line between center points

					//perpendicular line from center to Top
					double Gamma = atan(dX / dY);
					double Beta = (M_PI / 2) - Gamma - Angle_C;
					double Delta = (M_PI / 2) - Gamma - Angle_T;
					double Epsilon = Angle_T - Angle_C + (M_PI / 2);
					double PerpenLine = (Hypo * sin(Delta)) / sin(Epsilon); //Line-Length Perpendicular from center to left

					m_vQualifyList[m_vQualifyList[BlobNo].LastUsed].DistToNext = (float)PerpenLine;
					m_vQualifyList[BlobNo].DistToLast = (float)PerpenLine;
				}

				if (m_vQualifyList[BlobNo].NextUsed == 65535) m_vQualifyList[BlobNo].DistToNext = 0;
			}
		}
	}

	//Select Lines to display Values
	switch (m_MeasureMode)
	{
	case MeasureMode_Off:
		//No Lines are displayed
		for (int BlobNo = 0; BlobNo < m_vQualifyList.size(); BlobNo++)
		{
			m_vQualifyList[BlobNo].Display = 0;
			m_ValidMeasure = false;
		}
		break;
	case MeasureMode_AllLines:		//All valid lines are displayable
	case MeasureMode_StartLines:
	case MeasureMode_Angle:
	case MeasureMode_Stitch:
	case MeasureMode_Register:
		//All Valid Lises are displayable
		for (int BlobNo = 0; BlobNo < m_vQualifyList.size(); BlobNo++)
		{
			m_vQualifyList[BlobNo].Display = m_vQualifyList[BlobNo].Valid;
			m_ValidMeasure = true;
		}
		break;
	}
	
	return NOERROR;
}

HRESULT C_rx_AlignFilter::FindStartLines(BOOL Vertical, BOOL UpsideDown)
{
    //Check for first occurence of parallel lines

    //Check minimum number of valid lines
    int NumBlobsvalid = 0;
    float AvgLength = 0;
    float AvgCenter = 0;
    float AvgDist = 0;
    for (int BlobNo = 0; BlobNo < (int)m_vQualifyList.size(); BlobNo++)
    {
        if (m_vQualifyList[BlobNo].Valid == 1)
        {
			//Position in Window
			float Center = Vertical ? m_vQualifyList[BlobNo].PosY : m_vQualifyList[BlobNo].PosX;
			AvgLength += Vertical ? m_vQualifyList[BlobNo].Height : m_vQualifyList[BlobNo].Width;
			if (UpsideDown)
			{
				AvgCenter += Vertical ? (m_ImageHeight - Center) : (m_ImageWidth - Center);
			}
			else
			{
				AvgCenter += Center;
			}
			if(NumBlobsvalid > 0) AvgDist += m_vQualifyList[BlobNo].DistToLast;

			NumBlobsvalid++;
		}
    }
    if(NumBlobsvalid < m_MinNumStartLines) return NOERROR;

    AvgDist /= (float)NumBlobsvalid - 1;
    AvgCenter /= (float)NumBlobsvalid;
    AvgLength /= (float)NumBlobsvalid;

	//LParam =	Top/Right: 0x10000000, Covering: 0x20000000, Bottom/Left: 0x30000000
	//			NumLines: 0x0n000000 (0x0 - 0xf)
	//			Position: 0x00nnnnnn (0x000000 - 0xffffff) = (StartPos[px] / AvgDist[px]) * 1000 (from Bottom/left)
	long lpara = 0;
    float StartPos = 0;

	if (AvgLength > (Vertical ? ((float)m_ImageHeight * 0.95) : ((float)m_ImageWidth * 0.95)))
	{
		//Covering whole image
		lpara = 0x20000000;
	}
	else if (AvgCenter < (Vertical ? ((float)m_ImageHeight * 0.5) : ((float)m_ImageWidth * 0.5)))
	{
		//Entering from bottom
		StartPos = AvgCenter + (AvgLength / 2);
		lpara = 0x30000000;
	}
	else
	{
		//Entering from top
		StartPos = AvgCenter - (AvgLength / 2);
		lpara = 0x10000000;
	}

    lpara += NumBlobsvalid * 0x1000000;
    lpara += (StartPos == 0 ? 0 : (int)((StartPos / AvgDist) * 1000));

	m_PresetMeasureMode = MeasureModeEnum::MeasureMode_Off;
	if (m_HostHwnd != NULL) SendMessage(m_HostHwnd, WM_APP_ALIGNEV, (WPARAM)WP_StartLines, (LPARAM)lpara);

    return NOERROR;
}

HRESULT C_rx_AlignFilter::MeasureAngle(BOOL Vertical, BOOL UpsideDown)
{
	//Angle from FullAlignment (3 lines, 31px/64px)

	//Only 3 lines accepted
	int NumBlobsValid = 0;
	for (int BlobNo = 0; BlobNo < (int)m_vQualifyList.size(); BlobNo++)
	{
		if (m_vQualifyList[BlobNo].Valid == 1) NumBlobsValid++;
	}
	if (NumBlobsValid != 3)
	{
		for (int BlobNo = 0; BlobNo < (int)m_vQualifyList.size(); BlobNo++)
		{
			m_vQualifyList[BlobNo].Display = 0;
		}
		return NOERROR;
	}

	//Find the three valid blob numbers
	int BlobNums[3];
	int BlobCount = 0;
	for (int BlobNo = 0; (BlobNo < (int)m_vQualifyList.size()) && (BlobCount < 3); BlobNo++)
	{
		m_vQualifyList[BlobNo].Display = m_vQualifyList[BlobNo].Valid;
		if (m_vQualifyList[BlobNo].Valid == 1)
		{
			BlobNums[BlobCount++] = BlobNo;
		}
	}
	m_ValidMeasure = true;

	//Distance between outer lines / measured camera-pixels = μm per camera-pixel
	float umPpx = (float)m_AngleOuterDistance / (m_vQualifyList[BlobNums[0]].DistToNext + m_vQualifyList[BlobNums[1]].DistToNext);
	//Pattern-position vs Image-center
	float DPosX = ((m_vQualifyList[BlobNums[0]].PosX + m_vQualifyList[BlobNums[2]].PosX) / 2) - (m_ImageWidth / 2);
	float DPosY = ((m_vQualifyList[BlobNums[0]].PosY + m_vQualifyList[BlobNums[2]].PosY) / 2) - (m_ImageHeight / 2);

	//Calculate angular deviation
	double MeasuredRatio = 0;
	if (UpsideDown)
	{
		MeasuredRatio = (m_vQualifyList[BlobNums[0]].DistToNext + m_vQualifyList[BlobNums[1]].DistToNext) / m_vQualifyList[BlobNums[0]].DistToNext;
	}
	else
	{
		MeasuredRatio = (m_vQualifyList[BlobNums[0]].DistToNext + m_vQualifyList[BlobNums[1]].DistToNext) / m_vQualifyList[BlobNums[1]].DistToNext;
	}
	double b1 = m_AngleSideB / MeasuredRatio;
	double b2 = m_AngleSideB - b1;
	double BD = sqrt((b1 * b1) + (m_AngleSideC * m_AngleSideC) - (2 * b1 * m_AngleSideC * cos(m_AngleAlpha)));
	double Beta1 = acos(((m_AngleSideC * m_AngleSideC) + (BD * BD) - (b1 * b1)) / (2 * m_AngleSideC * BD));
	double Beta2 = acos(((m_AngleSideA * m_AngleSideA) + (BD * BD) - (b2 * b2)) / (2 * m_AngleSideA * BD));
	double DeltaBeta1 = m_AngleBeta1 - Beta1;
	double DeltaBeta2 = Beta2 - m_AngleBeta2;
	double DeltaAngle = (DeltaBeta1 + DeltaBeta2) / 2;
	m_vQualifyList[BlobNums[0]].AngleCorrection = DeltaAngle / m_AngleDegreesPerRev;

	if (m_MeasureRunning)
	{
		MeasureDataStruct MeasureData;
		MeasureData.Correction = m_vQualifyList[BlobNums[0]].AngleCorrection;
		MeasureData.DPosX = DPosX;
		MeasureData.DPosY = DPosY;
		m_vMeasureDataList.push_back(MeasureData);

		if (m_DebugOn)
		{
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tAngle measure: #%d\n", (float)m_TimeStamp / 1000000.0f, (int)m_vMeasureDataList.size());
		}

		if (--m_NumMeasures <= 0)
		{
			m_MeasureRunning = false;
			m_measureDone = true;
		}
	}

	return NOERROR;
}

HRESULT C_rx_AlignFilter::MeasureStitch(BOOL Vertical, BOOL UpsideDown)
{
	//Stitch from FullAlignment (3 lines, 27px/54px)

	//Only 3 lines accepted
	int NumBlobsValid = 0;
	for (int BlobNo = 0; BlobNo < (int)m_vQualifyList.size(); BlobNo++)
	{
		if (m_vQualifyList[BlobNo].Valid == 1) NumBlobsValid++;
	}
	if (NumBlobsValid != 3)
	{
		for (int BlobNo = 0; BlobNo < (int)m_vQualifyList.size(); BlobNo++)
		{
			m_vQualifyList[BlobNo].Display = 0;
		}
		return NOERROR;
	}

	//Find the three valid blob numbers
	int BlobNums[3];
	int BlobCount = 0;
	for (int BlobNo = 0; (BlobNo < (int)m_vQualifyList.size()) && (BlobCount < 3); BlobNo++)
	{
		m_vQualifyList[BlobNo].Display = m_vQualifyList[BlobNo].Valid;
		if (m_vQualifyList[BlobNo].Valid == 1)
		{
			BlobNums[BlobCount++] = BlobNo;
		}
	}
	m_ValidMeasure = true;

	//Distance between outer lines / measured camera-pixels = μm per camera-pixel
	float umPpx = m_StitchOuterDistance / (m_vQualifyList[BlobNums[0]].DistToNext + m_vQualifyList[BlobNums[1]].DistToNext);
	//Pattern-position vs Image-center
	float DPosX = ((m_vQualifyList[BlobNums[0]].PosX + m_vQualifyList[BlobNums[2]].PosX) / 2) - (m_ImageWidth / 2);
	float DPosY = ((m_vQualifyList[BlobNums[0]].PosY + m_vQualifyList[BlobNums[2]].PosY) / 2) - (m_ImageHeight / 2);

	//Deviation of middle line
	float StitchDeviationBot = m_StitchOuterDistance - m_StitchTargetDistance - (umPpx * m_vQualifyList[BlobNums[0]].DistToNext);
	float StitchDeviationTop = m_StitchTargetDistance - (umPpx * m_vQualifyList[BlobNums[1]].DistToNext);
	float StitchDeviationAvg = 0;
	if (UpsideDown)
	{
		StitchDeviationAvg = (StitchDeviationTop - StitchDeviationBot) / 2;
	}
	else
	{
		StitchDeviationAvg = (StitchDeviationBot - StitchDeviationTop) / 2;
	}
	m_vQualifyList[BlobNums[0]].StitchCorr = StitchDeviationAvg / m_StitchMicronsPerRev;

	if (m_MeasureRunning)
	{
		MeasureDataStruct MeasureData;
		MeasureData.Correction = m_vQualifyList[BlobNums[0]].StitchCorr;
		MeasureData.DPosX = DPosX;
		MeasureData.DPosY = DPosY;
		m_vMeasureDataList.push_back(MeasureData);

		if (m_DebugOn)
		{
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tStitch measure: #%d\n", (float)m_TimeStamp / 1000000.0f, (int)m_vMeasureDataList.size());
		}

		if (--m_NumMeasures <= 0)
		{
			m_MeasureRunning = false;
			m_measureDone = true;
		}
	}
	return NOERROR;
}

HRESULT C_rx_AlignFilter::MeasureRegister(BOOL Vertical, BOOL UpsideDown)
{
	//Register from FullAlignment (3 lines, 27px/54px)

	//Only 3 lines accepted
	int NumBlobsValid = 0;
	for (int BlobNo = 0; BlobNo < (int)m_vQualifyList.size(); BlobNo++)
	{
		if (m_vQualifyList[BlobNo].Valid == 1) NumBlobsValid++;
	}
	if (NumBlobsValid != 3)
	{
		for (int BlobNo = 0; BlobNo < (int)m_vQualifyList.size(); BlobNo++)
		{
			m_vQualifyList[BlobNo].Display = 0;
		}
		return NOERROR;
	}

	//Find the three valid blob numbers
	int BlobNums[3];
	int BlobCount = 0;
	for (int BlobNo = 0; (BlobNo < (int)m_vQualifyList.size()) && (BlobCount < 3); BlobNo++)
	{
		m_vQualifyList[BlobNo].Display = m_vQualifyList[BlobNo].Valid;
		if (m_vQualifyList[BlobNo].Valid == 1)
		{
			BlobNums[BlobCount++] = BlobNo;
		}
	}
	m_ValidMeasure = true;

	//Distance between outer lines / measured camera-pixels = μm per camera-pixel
	float umPpx = m_RegisterOuterDistance / (m_vQualifyList[BlobNums[0]].DistToNext + m_vQualifyList[BlobNums[1]].DistToNext);
	//Pattern-position vs Image-center
	float DPosX = ((m_vQualifyList[BlobNums[0]].PosX + m_vQualifyList[BlobNums[2]].PosX) / 2) - (m_ImageWidth / 2);
	float DPosY = ((m_vQualifyList[BlobNums[0]].PosY + m_vQualifyList[BlobNums[2]].PosY) / 2) - (m_ImageHeight / 2);

	//Deviation of middle line
	float RegisterDeviationBot = m_RegisterOuterDistance - m_RegisterTargetDistance - (umPpx * m_vQualifyList[BlobNums[0]].DistToNext);
	float RegisterDeviationTop = m_RegisterTargetDistance - (umPpx * m_vQualifyList[BlobNums[1]].DistToNext);
	float RegisterDeviationAvg = 0;
	if (UpsideDown)
	{
		RegisterDeviationAvg = (RegisterDeviationTop - RegisterDeviationBot) / 2;
	}
	else
	{
		RegisterDeviationAvg = (RegisterDeviationBot - RegisterDeviationTop) / 2;
	}

	if (!Vertical)m_vQualifyList[BlobNums[0]].RegisterCorr = -RegisterDeviationAvg;
	else m_vQualifyList[BlobNums[0]].RegisterCorr = RegisterDeviationAvg;

	if (m_MeasureRunning)
	{
		MeasureDataStruct MeasureData;
		MeasureData.Correction = m_vQualifyList[BlobNums[0]].RegisterCorr;
		MeasureData.DPosX = DPosX;
		MeasureData.DPosY = DPosY;
		m_vMeasureDataList.push_back(MeasureData);

		if (m_DebugOn)
		{
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tRegister measure: #%d\n", (float)m_TimeStamp / 1000000.0f, (int)m_vMeasureDataList.size());
		}

		if (--m_NumMeasures <= 0)
		{
			m_MeasureRunning = false;
			m_measureDone = true;
		}
	}

	return NOERROR;
}


//Display

HRESULT C_rx_AlignFilter::OverlayBlobs(IMediaSample* pSampleIn)
{
	//OverlayBlobs

	CheckPointer(pSampleIn, E_POINTER);
	HRESULT hr = NOERROR;

	//Databuffer In
	BYTE *pBufferIn;
	hr = pSampleIn->GetPointer(&pBufferIn);
	if (FAILED(hr))
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s(hr, MsgMsg, 10, 16);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tOverlayBlobs: Get Pointer Error: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		return E_POINTER;
	}

	Mat SourceMat(m_ImageHeight, m_ImageWidth, CV_8UC4, pBufferIn);

	//Draw Blob outlines for all valid Blobs
	for (int BlobNo = 0; BlobNo < m_vQualifyList.size(); BlobNo++)
	{
		if (m_vQualifyList[BlobNo].Valid == 1)
		{
			int LineThick = 2;
			cv::Point OffsetPoint; OffsetPoint.x = -LineThick; OffsetPoint.y = -LineThick;

			//Line Blobs
			drawContours(SourceMat, m_vContours, m_vQualifyList[BlobNo].BlobNo, Scalar(GetBValue(m_BlobColor), GetGValue(m_BlobColor), GetRValue(m_BlobColor)), 2 * LineThick, 8, m_vHierarchy, 0, OffsetPoint);
		}
	}

	SourceMat.release();

	return hr;
}

HRESULT C_rx_AlignFilter::OverlayCenters(IMediaSample* pSampleIn, BOOL Vertical)
{
	//OverlayCenters

	CheckPointer(pSampleIn, E_POINTER);
	HRESULT hr = NOERROR;

	//Databuffer In
	BYTE *pBufferIn;
	hr = pSampleIn->GetPointer(&pBufferIn);
	if (FAILED(hr))
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s(hr, MsgMsg, 10, 16);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tOverlayCenters: Get Pointer Error: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		return E_POINTER;
	}

	Mat SourceMat(m_ImageHeight, m_ImageWidth, CV_8UC4, pBufferIn);

	//Draw Center Crosses for all displayable Blobs
	for (int BlobNo = 0; BlobNo < m_vQualifyList.size(); BlobNo++)
	{
		if (m_vQualifyList[BlobNo].Display >= 1)
		{
			//Draw Mass-Center Crosses
			drawMarker(SourceMat, m_vCenters[m_vQualifyList[BlobNo].BlobNo], Scalar(GetBValue(m_CrossColor), GetGValue(m_CrossColor), GetRValue(m_CrossColor)), MARKER_CROSS, 20, 2, 8);
		}
	}

	SourceMat.release();

	return hr;
}

HRESULT C_rx_AlignFilter::OverlayValues(IMediaSample* pSampleIn, BOOL Vertical)
{
	//OverlayValues

	CheckPointer(pSampleIn, E_POINTER);
	HRESULT hr = NOERROR;

	//Databuffer In
	BYTE *pBufferIn;
	hr = pSampleIn->GetPointer(&pBufferIn);
	if (FAILED(hr))
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s(hr, MsgMsg, 10, 16);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tOverlayValues: Get Pointer Error: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		return E_POINTER;
	}

	//Create Text-Overlay Bitmap
	if (!OverlayBitmapReady)
	{
		//BMP Info Header
		BITMAPINFOHEADER bmpIH;
		std::memset(&bmpIH, 0, sizeof(BITMAPINFOHEADER));
		bmpIH.biSize = sizeof(BITMAPINFOHEADER);
		bmpIH.biWidth = m_ImageWidth;
		bmpIH.biHeight = m_ImageHeight;
		bmpIH.biPlanes = 1;
		bmpIH.biBitCount = 32;
		bmpIH.biCompression = 0;
		bmpIH.biSizeImage = m_ImageWidth * m_ImageHeight * 32;
		bmpIH.biXPelsPerMeter = 0;
		bmpIH.biYPelsPerMeter = 0;
		bmpIH.biClrUsed = 0;
		bmpIH.biClrImportant = 0;
		//Bmp Info
		BITMAPINFO bmpInfo;
		std::memset(&bmpInfo, 0, sizeof(BITMAPINFO));
		bmpInfo.bmiHeader = bmpIH;

		//TextBitmap
		MemDC = CreateCompatibleDC(NULL);
		SetTextAlign(MemDC, TA_LEFT || TA_TOP || TA_NOUPDATECP);
		SetBkMode(MemDC, TRANSPARENT);
		//BYTE *pText;
		TextBitmap = CreateDIBSection(MemDC, &bmpInfo, DIB_RGB_COLORS, (void**)&pText, NULL, NULL);
		if (TextBitmap == NULL)
		{
			if (m_DebugOn)
			{
				m_MeasureTime = std::chrono::steady_clock::now();
				m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
				printf("%6.6f\tOverlayValues: Could not create TextBitmap\n", (float)m_TimeStamp / 1000000.0f);
			}
			exit(-1);
		}
		OverlayBitmapReady = true;
	}

	HGDIOBJ TmpObj = SelectObject(MemDC, TextBitmap);

	//SetTextColor
	HFONT OldFont = (HFONT)SelectObject(MemDC, m_TextFont);
	SetTextColor(MemDC, m_TextColor);

	//Copy Buffer
	long BufferLen = pSampleIn->GetActualDataLength();
	std::memcpy(pText, pBufferIn, BufferLen);

	//Insert selected Values
	if (m_MeasureMode != MeasureMode_Off)
	{
		//Display Text String
		wchar_t Text[70];
		for (int t = 0; t < 70; t++) { Text[t] = '\0'; };

		switch (m_DisplayMode)
		{
		case Display_AllLines:
			for (int BlobNo = 0; BlobNo < (int)m_vQualifyList.size(); BlobNo++)
			{
				if (m_vQualifyList[BlobNo].Valid == 1 && m_vQualifyList[BlobNo].Display == 1)
				{
					if (m_vQualifyList[BlobNo].NextUsed != 65535)
					{
						RECT txtRect;
						txtRect.bottom = m_ImageHeight - (int)m_vQualifyList[m_vQualifyList[BlobNo].NextUsed].PosY;
						txtRect.top = m_ImageHeight - (int)m_vQualifyList[BlobNo].PosY;
						txtRect.right = (int)((m_vQualifyList[BlobNo].PosX + m_vQualifyList[m_vQualifyList[BlobNo].NextUsed].PosX) / 2);
						txtRect.left = txtRect.right;
						//Display Text String
						swprintf_s(Text, 69, TEXT("%.1f px"), m_vQualifyList[BlobNo].DistToNext);
						//draw text
						DrawText(MemDC, Text, -1, &txtRect, DT_NOCLIP | DT_CENTER);
					}
				}
			}
			break;

		case Display_Correction:

			//Find the three valid blob numbers
			int BlobNums[3];
			int BlobCount = 0;
			for (int BlobNo = 0; (BlobNo < (int)m_vQualifyList.size()) && (BlobCount < 3); BlobNo++)
			{
				if (m_vQualifyList[BlobNo].Valid == 1 && m_vQualifyList[BlobNo].Display == 1)
				{
					BlobNums[BlobCount++] = BlobNo;
				}
			}
			if (BlobCount != 3) break;

			//Text Rectangle
			RECT txtRect;
			txtRect.bottom = m_ImageHeight - (int)m_vQualifyList[BlobNums[1]].PosY;
			txtRect.top = txtRect.bottom;
			txtRect.right = (int)m_vQualifyList[BlobNums[1]].PosX;
			txtRect.left = txtRect.right;

			//If Text is outside image, put it to center
			if ((txtRect.right == 0) | (txtRect.bottom == 0))
			{
				txtRect.bottom = m_ImageHeight / 2;
				txtRect.top = txtRect.bottom;
				txtRect.right = m_ImageWidth / 2;
				txtRect.left = txtRect.right;
			}

			switch (m_MeasureMode)
			{
			case MeasureMode_AllLines:
				swprintf_s(Text, 69, TEXT("Distances [px] %.1f, %.1f"),
					m_vQualifyList[BlobNums[0]].DistToNext,
					m_vQualifyList[BlobNums[1]].DistToNext);
				break;

			case MeasureMode_StartLines:
				//No Display
				break;

			case MeasureMode_Angle:
				swprintf_s(Text, 69, TEXT("Angle %.1f Rev"), m_vQualifyList[BlobNums[0]].AngleCorrection);
				break;
			case MeasureMode_Stitch:
				swprintf_s(Text, 69, TEXT("Stitch %.1f Rev"), m_vQualifyList[BlobNums[0]].StitchCorr);
				break;
			case MeasureMode_Register:
				swprintf_s(Text, 69, TEXT("Register %.3f mm"), (m_vQualifyList[BlobNums[0]].RegisterCorr / 1000));
				break;
			}

			//draw text to Image
			DrawText(MemDC, Text, -1, &txtRect, DT_NOCLIP | DT_CENTER);
			break;
		}

	}

	
	//Overlay String
	if (TryEnterCriticalSection(&m_OvTextLock))
	{
		if (m_OverlayTxtReady)
		{
			SIZE s;
			GetTextExtentPoint32(MemDC, m_OverlayTxt, (int)wcslen(m_OverlayTxt), &s);
			RECT txtRect;
			txtRect.bottom = 50;
			txtRect.top = m_ImageHeight - (s.cy) - 10;
			txtRect.right = m_ImageWidth;
			txtRect.left = 10;
			SetTextColor(MemDC, m_OverlayTxtColor);
			DrawText(MemDC, m_OverlayTxt, -1, &txtRect, DT_NOCLIP/* | DT_LEFT | DT_BOTTOM | DT_SINGLELINE*/);
		}
		LeaveCriticalSection(&m_OvTextLock);
	}

	std::memcpy(pBufferIn, pText, BufferLen);
	SelectObject(MemDC, TmpObj);
	SelectObject(MemDC, TmpObj);
	DeleteObject(TmpObj);

	return hr;
}

HRESULT C_rx_AlignFilter::OverlayDistanceLines(IMediaSample* pSampleIn, BOOL Vertical)
{
	//OverlayDistances

	CheckPointer(pSampleIn, E_POINTER);
	HRESULT hr = NOERROR;

	//Databuffer In
	BYTE* pBufferIn;
	hr = pSampleIn->GetPointer(&pBufferIn);
	if (FAILED(hr))
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s(hr, MsgMsg, 10, 16);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tOverlayDistanceLines: Get Pointer Error: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		return E_POINTER;
	}

	Mat SourceMat(m_ImageHeight, m_ImageWidth, CV_8UC4, pBufferIn);

	//Draw perpendicular Lines for all displayable Blobs
	for (int BlobNo = 0; BlobNo < m_vQualifyList.size(); BlobNo++)
	{
		if ((m_vQualifyList[BlobNo].Display == 1) & (m_vQualifyList[BlobNo].DistToLast != 0) & (m_vQualifyList[BlobNo].DistToNext != 0))
		{
			if (Vertical)
			{
				//Left Line
				double Angle_C = m_vQualifyList[BlobNo].Angle * (M_PI / 180); //rad, Angle of center line
				Point2f Start = m_vCenters[m_vQualifyList[BlobNo].BlobNo]; //Start-Point, cetnter of center line
				Point2f End;
				End.y = Start.y - (float)((double)m_vQualifyList[BlobNo].DistToNext * sin(Angle_C));
				End.x = Start.x - (float)((double)m_vQualifyList[BlobNo].DistToNext * cos(Angle_C));

				LineIterator LineIt(SourceMat, Start, End, 8);
				for (int j = 0; j < LineIt.count; j++, LineIt++) if (j % 3 != 0) { (*LineIt)[0] = GetBValue(m_CrossColor); (*LineIt)[1] = GetGValue(m_CrossColor); (*LineIt)[2] = GetRValue(m_CrossColor); }

				//Right Line
				End.y = Start.y + (float)((double)m_vQualifyList[BlobNo].DistToLast * sin(Angle_C));
				End.x = Start.x + (float)((double)m_vQualifyList[BlobNo].DistToLast * cos(Angle_C));

				LineIt = LineIterator(SourceMat, Start, End, 8);
				for (int j = 0; j < LineIt.count; j++, LineIt++) if (j % 3 != 0) { (*LineIt)[0] = GetBValue(m_CrossColor); (*LineIt)[1] = GetGValue(m_CrossColor); (*LineIt)[2] = GetRValue(m_CrossColor); }
			}
			else
			{
				if (((double)m_vQualifyList[BlobNo].PosY < (double)m_vQualifyList[m_vQualifyList[BlobNo].LastUsed].PosY) &
					((double)m_vQualifyList[BlobNo].PosY > (double)m_vQualifyList[m_vQualifyList[BlobNo].NextUsed].PosY))
				{
					//Current Blob is middel Line

					//Top Line
					double Angle_C = m_vQualifyList[BlobNo].Angle * (M_PI / 180); //rad, Angle of center line
					Point2f Start = m_vCenters[m_vQualifyList[BlobNo].BlobNo]; //Start-Point, cetnter of center line
					Point2f End;
					End.y = Start.y + (float)((double)m_vQualifyList[BlobNo].DistToLast * cos(Angle_C));
					End.x = Start.x - (float)((double)m_vQualifyList[BlobNo].DistToLast * sin(Angle_C));

					LineIterator LineIt(SourceMat, Start, End, 8);
					for (int j = 0; j < LineIt.count; j++, LineIt++) if (j % 3 != 0) { (*LineIt)[0] = GetBValue(m_CrossColor); (*LineIt)[1] = GetGValue(m_CrossColor); (*LineIt)[2] = GetRValue(m_CrossColor); }

					//Bottom Line
					End.y = Start.y - (float)((double)m_vQualifyList[BlobNo].DistToNext * cos(Angle_C));
					End.x = Start.x + (float)((double)m_vQualifyList[BlobNo].DistToNext * sin(Angle_C));

					LineIt = LineIterator(SourceMat, Start, End, 8);
					for (int j = 0; j < LineIt.count; j++, LineIt++) if (j % 3 != 0) { (*LineIt)[0] = GetBValue(m_CrossColor); (*LineIt)[1] = GetGValue(m_CrossColor); (*LineIt)[2] = GetRValue(m_CrossColor); }
				}
			}
		}
	}

	SourceMat.release();

	return hr;
}


//SnapShot

HRESULT C_rx_AlignFilter::TakeSnapShot(IMediaSample* pSampleIn)
{
	CheckPointer(pSampleIn, E_POINTER);
	HRESULT hr = NOERROR;

	//Databuffer In
	BYTE *pBufferIn;
	hr = pSampleIn->GetPointer(&pBufferIn);
	if (FAILED(hr))
	{
		if (m_DebugOn)
		{
			TCHAR MsgMsg[10];
			_itow_s(hr, MsgMsg, 10, 16);
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tTakeSnapShot: Get Pointer InBuffer Error: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		return E_POINTER;
	}

	Mat SourceMat(m_ImageHeight, m_ImageWidth, CV_8UC4, pBufferIn);
	flip(SourceMat, SourceMat, 0);

	char string_SavePathName[MAX_PATH];
	sprintf_s(string_SavePathName, MAX_PATH, "%ls", m_SavePathName);

	vector<int> compression_params;
	compression_params.push_back(IMWRITE_PNG_COMPRESSION);
	compression_params.push_back(0);

	if (!imwrite(string_SavePathName, SourceMat))
	{
		if (m_DebugOn)
		{
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tTakeSnapShot: Image could not be saved\n", (float)m_TimeStamp / 1000000.0f);
		}
	}

	m_TakeSnapShot = FALSE;

	SourceMat.release();

	return NOERROR;
}


#pragma endregion

#pragma region Register DLL

// DllRegisterServer
__control_entrypoint(DllExport)
STDAPI DllRegisterServer(void)
{
	printf("Register filter\n");

	return AMovieDllRegisterServer2(TRUE);
}

// DllUnregisterServer
__control_entrypoint(DllExport)
STDAPI DllUnregisterServer(void)
{
	return AMovieDllRegisterServer2(FALSE);

	printf("Unregister filter\n");
}

#pragma endregion

#pragma region InputPin

// Crx_AlignFilter_InputPin constructor
C_rx_AlignFilter_InputPin::C_rx_AlignFilter_InputPin(TCHAR *pName, C_rx_AlignFilter *prx_AlignFilter, HRESULT *phr, LPCWSTR pPinName) :
	CBaseInputPin(pName, prx_AlignFilter, prx_AlignFilter, phr, pPinName),
	m_prx_AlignFilter(prx_AlignFilter),
	m_bInsideCheckMediaType(false)
{
	ASSERT(prx_AlignFilter);
	PinName = pPinName;

	printf("Filter create Input Pin\n");
}

// CheckMediaType
HRESULT C_rx_AlignFilter_InputPin::CheckMediaType(const CMediaType *pmt)
{
	CAutoLock lock_it(m_pLock);

	if (m_prx_AlignFilter->m_DebugOn)
	{
		m_prx_AlignFilter->m_MeasureTime = std::chrono::steady_clock::now();
		m_prx_AlignFilter->m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_prx_AlignFilter->m_MeasureTime - m_prx_AlignFilter->m_DebugStartTime).count();
		printf("%6.6f\tCheck Media Type\n", (float)m_prx_AlignFilter->m_TimeStamp / 1000000.0f);
	}

	if (m_bInsideCheckMediaType) return NOERROR;
	m_bInsideCheckMediaType = true;

	if (IsEqualGUID(*pmt->Type(), MEDIATYPE_Video))
	{
		if (IsEqualGUID(*pmt->Subtype(), MEDIASUBTYPE_RGB32))
		{
			m_bInsideCheckMediaType = false;
			return NOERROR;
		}
	}
	m_bInsideCheckMediaType = false;
	return VFW_E_TYPE_NOT_ACCEPTED;

	if (m_prx_AlignFilter->m_DebugOn)
	{
		m_prx_AlignFilter->m_MeasureTime = std::chrono::steady_clock::now();
		m_prx_AlignFilter->m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_prx_AlignFilter->m_MeasureTime - m_prx_AlignFilter->m_DebugStartTime).count();
		printf("%6.6f\tInputPin: CheckMediaType Input Unknown MediaSubtype\n", (float)m_prx_AlignFilter->m_TimeStamp / 1000000.0f);
	}
	m_bInsideCheckMediaType = false;
	return E_FAIL;

} // CheckMediaType

// SetMediaType
HRESULT C_rx_AlignFilter_InputPin::SetMediaType(const CMediaType *pmt)
{
	CAutoLock lock_it(m_pLock);
	HRESULT hr = NOERROR;

	if (m_prx_AlignFilter->m_DebugOn)
	{
		m_prx_AlignFilter->m_MeasureTime = std::chrono::steady_clock::now();
		m_prx_AlignFilter->m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_prx_AlignFilter->m_MeasureTime - m_prx_AlignFilter->m_DebugStartTime).count();
		printf("%6.6f\tSet Media Type\n", (float)m_prx_AlignFilter->m_TimeStamp / 1000000.0f);
	}

	// Make sure that the base class likes it
	hr = C_rx_AlignFilter_InputPin::CBaseInputPin::SetMediaType(pmt);
	if (FAILED(hr)) return hr;

	m_prx_AlignFilter->m_mt1 = *pmt;

	ASSERT(m_Connected != NULL);
	return NOERROR;

} // SetMediaType

// BreakConnect
HRESULT C_rx_AlignFilter_InputPin::BreakConnect()
{
	if (m_prx_AlignFilter->m_DebugOn)
	{
		m_prx_AlignFilter->m_MeasureTime = std::chrono::steady_clock::now();
		m_prx_AlignFilter->m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_prx_AlignFilter->m_MeasureTime - m_prx_AlignFilter->m_DebugStartTime).count();
		printf("%6.6f\tBreak Connect\n", (float)m_prx_AlignFilter->m_TimeStamp / 1000000.0f);
	}

	// Release any allocator that we are holding
	if (m_prx_AlignFilter->m_pAllocator)
	{
		m_prx_AlignFilter->m_pAllocator->Release();
		m_prx_AlignFilter->m_pAllocator = NULL;

        if (m_prx_AlignFilter->m_DebugOn)
        {
            m_prx_AlignFilter->m_MeasureTime = std::chrono::steady_clock::now();
            m_prx_AlignFilter->m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_prx_AlignFilter->m_MeasureTime - m_prx_AlignFilter->m_DebugStartTime).count();
            printf("%6.6f\tAllocator Released\n", (float)m_prx_AlignFilter->m_TimeStamp / 1000000.0f);
        }
    }

	return NOERROR;

} // BreakConnect

// NotifyAllocator
STDMETHODIMP C_rx_AlignFilter_InputPin::NotifyAllocator(IMemAllocator *pAllocator, BOOL bReadOnly)
{
    CheckPointer(pAllocator, E_FAIL);
    CAutoLock lock_it(m_pLock);

    // Free the old allocator if any
    if (m_prx_AlignFilter->m_pAllocator) m_prx_AlignFilter->m_pAllocator->Release();

    // Store away the new allocator
    pAllocator->AddRef();
    m_prx_AlignFilter->m_pAllocator = pAllocator;

    if (m_prx_AlignFilter->m_DebugOn)
    {
        m_prx_AlignFilter->m_MeasureTime = std::chrono::steady_clock::now();
        m_prx_AlignFilter->m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_prx_AlignFilter->m_MeasureTime - m_prx_AlignFilter->m_DebugStartTime).count();
        printf("%6.6f\tInputPin: NotifyAllocator\n", (float)m_prx_AlignFilter->m_TimeStamp / 1000000.0f);
    }
    // Notify the base class about the allocator
    return C_rx_AlignFilter_InputPin::CBaseInputPin::NotifyAllocator(pAllocator, bReadOnly);

} // NotifyAllocator

// EndOfStream
HRESULT C_rx_AlignFilter_InputPin::EndOfStream()
{
	CAutoLock lock_it(m_pLock);
	HRESULT hr = NOERROR;

	if (m_prx_AlignFilter->m_DebugOn)
	{
		m_prx_AlignFilter->m_MeasureTime = std::chrono::steady_clock::now();
		m_prx_AlignFilter->m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_prx_AlignFilter->m_MeasureTime - m_prx_AlignFilter->m_DebugStartTime).count();
		printf("%6.6f\tEnd of Stream\n", (float)m_prx_AlignFilter->m_TimeStamp / 1000000.0f);
	}

	hr = m_prx_AlignFilter->m_Output.DeliverEndOfStream();
	if (FAILED(hr)) return hr;

	return(NOERROR);

} // EndOfStream

// BeginFlush
HRESULT C_rx_AlignFilter_InputPin::BeginFlush()
{
	CAutoLock lock_it(m_pLock);
	HRESULT hr = NOERROR;

	if (m_prx_AlignFilter->m_DebugOn)
	{
		m_prx_AlignFilter->m_MeasureTime = std::chrono::steady_clock::now();
		m_prx_AlignFilter->m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_prx_AlignFilter->m_MeasureTime - m_prx_AlignFilter->m_DebugStartTime).count();
		printf("%6.6f\tBegin Flush\n", (float)m_prx_AlignFilter->m_TimeStamp / 1000000.0f);
	}

	hr = m_prx_AlignFilter->m_Output.DeliverBeginFlush();
	if (FAILED(hr)) return hr;

	return C_rx_AlignFilter_InputPin::CBaseInputPin::BeginFlush();

} // BeginFlush

// EndFlush
HRESULT C_rx_AlignFilter_InputPin::EndFlush()
{
	CAutoLock lock_it(m_pLock);
	HRESULT hr = NOERROR;

	if (m_prx_AlignFilter->m_DebugOn)
	{
		m_prx_AlignFilter->m_MeasureTime = std::chrono::steady_clock::now();
		m_prx_AlignFilter->m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_prx_AlignFilter->m_MeasureTime - m_prx_AlignFilter->m_DebugStartTime).count();
		printf("%6.6f\tEnd Flush\n", (float)m_prx_AlignFilter->m_TimeStamp / 1000000.0f);
	}

	hr = m_prx_AlignFilter->m_Output.DeliverEndFlush();
	if (FAILED(hr)) return hr;

	return C_rx_AlignFilter_InputPin::CBaseInputPin::EndFlush();

} // EndFlush

// NewSegment               
HRESULT C_rx_AlignFilter_InputPin::NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate)
{
	CAutoLock lock_it(m_pLock);
	HRESULT hr = NOERROR;

	if (m_prx_AlignFilter->m_DebugOn)
	{
		m_prx_AlignFilter->m_MeasureTime = std::chrono::steady_clock::now();
		m_prx_AlignFilter->m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_prx_AlignFilter->m_MeasureTime - m_prx_AlignFilter->m_DebugStartTime).count();
		printf("%6.6f\tNewSegment\n", (float)m_prx_AlignFilter->m_TimeStamp / 1000000.0f);
	}

	hr = m_prx_AlignFilter->m_Output.DeliverNewSegment(tStart, tStop, dRate);
	if (FAILED(hr)) return hr;

	return C_rx_AlignFilter_InputPin::CBaseInputPin::NewSegment(tStart, tStop, dRate);

} // NewSegment

// Receive
HRESULT C_rx_AlignFilter_InputPin::Receive(IMediaSample *pSampleIn)
{
	ASSERT(pSampleIn);
	CAutoLock lock_it(m_pLock);

	// Get frame
	HRESULT hr = NOERROR;
	hr = C_rx_AlignFilter_InputPin::CBaseInputPin::Receive(pSampleIn);
	if (hr != NOERROR)
	{
		if (m_prx_AlignFilter->m_DebugOn)
		{
			m_prx_AlignFilter->m_MeasureTime = std::chrono::steady_clock::now();
			m_prx_AlignFilter->m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_prx_AlignFilter->m_MeasureTime - m_prx_AlignFilter->m_DebugStartTime).count();
			printf("%6.6f\tCBaseInputPin::Receive Error (%d)\n", (float)m_prx_AlignFilter->m_TimeStamp / 1000000.0f, hr);
		}
		pSampleIn->Release();
		return hr;
	}

	//Set Start-Time for Frame Conversion Time measurement
	if (m_prx_AlignFilter->m_DebugOn && m_prx_AlignFilter->m_DspTiming)
	{
		m_prx_AlignFilter->m_FrameStartTime = std::chrono::steady_clock::now();
	}

	//Horizontal or vertical lines
	BOOL Vertical = !m_prx_AlignFilter->m_LinesHorizontal;

	//Histogram
	if (m_prx_AlignFilter->m_BinarizeMode != 0 || m_prx_AlignFilter->m_ShowHistogram)
	{
		if ((m_prx_AlignFilter->ImageCounter++ >= m_prx_AlignFilter->HistoRepeat))
		{
			m_prx_AlignFilter->ImageCounter = 0;
			switch (m_prx_AlignFilter->m_BinarizeMode) //0: off, 1: Auto, 2:Adaptive
			{
			case 1:
			case 2:
			{
				//Regular Histogram
				m_prx_AlignFilter->GetHistogram(pSampleIn, m_prx_AlignFilter->Histogram, m_prx_AlignFilter->m_BinarizeMode);
				if (m_prx_AlignFilter->m_ShowHistogram)
					m_prx_AlignFilter->ShowHistogram(pSampleIn, m_prx_AlignFilter->Histogram);
			}
			break;
			case 3:
			{
				//Multi Color  Histogram
				m_prx_AlignFilter->GetColorHistogram(pSampleIn, Vertical);
				if (m_prx_AlignFilter->m_ShowHistogram)
					m_prx_AlignFilter->ShowColorHistogram(pSampleIn, m_prx_AlignFilter->HistogramArray, Vertical);
			}
			break;
			}
			//SnapShot if needed
			if (TryEnterCriticalSection(&m_prx_AlignFilter->m_SnapShotLock))
			{
				if (m_prx_AlignFilter->m_TakeSnapShot)
					m_prx_AlignFilter->TakeSnapShot(pSampleIn);
				LeaveCriticalSection(&m_prx_AlignFilter->m_SnapShotLock);
			}

			if (m_prx_AlignFilter->m_ShowHistogram)
			{
				//Deliver Original Image with Histogram
				hr = m_prx_AlignFilter->m_Output.Deliver(pSampleIn);

				pSampleIn->Release();

				if (FAILED(hr))
				{
					if (m_prx_AlignFilter->m_DebugOn)
					{
						m_prx_AlignFilter->m_MeasureTime = std::chrono::steady_clock::now();
						m_prx_AlignFilter->m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_prx_AlignFilter->m_MeasureTime - m_prx_AlignFilter->m_DebugStartTime).count();
						printf("%6.6f\tDeliver Original Image with Histogram Failed (%d)\n", (float)m_prx_AlignFilter->m_TimeStamp / 1000000.0f, hr);
					}
				}
				return NOERROR;
			}
		}
	}

	//Binarization

	#pragma region Copy Out-Sample

	//Copy Sample
	IMediaSample *pOutSample = NULL;
	CMemAllocator *pOutAllocator;
	_AllocatorProperties OutAllocPropReq;
	_AllocatorProperties OutAllocPropAct;
	LONGLONG pOutSampleStart = 0;
	LONGLONG pOutSampleEnd = 0;
	BYTE* pData1 = NULL;
	BYTE* pData2 = NULL;


	pSampleIn->GetMediaTime(&pOutSampleStart, &pOutSampleEnd);
	pOutAllocator = new CMemAllocator(L"myAllocator", NULL, &hr);
	OutAllocPropReq.cBuffers = 1;
	OutAllocPropReq.cBuffers = 1;
	OutAllocPropReq.cbBuffer = pSampleIn->GetSize();
	OutAllocPropReq.cbAlign = 32;
	OutAllocPropReq.cbPrefix = 0;
	pOutAllocator->SetProperties(&OutAllocPropReq, &OutAllocPropAct);
	pOutAllocator->Commit();
	pOutAllocator->GetBuffer(&pOutSample, &pOutSampleStart, &pOutSampleEnd, NULL);
	pOutAllocator->Decommit();

	LONG cb1 = pSampleIn->GetActualDataLength();
	if (S_OK != pOutSample->SetActualDataLength(cb1))
	{
		if (m_prx_AlignFilter->m_DebugOn)
		{
			m_prx_AlignFilter->m_MeasureTime = std::chrono::steady_clock::now();
			m_prx_AlignFilter->m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_prx_AlignFilter->m_MeasureTime - m_prx_AlignFilter->m_DebugStartTime).count();
			printf("%6.6f\tCopy Out-Sample SetActualDataLength failed\n", (float)m_prx_AlignFilter->m_TimeStamp / 1000000.0f);
		}
	}

	if (S_OK != pSampleIn->GetPointer(&pData1))
	{
		if (m_prx_AlignFilter->m_DebugOn)
		{
			m_prx_AlignFilter->m_MeasureTime = std::chrono::steady_clock::now();
			m_prx_AlignFilter->m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_prx_AlignFilter->m_MeasureTime - m_prx_AlignFilter->m_DebugStartTime).count();
			printf("%6.6f\tCopy Out-Sample GetPointer pData1 failed\n", (float)m_prx_AlignFilter->m_TimeStamp / 1000000.0f);
		}
	}
	if (S_OK != pOutSample->GetPointer(&pData2))
	{
		if (m_prx_AlignFilter->m_DebugOn)
		{
			m_prx_AlignFilter->m_MeasureTime = std::chrono::steady_clock::now();
			m_prx_AlignFilter->m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_prx_AlignFilter->m_MeasureTime - m_prx_AlignFilter->m_DebugStartTime).count();
			printf("%6.6f\tCopy Out-Sample GetPointer pData2 failed\n", (float)m_prx_AlignFilter->m_TimeStamp / 1000000.0f);
		}
	}

	pOutSample->SetPreroll(S_OK == pSampleIn->IsPreroll());
	pOutSample->SetDiscontinuity(S_OK == pSampleIn->IsDiscontinuity());
	pOutSample->SetSyncPoint(S_OK == pSampleIn->IsSyncPoint());

	REFERENCE_TIME rt1, rt2;
	switch (pSampleIn->GetTime(&rt1, &rt2))
	{
	case S_OK: pOutSample->SetTime(&rt1, &rt2); break;
	case VFW_S_NO_STOP_TIME: pOutSample->SetTime(&rt1, NULL); break;
	case VFW_E_SAMPLE_TIME_NOT_SET: pOutSample->SetTime(NULL, NULL); break;
	default: 
		if (m_prx_AlignFilter->m_DebugOn)
		{
			m_prx_AlignFilter->m_MeasureTime = std::chrono::steady_clock::now();
			m_prx_AlignFilter->m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_prx_AlignFilter->m_MeasureTime - m_prx_AlignFilter->m_DebugStartTime).count();
			printf("%6.6f\tCopy Out-Sample SetTime failed\n", (float)m_prx_AlignFilter->m_TimeStamp / 1000000.0f);
		}
		break;
	}

	LONGLONG ll1, ll2;
	switch (pSampleIn->GetMediaTime(&ll1, &ll2))
	{
	case S_OK: pOutSample->SetMediaTime(&ll1, &ll2); break;
	case VFW_E_MEDIA_TIME_NOT_SET: pOutSample->SetMediaTime(NULL, NULL); break;
	default:
		if (m_prx_AlignFilter->m_DebugOn)
		{
			m_prx_AlignFilter->m_MeasureTime = std::chrono::steady_clock::now();
			m_prx_AlignFilter->m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_prx_AlignFilter->m_MeasureTime - m_prx_AlignFilter->m_DebugStartTime).count();
			printf("%6.6f\tCopy Out-Sample SetMediaTime failed\n", (float)m_prx_AlignFilter->m_TimeStamp / 1000000.0f);
		}
		break;
	}

	memcpy(pData2, pData1, cb1);

	#pragma endregion

	switch (m_prx_AlignFilter->m_BinarizeMode)
	{
	case 1:
	case 2:
		m_prx_AlignFilter->Binarize(pSampleIn, pOutSample);
		break;
	case 3:
		m_prx_AlignFilter->BinarizeMultiColor(pSampleIn, pOutSample, m_prx_AlignFilter->m_BinarizeMode);
		break;
	}

	//Dilate/Erode
	m_prx_AlignFilter->Erode(pOutSample, Vertical);

	//Find and qualify Blobs
	m_prx_AlignFilter->Blob(pOutSample);
	m_prx_AlignFilter->BlobQualifyer(pSampleIn, Vertical, pOutSample);

	//Measurement
    switch (m_prx_AlignFilter->m_MeasureMode)
    {
    case IFrx_AlignFilter::MeasureModeEnum::MeasureMode_AllLines:

        break;

	case IFrx_AlignFilter::MeasureModeEnum::MeasureMode_StartLines:
		m_prx_AlignFilter->FindStartLines(Vertical, m_prx_AlignFilter->m_UpsideDown);
		break;

	case IFrx_AlignFilter::MeasureModeEnum::MeasureMode_Angle:
		m_prx_AlignFilter->MeasureAngle(Vertical, m_prx_AlignFilter->m_UpsideDown);
		break;

	case IFrx_AlignFilter::MeasureModeEnum::MeasureMode_Stitch:
		m_prx_AlignFilter->MeasureStitch(Vertical, m_prx_AlignFilter->m_UpsideDown);
		break;

	case IFrx_AlignFilter::MeasureModeEnum::MeasureMode_Register:
		m_prx_AlignFilter->MeasureRegister(Vertical, m_prx_AlignFilter->m_UpsideDown);
		break;
	}

    //Overlay information
	if (m_prx_AlignFilter->m_OverlayBlobs)
	{
		m_prx_AlignFilter->OverlayBlobs(pSampleIn);
	}
	if (m_prx_AlignFilter->m_OverlayCenters)
	{
		m_prx_AlignFilter->OverlayCenters(pSampleIn, Vertical);
		m_prx_AlignFilter->OverlayDistanceLines(pSampleIn, Vertical);
	}
	if (m_prx_AlignFilter->m_OverlayValues)
	{
		m_prx_AlignFilter->OverlayValues(pSampleIn, Vertical);
	}


	if (m_prx_AlignFilter->m_ShowOriginal)
	{
		//Snapshot of current displayable Image
		if (TryEnterCriticalSection(&m_prx_AlignFilter->m_SnapShotLock))
		{
			if (m_prx_AlignFilter->m_TakeSnapShot) { m_prx_AlignFilter->TakeSnapShot(pSampleIn); }
			LeaveCriticalSection(&m_prx_AlignFilter->m_SnapShotLock);
		}

		//Deliver Original Image with/without Overlays
		hr = m_prx_AlignFilter->m_Output.Deliver(pSampleIn);
		if (FAILED(hr))
		{
			if (m_prx_AlignFilter->m_DebugOn)
			{
				m_prx_AlignFilter->m_MeasureTime = std::chrono::steady_clock::now();
				m_prx_AlignFilter->m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_prx_AlignFilter->m_MeasureTime - m_prx_AlignFilter->m_DebugStartTime).count();
				printf("%6.6f\tFrame Delivey Failed\n", (float)m_prx_AlignFilter->m_TimeStamp / 1000000.0f);
			}
			//Cleanup
			pOutSample->Release();
			pSampleIn->Release();
			return hr;
		}
	}
	else
	{
		//Snapshot of current displayable Image
		if (TryEnterCriticalSection(&m_prx_AlignFilter->m_SnapShotLock))
		{
			if (m_prx_AlignFilter->m_TakeSnapShot) { m_prx_AlignFilter->TakeSnapShot(pOutSample); }
			LeaveCriticalSection(&m_prx_AlignFilter->m_SnapShotLock);
		}

		//Deliver Process-View
		hr = m_prx_AlignFilter->m_Output.Deliver(pOutSample);
		if (FAILED(hr))
		{
			if (m_prx_AlignFilter->m_DebugOn)
			{
				m_prx_AlignFilter->m_MeasureTime = std::chrono::steady_clock::now();
				m_prx_AlignFilter->m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_prx_AlignFilter->m_MeasureTime - m_prx_AlignFilter->m_DebugStartTime).count();
				printf("%6.6f\tDeliver Blob-View Failed\n", (float)m_prx_AlignFilter->m_TimeStamp / 1000000.0f);
			}
			//Cleanup
			pOutSample->Release();
			pSampleIn->Release();
			return hr;
		}
	}

	//Prepare Measurement for Host
	if (m_prx_AlignFilter->m_measureDone)
	{
		if (TryEnterCriticalSection(&m_prx_AlignFilter->m_MeasureLock))
		{
			m_prx_AlignFilter->m_DataListSize = 0;
			m_prx_AlignFilter->m_DataListforHostReady = true;
			m_prx_AlignFilter->m_measureDone = false;
			LeaveCriticalSection(&m_prx_AlignFilter->m_MeasureLock);

			if (m_prx_AlignFilter->m_DebugOn)
			{
				m_prx_AlignFilter->m_MeasureTime = std::chrono::steady_clock::now();
				m_prx_AlignFilter->m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_prx_AlignFilter->m_MeasureTime - m_prx_AlignFilter->m_DebugStartTime).count();
				printf("%6.6f\tData for Host Ready\n", (float)m_prx_AlignFilter->m_TimeStamp / 1000000.0f);
			}

			switch (m_prx_AlignFilter->m_MeasureMode)
			{
			case IFrx_AlignFilter::MeasureModeEnum::MeasureMode_Angle:
				if (m_prx_AlignFilter->m_HostHwnd != NULL) SendMessage(m_prx_AlignFilter->m_HostHwnd, WM_APP_ALIGNEV, (WPARAM)WP_Angle, (LPARAM)0);
				break;
			case IFrx_AlignFilter::MeasureModeEnum::MeasureMode_Stitch:
				if (m_prx_AlignFilter->m_HostHwnd != NULL) SendMessage(m_prx_AlignFilter->m_HostHwnd, WM_APP_ALIGNEV, (WPARAM)WP_Stitch, (LPARAM)0);
				break;
			case IFrx_AlignFilter::MeasureModeEnum::MeasureMode_Register:
				if (m_prx_AlignFilter->m_HostHwnd != NULL) SendMessage(m_prx_AlignFilter->m_HostHwnd, WM_APP_ALIGNEV, (WPARAM)WP_Register, (LPARAM)0);
				break;
			}
		}
	}

    //Cleanup
	pOutSample->Release();
	pSampleIn->Release();

	//Change Modes and Settings
	m_prx_AlignFilter->ChangeModes();

	//Stop Timing
	if (m_prx_AlignFilter->m_DebugOn && m_prx_AlignFilter->m_DspTiming)
	{
		//Set End-Time for Frame Conversion Time
		m_prx_AlignFilter->m_FrameEndTime = std::chrono::steady_clock::now();
		//Calculate Frame Time in ms
		m_prx_AlignFilter->m_FrameTime += std::chrono::duration_cast<std::chrono::microseconds>(m_prx_AlignFilter->m_FrameEndTime - m_prx_AlignFilter->m_FrameStartTime).count();

		//Average 30 Frames
		if (m_prx_AlignFilter->NumFrames++ >= 30)
		{
			m_prx_AlignFilter->m_AvgFrameTime = m_prx_AlignFilter->m_FrameTime / 30;
			m_prx_AlignFilter->m_FrameTime = 0;;

			m_prx_AlignFilter->m_MeasureTime = std::chrono::steady_clock::now();
			m_prx_AlignFilter->m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_prx_AlignFilter->m_MeasureTime - m_prx_AlignFilter->m_DebugStartTime).count();
			printf("%6.6f\tFrame Time: %3.3fms\n", (float)m_prx_AlignFilter->m_TimeStamp / 1000000.0f, (float)m_prx_AlignFilter->m_AvgFrameTime / 1000);

			m_prx_AlignFilter->m_AvgFrameTime = 0;
			m_prx_AlignFilter->NumFrames = 0;
		}
	}
	m_prx_AlignFilter->m_DspTiming = m_prx_AlignFilter->m_PresetDspTiming;
	
	return NOERROR;
} // Receive

// Completed a connection to a pin
HRESULT C_rx_AlignFilter_InputPin::CompleteConnect(IPin *pReceivePin)
{
	ASSERT(pReceivePin);

	HRESULT hr = C_rx_AlignFilter_InputPin::CBaseInputPin::CompleteConnect(pReceivePin);
	if (FAILED(hr)) return hr;

	if (m_prx_AlignFilter->m_Output.m_Connected != NULL)
	{
		if (m_prx_AlignFilter->m_mt1 != m_prx_AlignFilter->m_Output.m_mt)
			m_prx_AlignFilter->ReconnectPin(&m_prx_AlignFilter->m_Output, &m_prx_AlignFilter->m_mt1);
	}

	return S_OK;
}

#pragma endregion

#pragma region OutputPin

// C_rx_AlignFilter_OutputPin constructor
C_rx_AlignFilter_OutputPin::C_rx_AlignFilter_OutputPin(TCHAR *pName, C_rx_AlignFilter *prx_AlignFilter, HRESULT *phr, LPCWSTR pPinName) :
	CBaseOutputPin(pName, prx_AlignFilter, prx_AlignFilter, phr, pPinName),
	m_pOutputQueue(NULL),
	m_bHoldsSeek(FALSE),
	m_pPosition(NULL),
	m_prx_AlignFilter(prx_AlignFilter),
	m_cOurRef(0),
	m_bInsideCheckMediaType(false)
{
	ASSERT(prx_AlignFilter);
	C_rx_AlignFilter_OutputPin::PinName = pPinName;

	printf("Filter create Output Pin\n");
}

// DecideBufferSize, This has to be present to override the PURE virtual class base function
HRESULT C_rx_AlignFilter_OutputPin::DecideBufferSize(IMemAllocator *pMemAllocator, ALLOCATOR_PROPERTIES * ppropInputRequest)
{
	return NOERROR;

} // DecideBufferSize

// DecideAllocator
HRESULT C_rx_AlignFilter_OutputPin::DecideAllocator(IMemInputPin *pPin, IMemAllocator **ppAlloc)
{
    CheckPointer(pPin, E_POINTER);
    CheckPointer(ppAlloc, E_POINTER);
    ASSERT(m_prx_AlignFilter->m_pAllocator != NULL);

    *ppAlloc = NULL;

    // Tell the pin about our allocator, set by the input pin.
    HRESULT hr = NOERROR;
    hr = pPin->NotifyAllocator(m_prx_AlignFilter->m_pAllocator, TRUE);
    if (FAILED(hr))
		return hr;

    // Return the allocator
    *ppAlloc = m_prx_AlignFilter->m_pAllocator;
    m_prx_AlignFilter->m_pAllocator->AddRef();

    if (m_prx_AlignFilter->m_DebugOn)
    {
        m_prx_AlignFilter->m_MeasureTime = std::chrono::steady_clock::now();
        m_prx_AlignFilter->m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_prx_AlignFilter->m_MeasureTime - m_prx_AlignFilter->m_DebugStartTime).count();
        printf("%6.6f\tOutputPin: DecideAllocator\n", (float)m_prx_AlignFilter->m_TimeStamp / 1000000.0f);
    }

    return NOERROR;

} // DecideAllocator

// CheckMediaType
HRESULT C_rx_AlignFilter_OutputPin::CheckMediaType(const CMediaType *pmt)
{
	CAutoLock lock_it(m_pLock);

	if (m_bInsideCheckMediaType) return NOERROR;
	m_bInsideCheckMediaType = true;

	HRESULT hr = NOERROR;

	if (IsEqualGUID(*pmt->Type(), MEDIATYPE_Video))
	{
		if (IsEqualGUID(*pmt->Subtype(), MEDIASUBTYPE_RGB32))
		{
			m_bInsideCheckMediaType = false;
			return NOERROR;
		}
	}
	m_bInsideCheckMediaType = false;
	return VFW_E_TYPE_NOT_ACCEPTED;

	if (m_prx_AlignFilter->m_DebugOn)
	{
		m_prx_AlignFilter->m_MeasureTime = std::chrono::steady_clock::now();
		m_prx_AlignFilter->m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_prx_AlignFilter->m_MeasureTime - m_prx_AlignFilter->m_DebugStartTime).count();
		printf("%6.6f\tOutputPin: CheckMediaType Input Unknown MediaSubtype\n", (float)m_prx_AlignFilter->m_TimeStamp / 1000000.0f);
	}
	m_bInsideCheckMediaType = false;
	return E_FAIL;

} // CheckMediaType

// EnumMediaTypes
STDMETHODIMP C_rx_AlignFilter_OutputPin::EnumMediaTypes(IEnumMediaTypes **ppEnum)
{
	CAutoLock lock_it(m_pLock);
	ASSERT(ppEnum);

	// Make sure that we are connected
	if (m_prx_AlignFilter->m_Input.m_Connected == NULL) return VFW_E_NOT_CONNECTED;
	// We will simply return the enumerator of our input pin's peer
	return m_prx_AlignFilter->m_Input.m_Connected->EnumMediaTypes(ppEnum);

	return NULL;
} // EnumMediaTypes

// SetMediaType
HRESULT C_rx_AlignFilter_OutputPin::SetMediaType(const CMediaType *pmt)
{
	CAutoLock lock_it(m_pLock);
	HRESULT hr = NOERROR;

	// Make sure that we have an input connected
	if (m_prx_AlignFilter->m_Input.m_Connected == NULL) return VFW_E_NOT_CONNECTED;

	// Make sure that the base class likes it
	hr = C_rx_AlignFilter_OutputPin::CBaseOutputPin::SetMediaType(pmt);
	if (FAILED(hr))	return hr;

	return NOERROR;

} // SetMediaType

// CompleteConnect
HRESULT C_rx_AlignFilter_OutputPin::CompleteConnect(IPin *pReceivePin)
{
	CAutoLock lock_it(m_pLock);
	ASSERT(m_Connected == pReceivePin);
	HRESULT hr = NOERROR;

	hr = C_rx_AlignFilter_OutputPin::CBaseOutputPin::CompleteConnect(pReceivePin);
	if (FAILED(hr)) return hr;

	// If the type is not the same as that stored for the input
	// pin then force the input pins peer to be reconnected

	if (m_prx_AlignFilter->m_mt1 != m_prx_AlignFilter->m_Input.m_mt)
	{
		hr = m_prx_AlignFilter->ReconnectPin(m_prx_AlignFilter->m_Input.m_Connected, &m_prx_AlignFilter->m_mt1);
		if (FAILED(hr))
		{
			if (m_prx_AlignFilter->m_DebugOn)
			{
				TCHAR MsgMsg[10];
				_itow_s(hr, MsgMsg, 10, 16);
				m_prx_AlignFilter->m_MeasureTime = std::chrono::steady_clock::now();
				m_prx_AlignFilter->m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_prx_AlignFilter->m_MeasureTime - m_prx_AlignFilter->m_DebugStartTime).count();
				printf("%6.6f\tOutputPin: Complete Connect Set MediaType failed: %ls\n", (float)m_prx_AlignFilter->m_TimeStamp / 1000000.0f, MsgMsg);
			}
			return hr;
		}
	}


	return NOERROR;

} // CompleteConnect

// Active
HRESULT C_rx_AlignFilter_OutputPin::Active()
{
	CAutoLock lock_it(m_pLock);
	HRESULT hr = NOERROR;

	if (m_prx_AlignFilter->m_DebugOn)
    {
        m_prx_AlignFilter->m_MeasureTime = std::chrono::steady_clock::now();
        m_prx_AlignFilter->m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_prx_AlignFilter->m_MeasureTime - m_prx_AlignFilter->m_DebugStartTime).count();
        printf("%6.6f\tOutputPin::Active\n", (float)m_prx_AlignFilter->m_TimeStamp / 1000000.0f);
    }

    // Make sure that the pin is connected
	if (m_Connected == NULL) return NOERROR;

	// Create the output queue if we have to
	if (m_pOutputQueue == NULL)
	{
		m_pOutputQueue = new COutputQueue(m_Connected, &hr, FALSE, FALSE, 1, FALSE, DEFAULTCACHE, THREAD_PRIORITY_HIGHEST);
		if (m_pOutputQueue == NULL) return E_OUTOFMEMORY;

		// Make sure that the constructor did not return any error
		if (FAILED(hr))
		{
			delete m_pOutputQueue;
			m_pOutputQueue = NULL;
			return hr;
		}
	}
	
	// Pass the call on to the base class
	hr = C_rx_AlignFilter_OutputPin::CBaseOutputPin::Active();
	if (FAILED(hr))
	{
		return hr;
	}
	return NOERROR;

} // Active

// Inactive
HRESULT C_rx_AlignFilter_OutputPin::Inactive()
{
	CAutoLock lock_it(m_pLock);

	if (m_prx_AlignFilter->m_DebugOn)
	{
		m_prx_AlignFilter->m_MeasureTime = std::chrono::steady_clock::now();
		m_prx_AlignFilter->m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_prx_AlignFilter->m_MeasureTime - m_prx_AlignFilter->m_DebugStartTime).count();
		printf("%6.6f\tOutputPin::Inactive\n", (float)m_prx_AlignFilter->m_TimeStamp / 1000000.0f);
	}

	// Delete the output queus associated with the pin.
	if (m_pOutputQueue)
	{
		delete m_pOutputQueue;
		m_pOutputQueue = NULL;
	}

	C_rx_AlignFilter_OutputPin::CBaseOutputPin::Inactive();
	return NOERROR;

} // Inactive


// Deliver
HRESULT C_rx_AlignFilter_OutputPin::Deliver(IMediaSample *pMediaSample)
{
	CheckPointer(pMediaSample, E_POINTER);

	// Make sure that we have an output queue
	if (m_pOutputQueue == NULL) return NOERROR;


	pMediaSample->AddRef();
	return m_pOutputQueue->Receive(pMediaSample);

} // Deliver


// DeliverEndOfStream
HRESULT C_rx_AlignFilter_OutputPin::DeliverEndOfStream()
{
    if (m_prx_AlignFilter->m_DebugOn)
    {
        m_prx_AlignFilter->m_MeasureTime = std::chrono::steady_clock::now();
        m_prx_AlignFilter->m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_prx_AlignFilter->m_MeasureTime - m_prx_AlignFilter->m_DebugStartTime).count();
        printf("%6.6f\tOutputPin::DeliverEndOfStream\n", (float)m_prx_AlignFilter->m_TimeStamp / 1000000.0f);
    }

    // Make sure that we have an output queue
	if (m_pOutputQueue == NULL) return NOERROR;

	m_pOutputQueue->EOS();
	return NOERROR;

} // DeliverEndOfStream


// DeliverBeginFlush
HRESULT C_rx_AlignFilter_OutputPin::DeliverBeginFlush()
{
	// Make sure that we have an output queue
	if (m_pOutputQueue == NULL) return NOERROR;

	m_pOutputQueue->BeginFlush();
	return NOERROR;

} // DeliverBeginFlush


// DeliverEndFlush
HRESULT C_rx_AlignFilter_OutputPin::DeliverEndFlush()
{
	// Make sure that we have an output queue
	if (m_pOutputQueue == NULL) return NOERROR;

	m_pOutputQueue->EndFlush();
	return NOERROR;

} // DeliverEndFlish


// DeliverNewSegment
HRESULT C_rx_AlignFilter_OutputPin::DeliverNewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate)
{
	// Make sure that we have an output queue
	if (m_pOutputQueue == NULL) return NOERROR;

	m_pOutputQueue->NewSegment(tStart, tStop, dRate);
	return NOERROR;

} // DeliverNewSegment


#pragma endregion


#pragma region Host Interface

#pragma region General

//Version Info
STDMETHODIMP C_rx_AlignFilter::GetVersion(wchar_t* VersionInfo, INT32* VersionInfoSize)
{
		//If Version Info is empty, return num Charcters
	if (VersionInfo == NULL)
	{
		CheckPointer(VersionInfoSize, E_POINTER);
		size_t Length = std::string(VER_PRODUCT_VERSION_STR).length();
		*VersionInfoSize = (UINT32)Length;
		return NOERROR;
	}

	//Othetwise set string
	CheckPointer(VersionInfo, E_POINTER);

	size_t* CharCount = 0;
	mbstowcs_s(CharCount, VersionInfo, *VersionInfoSize + (int)1, VER_PRODUCT_VERSION_STR, _TRUNCATE);

	return NOERROR;
}

//C++ Version Info
STDMETHODIMP_(UINT32) C_rx_AlignFilter::GetCppVersion()
{
	return _MSC_VER;
}

//WindowHandle of Host Interface
STDMETHODIMP C_rx_AlignFilter::SetHostPointer(HWND HostHwnd)
{
	CAutoLock cAutolock(m_pLock);
	m_HostHwnd = HostHwnd;
	SetDirty(TRUE);

	return NOERROR;
}

//Display Device Name
STDMETHODIMP C_rx_AlignFilter::GetDeviceName(LPCWSTR DeviceName)
{
	CheckPointer(DeviceName, E_POINTER);
	wcscpy((wchar_t*)DeviceName, m_DeviceName);
	return NOERROR;
}

//Debug On
STDMETHODIMP C_rx_AlignFilter::SetDebug(BOOL DebugOn)
{
    CAutoLock cAutolock(m_pLock);

    m_DebugPrepare = DebugOn;

    if (m_DebugPrepare)
    {
        if (!m_ConsoleAllocated)
        {
            if (AllocConsole())
            {
                pStdIn = freopen("CONIN$", "r", stdin);
                pStdOut = freopen("CONOUT$", "w", stdout);
                pStdErr = freopen("CONOUT$", "w", stderr);
                printf("rx_AlignFilter Debug-Mode\n");
                m_ConsoleAllocated = TRUE;
                m_DebugOn = true;
            }
            else
            {
                //MessageBox(NULL, L"Could not allocate Console", L"Console", MB_OK);
            }
        }
    }
    else
    {
        m_DebugOn = false;
        if (m_ConsoleAllocated)
        {
            printf("rx_AlignFilter closing Debug-Mode\n");

            if (!FreeConsole())
            {
                m_MeasureTime = std::chrono::steady_clock::now();
				m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
				printf("%6.6f\tSetDebugOn: Could not free console\n", (float)m_TimeStamp / 1000000.0f);
            }
            else
            {
                m_ConsoleAllocated = FALSE;
            }
        }
    }

    SetDirty(TRUE);
    return NOERROR;
}

//Display Frame Timimg
STDMETHODIMP C_rx_AlignFilter::SetFrameTiming(BOOL DspFrameTime)
{
	m_PresetDspTiming = DspFrameTime;

	if (m_DebugOn)
	{
		m_MeasureTime = std::chrono::steady_clock::now();
		m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
		printf("%6.6f\tDisplay Frame Timing: %d\n", (float)m_TimeStamp / 1000000.0f, m_DspTiming);
	}

	return NOERROR;
}


//Take SnapShot and Save to
STDMETHODIMP_ (BOOL) C_rx_AlignFilter::TakeSnapShot(const wchar_t* SnapDirectory, const wchar_t* SnapFileName)
{
	if (!TryEnterCriticalSection(&m_SnapShotLock))
	{
		return false;
	}
	else
	{
		//kompletter Speicherpfad

		//Filename
		wchar_t FileName[MAX_PATH];

		if (SnapFileName == NULL)
		{
			struct tm now;
			__time32_t nowtime;
			_time32(&nowtime);
			_localtime32_s(&now, &nowtime);

			wchar_t Year[5];
			_itow_s(now.tm_year + 1900, Year, 5, 10);
			wchar_t Month[3];
			_itow_s(now.tm_mon + 1, Month, 3, 10);
			wchar_t Day[3];
			_itow_s(now.tm_mday, Day, 3, 10);
			wchar_t Hour[3];
			_itow_s(now.tm_hour, Hour, 3, 10);
			wchar_t Min[3];
			_itow_s(now.tm_min, Min, 3, 10);
			wchar_t Sec[3];
			_itow_s(now.tm_sec, Sec, 3, 10);

			wcscpy_s(FileName, MAX_PATH, Year);
			wcscat_s(FileName, MAX_PATH, L"-");
			wcscat_s(FileName, MAX_PATH, Month);
			wcscat_s(FileName, MAX_PATH, L"-");
			wcscat_s(FileName, MAX_PATH, Day);
			wcscat_s(FileName, MAX_PATH, L"-");
			wcscat_s(FileName, MAX_PATH, Hour);
			wcscat_s(FileName, MAX_PATH, L"-");
			wcscat_s(FileName, MAX_PATH, Min);
			wcscat_s(FileName, MAX_PATH, L"-");
			wcscat_s(FileName, MAX_PATH, Sec);
		}
		else
		{
			wcscpy_s(FileName, MAX_PATH, SnapFileName);
		}

		wcscpy_s(m_SavePathName, MAX_PATH, SnapDirectory);
		wcscat_s(m_SavePathName, MAX_PATH, L"\\");
		wcscat_s(m_SavePathName, MAX_PATH, FileName);
		wcscat_s(m_SavePathName, MAX_PATH, L".bmp");

		if (m_DebugOn)
		{
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tTake SnapShot\n", (float)m_TimeStamp / 1000000.0f);
		}

		m_TakeSnapShot = TRUE;
		LeaveCriticalSection(&m_SnapShotLock);
	}

	return true;
}

//ShowOriginalImage
STDMETHODIMP C_rx_AlignFilter::SetShowOriginalImage(BOOL ShowOriginalImage)
{
	m_PresetShowOriginal = ShowOriginalImage;

	if (m_DebugOn)
	{
		m_MeasureTime = std::chrono::steady_clock::now();
		m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
		printf("%6.6f\tShow Original Image: %d\n", (float)m_TimeStamp / 1000000.0f, m_ShowOriginal);
	}

	return NOERROR;
}
STDMETHODIMP_(BOOL) C_rx_AlignFilter::GetShowOriginalImage()
{
	//	CAutoLock cAutolock(m_pLock);
	return m_ShowOriginal;
}

//Overlay-Text
STDMETHODIMP_(BOOL) C_rx_AlignFilter::SetOverlayTxt(const wchar_t* OverlayTxt, UINT32 OverlayTxtColor)
{
	int TryCount = 20;
	bool Done = false;

	while (!Done && TryCount > 0)
	{
		if (!TryEnterCriticalSection(&m_OvTextLock))
		{
			TryCount--;
			if (m_DebugOn)
			{
				m_MeasureTime = std::chrono::steady_clock::now();
				m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
				printf("%6.6f\tSetOverlayTxt, Try#: %d\n", (float)m_TimeStamp / 1000000.0f, TryCount);
			}
			continue;
		}
		else
		{
			m_OverlayTxtReady = false;
			if (OverlayTxt != NULL)
			{
				m_OverlayTxtColor = OverlayTxtColor;
				wcscpy_s(m_OverlayTxt, MAX_PATH, OverlayTxt);
				m_OverlayTxtReady = true;
			}
			LeaveCriticalSection(&m_OvTextLock);
			Done = true;
			if (m_DebugOn)
			{
				m_MeasureTime = std::chrono::steady_clock::now();
				m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
				printf("%6.6f\tSetOverlayTxt, Success\n", (float)m_TimeStamp / 1000000.0f);
			}
		}
	}
	if (!Done) return false;

	return true;
}

#pragma endregion

#pragma region Binarize

//Binarization
STDMETHODIMP C_rx_AlignFilter::SetBinarizeMode(UINT BinarizeMode)
{
	m_PresetBinarizeMode = BinarizeMode;

	if (m_DebugOn)
	{
		m_MeasureTime = std::chrono::steady_clock::now();
		m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
		printf("%6.6f\tSet Binarite: %d\n", (float)m_TimeStamp / 1000000.0f, m_BinarizeMode);
	}

	return NOERROR;
}
STDMETHODIMP_(UINT) C_rx_AlignFilter::GetBinarizeMode()
{
	CAutoLock cAutolock(m_pLock);
	return m_BinarizeMode;
}
//Threshold
STDMETHODIMP C_rx_AlignFilter::SetThreshold(UINT Threshold)
{
	m_PresetThreshold = Threshold;

	if (m_DebugOn)
	{
		m_MeasureTime = std::chrono::steady_clock::now();
		m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
		printf("%6.6f\tSet Threshold: %d\n", (float)m_TimeStamp / 1000000.0f, m_Threshold);
	}

	return NOERROR;
}
STDMETHODIMP_(UINT) C_rx_AlignFilter::GetThreshold()
{
	CAutoLock cAutolock(m_pLock);
	return m_Threshold;
}
//Show Histogram
STDMETHODIMP C_rx_AlignFilter::ShowHistogram(BOOL ShowHistogram)
{
	m_PresetShowHistogram = ShowHistogram;

	return NOERROR;
}

#pragma endregion

#pragma region Dilate-Erode

//Num Dilate-Erodes
STDMETHODIMP C_rx_AlignFilter::SetNumDilateErodes(UINT DilateErodes)
{
	m_PresetDilateErodes = DilateErodes;

	if (m_DebugOn)
	{
		m_MeasureTime = std::chrono::steady_clock::now();
		m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
		printf("%6.6f\tSet Num Dilate-Erodes: %d\n", (float)m_TimeStamp / 1000000.0f, m_DilateErodes);
	}

	return NOERROR;
}
STDMETHODIMP_(UINT) C_rx_AlignFilter::GetNumDilateErodes()
{
//	CAutoLock cAutolock(m_pLock);
	return m_DilateErodes;
}
//Num Extra-Erodes
STDMETHODIMP C_rx_AlignFilter::SetNumExtraErodes(UINT ExtraErodes)
{
	m_PresetExtraErodes = ExtraErodes;

	if (m_DebugOn)
	{
		m_MeasureTime = std::chrono::steady_clock::now();
		m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
		printf("%6.6f\tSet Num Extra Erodes: %d\n", (float)m_TimeStamp / 1000000.0f, m_ExtraErodes);
	}

	return NOERROR;
}
STDMETHODIMP_(UINT) C_rx_AlignFilter::GetNumExtraErodes()
{
//	CAutoLock cAutolock(m_pLock);
	return m_ExtraErodes;
}
//Erode Seed
STDMETHODIMP C_rx_AlignFilter::SetErodeSeedX(UINT ErodeSeedX)
{
	m_PresetErodeSeedX = ErodeSeedX;

	if (m_DebugOn)
	{
		m_MeasureTime = std::chrono::steady_clock::now();
		m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
		printf("%6.6f\tSet Erode Seed X: %d\n", (float)m_TimeStamp / 1000000.0f, m_ErodeSeedX);
	}

	return NOERROR;
}
STDMETHODIMP C_rx_AlignFilter::SetErodeSeedY(UINT ErodeSeedY)
{
	m_PresetErodeSeedY = ErodeSeedY;

	if (m_DebugOn)
	{
		m_MeasureTime = std::chrono::steady_clock::now();
		m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
		printf("%6.6f\tSet Erode Seed Y: %d\n", (float)m_TimeStamp / 1000000.0f, m_ErodeSeedY);
	}

	return NOERROR;
}
STDMETHODIMP_(UINT) C_rx_AlignFilter::GetErodeSeedX()
{
//	CAutoLock cAutolock(m_pLock);
	return m_ErodeSeedX;
}
STDMETHODIMP_(UINT) C_rx_AlignFilter::GetErodeSeedY()
{
//	CAutoLock cAutolock(m_pLock);
	return m_ErodeSeedY;
}

#pragma endregion

#pragma region Blob

STDMETHODIMP C_rx_AlignFilter::SetCrossColor(COLORREF CrossColorBlob)
{
	CAutoLock cAutolock(m_pLock);

	m_CrossColorBlob = CrossColorBlob;
	SetDirty(TRUE);

	return NOERROR;
}
STDMETHODIMP_(COLORREF) C_rx_AlignFilter::GetCrossColor()
{
//	CAutoLock cAutolock(m_pLock);
	return m_CrossColorBlob;
}

//BlobOutlineColor
STDMETHODIMP C_rx_AlignFilter::SetBlobOutlineColor(UINT32 BlobOutlineColor)
{
	CAutoLock cAutolock(m_pLock);

	m_BlobColor = BlobOutlineColor;

	SetDirty(TRUE);
	return NOERROR;
}
STDMETHODIMP_(UINT32) C_rx_AlignFilter::GetBlobOutlineColor()
{
//	CAutoLock cAutolock(m_pLock);
	return m_BlobColor;
}
//BlobCrossColor
STDMETHODIMP C_rx_AlignFilter::SetBlobCrossColor(UINT32 BlobCrossColor)
{
	CAutoLock cAutolock(m_pLock);

	m_CrossColor = BlobCrossColor;

	SetDirty(TRUE);
	return NOERROR;
}
STDMETHODIMP_(UINT32) C_rx_AlignFilter::GetBlobCrossColor()
{
//	CAutoLock cAutolock(m_pLock);
	return m_CrossColor;
}
//BlobTextColor
STDMETHODIMP C_rx_AlignFilter::SetBlobTextColor(UINT32 BlobTextColor)
{
	CAutoLock cAutolock(m_pLock);

	m_TextColor = BlobTextColor;

	SetDirty(TRUE);
	return NOERROR;
}
STDMETHODIMP_(UINT32) C_rx_AlignFilter::GetBlobTextColor()
{
//	CAutoLock cAutolock(m_pLock);
	return m_TextColor;
}

//BlobAspectLimit
STDMETHODIMP C_rx_AlignFilter::SetBlobAspectLimit(UINT32 BlobAspectLimit)
{
	m_PresetBlobAspectLimit = BlobAspectLimit;

	return NOERROR;
}
STDMETHODIMP_(UINT32) C_rx_AlignFilter::GetBlobAspectLimit()
{
//	CAutoLock cAutolock(m_pLock);
	return m_BlobAspectLimit;
}
//BlobAreaDivisor
STDMETHODIMP C_rx_AlignFilter::SetBlobAreaDivisor(UINT32 BlobAreaDivisor)
{
	m_PresetBlobAreaDivisor = BlobAreaDivisor;

	return NOERROR;
}
STDMETHODIMP_(UINT32) C_rx_AlignFilter::GetBlobAreaDivisor()
{
//	CAutoLock cAutolock(m_pLock);
	return m_BlobAreaDivisor;
}

//ShowBlobOutlines
STDMETHODIMP C_rx_AlignFilter::SetShowBlobOutlines(BOOL ShowBlobOutlines)
{
	CAutoLock cAutolock(m_pLock);

	m_OverlayBlobs = ShowBlobOutlines;

	SetDirty(TRUE);
	return NOERROR;
}
STDMETHODIMP_(BOOL) C_rx_AlignFilter::GetShowBlobOutlines()
{
//	CAutoLock cAutolock(m_pLock);
	return m_OverlayBlobs;
}
//ShowBlobCenters
STDMETHODIMP C_rx_AlignFilter::SetShowBlobCenters(BOOL ShowBlobCenters)
{
	CAutoLock cAutolock(m_pLock);

	m_OverlayCenters = ShowBlobCenters;

	SetDirty(TRUE);
	return NOERROR;
}
STDMETHODIMP_(BOOL) C_rx_AlignFilter::GetShowBlobCenters()
{
//	CAutoLock cAutolock(m_pLock);
	return m_OverlayCenters;
}
//ShowBlobValues
STDMETHODIMP C_rx_AlignFilter::SetShowBlobValues(BOOL ShowBlobValues)
{
	CAutoLock cAutolock(m_pLock);

	m_OverlayValues = ShowBlobValues;

	SetDirty(TRUE);
	return NOERROR;
}
STDMETHODIMP_(BOOL) C_rx_AlignFilter::GetShowBlobValues()
{
//	CAutoLock cAutolock(m_pLock);
	return m_OverlayValues;
}
//Font for Blob Values
STDMETHODIMP C_rx_AlignFilter::SetBlobFont(void* pLogFontStruct)
{
	CAutoLock cAutolock(m_pLock);

	LOGFONT lf;
	memcpy(&lf, pLogFontStruct, sizeof(LOGFONT));

	if (m_DebugOn)
	{
		m_MeasureTime = std::chrono::steady_clock::now();
		m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
		printf("%6.6f\tSetFont:\n", (float)m_TimeStamp / 1000000.0f);
		printf("\t\tHeight: %d\n", lf.lfHeight);
		printf("\t\tFaceName: %ls\n", lf.lfFaceName);
		printf("\t\tCharSet: %d\n", lf.lfCharSet);
		printf("\t\tItalic: %d\n", lf.lfItalic);
		printf("\t\tWeight: %d\n", lf.lfWeight);
		printf("\t\tOrientation: %d\n", lf.lfOrientation);
	}

	m_TextFont = CreateFontIndirect(&lf);
	m_FontHeight = lf.lfHeight;
	lf.lfHeight = (long)(lf.lfHeight * 1.5);

	SetDirty(TRUE);
	return NOERROR;
}
STDMETHODIMP C_rx_AlignFilter::GetBlobFont(void* pLogFontStruct, UINT32* LogFontSize)
{
//	CAutoLock cAutolock(m_pLock);

	//retrun Structure Size
	if (pLogFontStruct == NULL)
	{
		*LogFontSize = sizeof(LOGFONT);
	}
	//return Structure Data
	else
	{
		LOGFONT lf;
		GetObject(m_TextFont, sizeof(LOGFONT), &lf);
		memcpy(pLogFontStruct, &lf, sizeof(LOGFONT));
	}

	return NOERROR;
}

#pragma endregion

#pragma region Measurement

//Measure in Pixels
STDMETHODIMP C_rx_AlignFilter::SetMeasurePixels(BOOL MeasurePixels)
{
	CAutoLock cAutolock(m_pLock);

	m_MeasurePixels = MeasurePixels;

	if (m_DebugOn)
	{
		m_MeasureTime = std::chrono::steady_clock::now();
		m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
		printf("%6.6f\tSet MeasurePixels: %d\n", (float)m_TimeStamp / 1000000.0f, m_MeasurePixels);
	}

	SetDirty(TRUE);
	return NOERROR;
}
STDMETHODIMP_(BOOL) C_rx_AlignFilter::GetMeasurePixels()
{
//	CAutoLock cAutolock(m_pLock);
	return m_MeasurePixels;
}

//Inverse Image for White
STDMETHODIMP C_rx_AlignFilter::SetInverse(BOOL InverseImage)
{
	CAutoLock cAutolock(m_pLock);

	m_InverseImage = InverseImage;

	if (m_DebugOn)
	{
		m_MeasureTime = std::chrono::steady_clock::now();
		m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
		printf("%6.6f\tInverse Image: %d\n", (float)m_TimeStamp / 1000000.0f, m_InverseImage);
	}

	SetDirty(TRUE);
	return NOERROR;
}
STDMETHODIMP_(BOOL) C_rx_AlignFilter::GetInverse()
{
	return m_InverseImage;
}

//MeasureMode
STDMETHODIMP C_rx_AlignFilter::SetMeasureMode(C_rx_AlignFilter::MeasureModeEnum MeasureMode)
{
	m_PresetMeasureMode = MeasureMode;

	if (m_DebugOn)
	{
		m_MeasureTime = std::chrono::steady_clock::now();
		m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
		printf("%6.6f\tSet Measure Mode: %d\n", (float)m_TimeStamp / 1000000.0f, m_MeasureMode);
	}

	return NOERROR;
}
STDMETHODIMP_(C_rx_AlignFilter::MeasureModeEnum) C_rx_AlignFilter::GetMeasureMode()
{
//	CAutoLock cAutolock(m_pLock);
	return m_MeasureMode;
}

//DisplayMode
STDMETHODIMP C_rx_AlignFilter::SetDisplayMode(C_rx_AlignFilter::DisplayModeEnum DisplayMode)
{
	m_PresetDisplayMode = DisplayMode;

	if (m_DebugOn)
	{
		m_MeasureTime = std::chrono::steady_clock::now();
		m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
		printf("%6.6f\tSet Display Mode: %d\n", (float)m_TimeStamp / 1000000.0f, m_DisplayMode);
	}

	return NOERROR;
}
STDMETHODIMP_(C_rx_AlignFilter::DisplayModeEnum) C_rx_AlignFilter::GetDisplayMode()
{
//	CAutoLock cAutolock(m_pLock);
	return m_DisplayMode;
}

//Minimum number of StartLines
STDMETHODIMP C_rx_AlignFilter::SetMinNumStartLines(UINT32 MinNumStartLines)
{
    CAutoLock cAutolock(m_pLock);

    m_MinNumStartLines = MinNumStartLines;

    if (m_DebugOn)
    {
        m_MeasureTime = std::chrono::steady_clock::now();
		m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
		printf("%6.6f\tSet MinNumStartLines: %d\n", (float)m_TimeStamp / 1000000.0f, m_MinNumStartLines);
	}

    SetDirty(TRUE);
    return NOERROR;
}

//Execute Measures
STDMETHODIMP_(BOOL) C_rx_AlignFilter::DoMeasures(UINT32 NumMeasures)
{
	CAutoLock cAutolock(m_pLock);

	if (m_MeasureMode == MeasureModeEnum::MeasureMode_Off ||
		m_MeasureMode == MeasureModeEnum::MeasureMode_AllLines ||
		m_MeasureMode == MeasureModeEnum::MeasureMode_StartLines ||
		m_MeasureRunning)
	{
		if (m_DebugOn)
		{
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tDoMeasures but MeasureMode: %d, MeasureRunning: %d\n", (float)m_TimeStamp / 1000000.0f, m_MeasureMode, m_MeasureRunning);
		}
		return false;
	}

	if (m_DebugOn)
	{
		m_MeasureTime = std::chrono::steady_clock::now();
		m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
		printf("%6.6f\tDoMeasures: %d\n", (float)m_TimeStamp / 1000000.0f, NumMeasures);
	}

	m_measureDone = false;
	m_DataListforHostReady = false;
	m_vMeasureDataList.clear();
	m_NumMeasures = NumMeasures;
	m_StartMeasure = true;

	SetDirty(TRUE);
	return true;
}

//GetMeasure Results
STDMETHODIMP_(BOOL) C_rx_AlignFilter::GetMeasureResults(void* pMeasureDataStructArray, UINT32* ListSize)
{
	//Measuremode selected and data ready
	if (m_MeasureMode == MeasureModeEnum::MeasureMode_Off ||
		m_MeasureMode == MeasureModeEnum::MeasureMode_AllLines ||
		m_MeasureMode == MeasureModeEnum::MeasureMode_StartLines ||
		m_MeasureRunning || !m_DataListforHostReady)
	{
		if (m_DebugOn)
		{
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tGetMeasureResults but MeasureMode: %d, MeasureRunning: %d, DataListReady: %d\n", (float)m_TimeStamp / 1000000.0f, m_MeasureMode, m_MeasureRunning, m_DataListforHostReady);
		}
		*ListSize = 0;
		return false;
	}

	//Critical section busy
	if (!TryEnterCriticalSection(&m_MeasureLock))
	{
		if (m_DebugOn)
		{
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tGetMeasureResults but Critical Section Busy\n", (float)m_TimeStamp / 1000000.0f);
		}

		*ListSize = 0;
		return false;
	}

	if (pMeasureDataStructArray == NULL)
	{
		//Return size of List
		m_DataListSize = (UINT32)m_vMeasureDataList.size();
		*ListSize = m_DataListSize;

		if (m_DebugOn)
		{
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tGetMeasureResults Size: %d\n", (float)m_TimeStamp / 1000000.0f, m_DataListSize);
		}
	}
	else
	{
		//Return Data
		m_DataListSize = (UINT32)m_vMeasureDataList.size();
		*ListSize = m_DataListSize;
		memcpy(pMeasureDataStructArray, m_vMeasureDataList.data(), sizeof(MeasureDataStruct) * *ListSize);
		m_DataListforHostReady = false;

		if (m_DebugOn)
		{
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tGetMeasureResults Data\n", (float)m_TimeStamp / 1000000.0f);
		}
	}

	LeaveCriticalSection(&m_MeasureLock);
	return true;
}

#pragma endregion

#pragma region Line Direction

//Lines Horizontal
STDMETHODIMP C_rx_AlignFilter::SetLinesHorizontal(BOOL LinesHorizontal)
{
	CAutoLock cAutolock(m_pLock);

	m_LinesHorizontal = LinesHorizontal;

	if (m_DebugOn)
	{
		m_MeasureTime = std::chrono::steady_clock::now();
		m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
		printf("%6.6f\tSet Lines Horizontal: %d\n", (float)m_TimeStamp / 1000000.0f, m_LinesHorizontal);
	}

	SetDirty(TRUE);
	return NOERROR;
}
STDMETHODIMP_(BOOL) C_rx_AlignFilter::GetLinesHorizontal()
{
//	CAutoLock cAutolock(m_pLock);
	return m_LinesHorizontal;
}

//Line Direction
STDMETHODIMP C_rx_AlignFilter::SetUpsideDown(BOOL UpsideDown)
{
	CAutoLock cAutolock(m_pLock);

	m_UpsideDown = UpsideDown;

	if (m_DebugOn)
	{
		m_MeasureTime = std::chrono::steady_clock::now();
		m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
		printf("%6.6f\tSet Lines UpsideDown: %d\n", (float)m_TimeStamp / 1000000.0f, m_UpsideDown);
	}

	SetDirty(TRUE);
	return NOERROR;
}
STDMETHODIMP_(BOOL) C_rx_AlignFilter::GetUpsideDown()
{
	//	CAutoLock cAutolock(m_pLock);
	return m_UpsideDown;
}

#pragma endregion

#pragma endregion


