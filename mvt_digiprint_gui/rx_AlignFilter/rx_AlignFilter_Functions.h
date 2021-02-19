#pragma once


#include "stdafx.h"
#include <shlobj.h>
#include <locale>
#include <codecvt>
using namespace std;

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


//Debugs
void C_rx_AlignFilter::CallbackDebug(const char* MessageFormat, ...)
{
	if (m_HostHwnd == NULL || !m_CallbackDebug)
	{
		if (m_DebugOn && m_HostHwnd == NULL)
		{
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tCallbackDebug called but no HostPointer\n", (float)m_TimeStamp / 1000000.0f);
		}
		return;
	}

	char msg[255];
	va_list args;
	va_start(args, MessageFormat);
	vsnprintf(msg, sizeof(msg), MessageFormat, args);
	va_end(args);

	COPYDATASTRUCT CopyData;
	CopyData.dwData = WP_CallBackDebug;
	CopyData.cbData = (DWORD)strlen(msg);
	CopyData.lpData = msg;

	if (m_DebugOn)
	{
		m_MeasureTime = std::chrono::steady_clock::now();
		m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
		printf("%6.6f\tCallbackDebug: cbData: %d, lpData: %s\n", (float)m_TimeStamp / 1000000.0f, CopyData.cbData, (char*)CopyData.lpData);
	}

	if (m_LogToFile) LogToFile("CallbackDebug: cbData: %d, lpData: %s", CopyData.cbData, (char*)CopyData.lpData);

	SendMessage(m_HostHwnd, WM_COPYDATA, (WPARAM)(HWND)hMySelf, (LPARAM)(LPVOID)&CopyData);
}

void C_rx_AlignFilter::LogToFile(char* MessageFormat, ...)
{
	if (DebugLogPath == L"") return;

	char msg[255];
	va_list args;
	va_start(args, MessageFormat);
	vsnprintf(msg, sizeof(msg), MessageFormat, args);
	va_end(args);

	m_MeasureTime = std::chrono::steady_clock::now();
	m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();

	FILE* temp_file;
	temp_file = _wfopen(DebugLogPath, L"a");
	fprintf(temp_file, "%6.6f\t%s\n", (float)m_TimeStamp / 1000000.0f, msg);
	fclose(temp_file);
}

void C_rx_AlignFilter::LogToFile(wchar_t* MessageFormat, ...)
{
	if (DebugLogPath == L"") return;

	wchar_t msg[255];
	va_list args;
	va_start(args, MessageFormat);
	_vsnwprintf(msg, sizeof(msg), MessageFormat, args);
	va_end(args);

	m_MeasureTime = std::chrono::steady_clock::now();
	m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();

	FILE* temp_file;
	temp_file = _wfopen(DebugLogPath, L"a");
	fwprintf(temp_file, L"%6.6f\t%ls\n", (float)m_TimeStamp / 1000000.0f, msg);
	fclose(temp_file);
}

//Graphics debug
void C_rx_AlignFilter::DebugLog(const char* MessageFormat, ...)
{
	char msg[255];
	va_list args;
	va_start(args, MessageFormat);
	vsnprintf(msg, sizeof(msg), MessageFormat, args);
	va_end(args);

	m_MeasureTime = std::chrono::steady_clock::now();
	m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();

	FILE* temp_file;
	temp_file = _wfopen(DebugLogPath, L"a");
	fprintf(temp_file, "%6.6f\tlpData: %s\n", (float)m_TimeStamp / 1000000.0f, msg);
	fclose(temp_file);
}

void DebugUserLog(wchar_t* Description, wchar_t* ValueString)
{
	wchar_t my_documents[MAX_PATH];
	if(SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, my_documents) != S_OK)
	{
		MessageBox(NULL, TEXT("my_documents not found"), TEXT("my_documents"), MB_OK);
	}
	else
	{
		wcsncat(my_documents, TEXT("\\rx_AlignFilter_Log.txt"), 19);
		std::wofstream file(my_documents, std::ios::out | std::ios::binary | std::ios::app);
		file << Description << " : " << ValueString << std::endl;
		file.close();
	}
}


//Sort Blobs
BOOL CompareX(const QualifyStruct Qualyfier_1, const QualifyStruct Qualyfier_2)
{
	return(Qualyfier_1.PosX > Qualyfier_2.PosX);
}

BOOL CompareY(const QualifyStruct Qualyfier_1, const QualifyStruct Qualyfier_2)
{
	return(Qualyfier_1.PosY > Qualyfier_2.PosY);
}


//OCR
BOOL CompareXPos(const OcrResultStruct Qualyfier_1, const OcrResultStruct Qualyfier_2)
{
	return(Qualyfier_1.XPos < Qualyfier_2.XPos);
}

BOOL CompareYPos(const OcrResultStruct Qualyfier_1, const OcrResultStruct Qualyfier_2)
{
	return(Qualyfier_1.YPos < Qualyfier_2.YPos);
}

BOOL CompareXPosReverse(const OcrResultStruct Qualyfier_1, const OcrResultStruct Qualyfier_2)
{
	return(Qualyfier_1.XPos > Qualyfier_2.XPos);
}

BOOL CompareYPosReverse(const OcrResultStruct Qualyfier_1, const OcrResultStruct Qualyfier_2)
{
	return(Qualyfier_1.YPos > Qualyfier_2.YPos);
}

BOOL C_rx_AlignFilter::TrainOCR()
{
	std::string OcrLearnedPath = std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(LearnedOcrPath);
	FileStorage fs(OcrLearnedPath, cv::FileStorage::READ);

	Learn_Samples.release();
	Learn_Responses.release();

	if (fs.isOpened())
	{
		fs["Samples"] >> Learn_Samples;
		fs["Responses"] >> Learn_Responses;
		fs.release();

		if (Learn_Samples.empty() || Learn_Responses.empty())
		{
			if (m_DebugOn)
			{
				m_MeasureTime = std::chrono::steady_clock::now();
				m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
				printf("%6.6f\tTrainOCR failed: Learned Samples or Responses empty\n", (float)m_TimeStamp / 1000000.0f);
			}
			if (m_LogToFile) LogToFile(L"TrainOCR failed: Learned Samples or Responses empty");
			CallbackDebug("TrainOCR failed: Learned Samples or Responses empty");
			return false;
		}

		KNearOcr = (Ptr<KNearest>)KNearest::create();

		try
		{
			if (KNearOcr->train(Learn_Samples, 0, Learn_Responses))
			{
				return true;
			}
			if (m_DebugOn)
			{
				m_MeasureTime = std::chrono::steady_clock::now();
				m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
				printf("%6.6f\tTrainOCR failed (no message)\n", (float)m_TimeStamp / 1000000.0f);
			}
			if (m_LogToFile) LogToFile(L"TrainOCR failed (no message)");
			CallbackDebug("TrainOCR failed (no message)");
			return false;
		}
		catch (Exception exep)
		{
			const char* Msg = exep.what();
			if (m_DebugOn)
			{
				m_MeasureTime = std::chrono::steady_clock::now();
				m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
				printf("%6.6f\tTrainOCR failed: %s\n", (float)m_TimeStamp / 1000000.0f, Msg);
			}
			if (m_LogToFile) LogToFile("TrainOCR failed: %s", Msg);
			CallbackDebug("TrainOCR failed: %s", Msg);
			return false;
		}
	}
	if (m_DebugOn)
	{
		m_MeasureTime = std::chrono::steady_clock::now();
		m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
		printf("%6.6f\tTrainOCR failed: could not open Learned Samples or Responses\n", (float)m_TimeStamp / 1000000.0f);
	}
	if (m_LogToFile) LogToFile("TrainOCR failed: could not open Learned Samples or Responses");
	CallbackDebug("TrainOCR failed: could not open Learned Samples or Responses");
	return false;
}


void GetCvMousePos(int event, int x, int y, int flags, void* userdata)
{
	if (event == EVENT_LBUTTONDOWN)
	{
		cv::Point3i* MousePos = (cv::Point3i*)userdata;
		MousePos->x = x;
		MousePos->y = y;
		MousePos->z = 1;
	}
	else if (event == EVENT_MOUSEMOVE)
	{
		if (flags == EVENT_FLAG_LBUTTON)
		{
			cv::Point3i* MousePos = (cv::Point3i*)userdata;
			MousePos->x = x;
			MousePos->y = y;
			MousePos->z = 2;
		}
		else if (flags == EVENT_FLAG_RBUTTON)
		{
			cv::Point3i* MousePos = (cv::Point3i*)userdata;
			MousePos->x = x;
			MousePos->y = y;
			MousePos->z = 3;
		}
	}
}


//OpenCL

BOOL C_rx_AlignFilter::FindPlatformDevice(cl_platform_id* ClPlatform, int* PlatformNum, cl_device_id* ClDevice, int* DeviceNum, char* DeviceName)
{
	//Find and select Platform and Device
	cl_int cl_Error;
	struct SelectorStruct
	{
		int PlatformNum;
		cl_platform_id PlatformID;
		int DeviceNum;
		cl_device_id DeviceID;
		int SelectorValue;
		char DeviceName[256];
		char DeviceVendor[256];
		char OpenClVersion[256];
		UINT ComputeUnits;
		UINT MaxWGSize;
		UINT MaxFreq;
		cl_device_type DeviceType;
	};
	vector<SelectorStruct> Selector;

	//Platforms
	vector<cl::Platform> PlatformList;
	cl_Error = cl::Platform::get(&PlatformList);	//All available Platforms
	if (cl_Error != CL_SUCCESS)
	{
		TCHAR MsgMsg[10];
		_itow_s((int)cl_Error, MsgMsg, 10, 10);

		if (m_DebugOn)
		{
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tFindPlatformDevice: Could not get Platform List: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		if (m_LogToFile) LogToFile(L"FindPlatformDevice: Could not get Platform List: %ls", MsgMsg);
		return false;
	}
	if (PlatformList.size() == 0)
	{
		if (m_DebugOn)
		{
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tFindPlatformDevice: No platforms found\n", (float)m_TimeStamp / 1000000.0f);
		}
		if (m_LogToFile) LogToFile(L"FindPlatformDevice: No platforms found");
		return false;
	}

	//Platform ID's
	vector<cl_platform_id> PlatformIdList;
	PlatformIdList.resize(PlatformList.size());
	clGetPlatformIDs((cl_uint)PlatformList.size(), PlatformIdList.data(), NULL);

	//Devices in Platforms
	for (int i = 0; i < PlatformList.size(); i++)
	{
		vector<cl::Device> DeviceList;
		cl_Error = PlatformList[i].getDevices(CL_DEVICE_TYPE_CPU | CL_DEVICE_TYPE_GPU, &DeviceList);	//All available Devices of this Platform
		vector<cl_device_id> DeviceIdList;
		DeviceIdList.resize(DeviceList.size());
		cl_Error = clGetDeviceIDs(PlatformIdList[i], CL_DEVICE_TYPE_CPU | CL_DEVICE_TYPE_GPU, (cl_uint)DeviceList.size(), DeviceIdList.data(), NULL);

		for (int j = 0; j < DeviceList.size(); j++)
		{
			cl_bool DeviceAvailable;
			cl_Error = clGetDeviceInfo(DeviceIdList[j], CL_DEVICE_AVAILABLE, sizeof(cl_bool), &DeviceAvailable, NULL);
			if (DeviceAvailable)
			{
				SelectorStruct CurrentDevice;
				CurrentDevice.PlatformNum = i;
				CurrentDevice.PlatformID = PlatformIdList[i];
				CurrentDevice.DeviceNum = j;
				CurrentDevice.DeviceID = DeviceIdList[j];

				//Get Details
				cl_Error = clGetDeviceInfo(DeviceIdList[j], CL_DEVICE_VENDOR, sizeof(CurrentDevice.DeviceVendor), &CurrentDevice.DeviceVendor, NULL);
				cl_Error = clGetDeviceInfo(DeviceIdList[j], CL_DEVICE_NAME, sizeof(CurrentDevice.DeviceName), &CurrentDevice.DeviceName, NULL);
				cl_Error = clGetDeviceInfo(DeviceIdList[j], CL_DEVICE_VERSION, sizeof(CurrentDevice.OpenClVersion), &CurrentDevice.OpenClVersion, NULL);	//OpenCL 1.2
				cl_Error = clGetDeviceInfo(DeviceIdList[j], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_uint), (UINT*)&CurrentDevice.ComputeUnits, NULL);		//e.g. 20
				cl_Error = clGetDeviceInfo(DeviceIdList[j], CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t), (UINT*)&CurrentDevice.MaxWGSize, NULL);			//e.g.512
				cl_Error = clGetDeviceInfo(DeviceIdList[j], CL_DEVICE_TYPE, sizeof(cl_device_type), &CurrentDevice.DeviceType, NULL);
				cl_Error = clGetDeviceInfo(DeviceIdList[j], CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(cl_uint), &CurrentDevice.MaxFreq, NULL);
				Selector.push_back(CurrentDevice);
			}
		}
	}

	//Select Platform/Device

	//	Rating:
	//	CPU = 1
	//	GPGPU = 2
	//		MaxFreq *
	//			ComputeUnits *

	vector<int> SelectorValueList;
	for (int i = 0; i < Selector.size(); i++)
	{
		//GPU or CPU
		if (Selector[i].DeviceType == CL_DEVICE_TYPE_GPU) Selector[i].SelectorValue = 2;
		else if (Selector[i].DeviceType == CL_DEVICE_TYPE_CPU) Selector[i].SelectorValue = 1;
		else Selector[i].SelectorValue = 0;

		//as I had strange problems with NVIDIA
		//char* NvidFound = NULL;
		//NvidFound = strstr(Selector[i].DeviceVendor, "NVIDIA");
		//if(NvidFound != NULL)Selector[i].SelectorValue = 0;

		Selector[i].SelectorValue *= Selector[i].MaxFreq;;
		Selector[i].SelectorValue *= Selector[i].ComputeUnits;

		SelectorValueList.push_back(Selector[i].SelectorValue);
	}

	INT64 BestDeviceNum = max_element(SelectorValueList.begin(), SelectorValueList.end()) - SelectorValueList.begin();

	//Debug Specify Device manually ************************************************************************************************************************************
	//BestDeviceNum = 1;
	//DebugUserLog(TEXT("Best Device manual = "), TEXT("1"));

	*ClPlatform = Selector[BestDeviceNum].PlatformID;
	*PlatformNum = Selector[BestDeviceNum].PlatformNum;
	*ClDevice = Selector[BestDeviceNum].DeviceID;
	*DeviceNum = Selector[BestDeviceNum].DeviceNum;
	int StrLen = (int)strlen(Selector[BestDeviceNum].DeviceName);
	strcpy_s(DeviceName, (rsize_t)StrLen + 1, Selector[BestDeviceNum].DeviceName);


	//Debug Show Decision *********************************************************************************************************************************************
	if (m_DebugOn)
	{
		for (int i = 0; i < SelectorValueList.size(); i++)
		{
			wchar_t* device_wString = new wchar_t[1024];

			_itow_s((int)Selector[i].PlatformNum, device_wString, 10, 10);
			DebugUserLog(TEXT("\nPlatformNum"), device_wString);

			_itow_s((int)Selector[i].DeviceNum, device_wString, 10, 10);
			DebugUserLog(TEXT("DeviceNum"), device_wString);

			MultiByteToWideChar(CP_ACP, 0, (LPCCH)Selector[i].DeviceName, -1, device_wString, 1024);
			DebugUserLog(TEXT("CL_DEVICE_NAME"), device_wString);

			MultiByteToWideChar(CP_ACP, 0, (LPCCH)Selector[i].DeviceVendor, -1, device_wString, 1024);
			DebugUserLog(TEXT("CL_DEVICE_VENDOR"), device_wString);

			MultiByteToWideChar(CP_ACP, 0, (LPCCH)Selector[i].OpenClVersion, -1, device_wString, 1024);
			DebugUserLog(TEXT("OpenClVersion"), device_wString);

			_itow_s((int)Selector[i].MaxFreq, device_wString, 10, 10);
			DebugUserLog(TEXT("Device maximum Freq"), device_wString);

			_itow_s((int)Selector[i].ComputeUnits, device_wString, 10, 10);
			DebugUserLog(TEXT("Device ComputeUnits"), device_wString);

			_itow_s((int)Selector[i].MaxWGSize, device_wString, 10, 10);
			DebugUserLog(TEXT("Device maximum WG Size"), device_wString);

			_ltow_s((long)Selector[i].SelectorValue, device_wString, 10, 10);
			DebugUserLog(TEXT("Selector Value"), device_wString);

			delete[] device_wString;
		}

		wchar_t* device_wString = new wchar_t[1024];
		MultiByteToWideChar(CP_ACP, 0, (LPCCH)Selector[BestDeviceNum].DeviceName, -1, device_wString, 1024);
		DebugUserLog(TEXT("\nBest Device"), device_wString);
		DebugUserLog(TEXT("\n"), TEXT("***************************************\n\n"));

		delete[] device_wString;
	}


	return true;
}

cl_context C_rx_AlignFilter::CreateContext(cl_platform_id ClPlatformId, int PlatformNum, cl_device_id ClDeviceId, int DeviceNum)
{
	//Context for selected Device
	cl_int cl_Error = 0;
	cl_context_properties contextProperties[] = { CL_CONTEXT_PLATFORM,(cl_context_properties)(ClPlatformId),0 };
	cl_context context = clCreateContext(contextProperties, 1, &ClDeviceId, nullptr, nullptr, &cl_Error);
	if (cl_Error != CL_SUCCESS)
	{
		TCHAR MsgMsg[10];
		_itow_s((int)cl_Error, MsgMsg, 10, 10);
		if (m_DebugOn)
		{
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tCreateContext: Could not create Context: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		if (m_LogToFile) LogToFile(L"CreateContext: Could not create Context: %ls", MsgMsg);
		exit(-1);
	}
	return context;
}

cl_command_queue C_rx_AlignFilter::CreateCommandQueue(cl_context context, cl_device_id ClDeviceId, int DeviceNum)
{
	//Command-Queue for selected Device
	cl_int cl_Error;
	cl_command_queue queue = clCreateCommandQueue(context, ClDeviceId, CL_QUEUE_PROFILING_ENABLE, &cl_Error);
	if (cl_Error != CL_SUCCESS)
	{
		TCHAR MsgMsg[10];
		_itow_s((int)cl_Error, MsgMsg, 10, 10);
		if (m_DebugOn)
		{
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tCreateCommandQueue: Could not create Command-Queue: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		if (m_LogToFile) LogToFile(L"CreateCommandQueue: Could not create Command-Queue: %ls", MsgMsg);

		exit(-1);
	}
	return queue;
}

cl_program C_rx_AlignFilter::MakeProgram(int KernelSourceID, cl_context context, cl_device_id DeviceId, int DeviceNum)
{
	//Read Kernel-Code into char-Buffer
	cl_int cl_Error;

	//Load Kernel from Resources
	HRSRC hResult = FindResource(hMySelf, MAKEINTRESOURCE(KernelSourceID), _T("OPENCL"));
	if (hResult == NULL)
	{
		if (m_DebugOn)
		{
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tMakeProgram: KernelSource-Resource not found\n", (float)m_TimeStamp / 1000000.0f);
		}
		if (m_LogToFile) LogToFile(L"MakeProgram: KernelSource-Resource not found");
		exit(-1);
	}

	HGLOBAL rcData = LoadResource(hMySelf, hResult);
	if (rcData == NULL)
	{
		if (m_DebugOn)
		{
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tMakeProgram: Module Pointer-Resource not found\n", (float)m_TimeStamp / 1000000.0f);
		}
		if (m_LogToFile) LogToFile(L"MakeProgram: Module Pointer-Resource not found");
		exit(-1);
	}
	DWORD KernelSize = SizeofResource(hMySelf, hResult);
	wchar_t* KernelBuffer = NULL;
	KernelBuffer = (wchar_t*)LockResource(rcData);
	if (KernelBuffer == NULL || KernelSize == 0)
	{
		if (m_DebugOn)
		{
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tMakeProgram: Could not load Kernel Source\n", (float)m_TimeStamp / 1000000.0f);
		}
		if (m_LogToFile) LogToFile(L"MakeProgram: Could not load Kernel Source");
		exit(-1);
	}

	//Create Program
	cl_program Program = clCreateProgramWithSource(context, 1, (const char**)&KernelBuffer, NULL, &cl_Error);
	if (cl_Error != CL_SUCCESS)
	{
		TCHAR MsgMsg[10];
		_itow_s((int)cl_Error, MsgMsg, 10, 10);
		if (m_DebugOn)
		{
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tMakeProgram: Could not create Program Binarize: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		if (m_LogToFile) LogToFile(L"MakeProgram: Could not create Program Binarize: %ls", MsgMsg);
		exit(-1);
	}

	//Build Program
	cl_Error = clBuildProgram(Program, 1, &DeviceId, NULL/*"-cl-denorms-are-zero"*/, NULL, NULL);
	if (cl_Error != CL_SUCCESS)
	{
		TCHAR MsgMsg[10];
		_itow_s((int)cl_Error, MsgMsg, 10, 10);
		if (m_DebugOn)
		{
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tMakeProgram: Could not build Program Full-Alignment: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		if (m_LogToFile) LogToFile(L"MakeProgram: Could not build Program Full-Alignment: %ls", MsgMsg);

		if (m_DebugOn)
		{
			size_t log_size;
			clGetProgramBuildInfo(Program, DeviceId, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);

			// Allocate memory for the log
			char* log = (char*)malloc(log_size);

			// Get the log
			clGetProgramBuildInfo(Program, DeviceId, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);

			// Print the log
			wchar_t my_documents[MAX_PATH];
			if (SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, my_documents) != S_OK)
			{
					m_MeasureTime = std::chrono::steady_clock::now();
					m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
					printf("%6.6f\tMakeProgram: Folder \"my_documents\" not found\n", (float)m_TimeStamp / 1000000.0f);
					if (m_LogToFile) LogToFile(L"MakeProgram: Folder \"my_documents\" not found");
			}
			else
			{
				wcsncat(my_documents, L"\\rx_AlignFilter_Log.txt", MAX_PATH);
				std::fstream file(my_documents, std::ios::out | std::ios::binary | std::ios::app);
				file << "BuildLog Start:" << std::endl;
				file << log << std::endl;
				file << "BuildLog End" << std::endl;
				file << "\n***************************************\n\n" << std::endl;
				file.close();
			}
		}

		exit(-1);
	}

	return Program;
}

BOOL C_rx_AlignFilter::CreateKernels(cl_program Program)
{
	//Create all OpenCl Kernels
	cl_int cl_Error = 0;

	//Create Full Histogram-Kernel
	ClFullHistogramKernel = clCreateKernel(Program, "HistogramFull", &cl_Error);
	if (cl_Error != CL_SUCCESS)
	{
		TCHAR MsgMsg[10];
		_itow_s((int)cl_Error, MsgMsg, 10, 10);
		if (m_DebugOn)
		{
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tCreateKernels: Could not create OpenCL Kernel FullHistogram: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		if (m_LogToFile) LogToFile(L"CreateKernels: Could not create OpenCL Kernel FullHistogram: %ls", MsgMsg);
		return false;
	}

	//Create Block Histogram-Kernel
	ClBlockHistogramKernel = clCreateKernel(Program, "HistogramBlock", &cl_Error);
	if (cl_Error != CL_SUCCESS)
	{
		TCHAR MsgMsg[10];
		_itow_s((int)cl_Error, MsgMsg, 10, 10);
		if (m_DebugOn)
		{
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tCreateKernels: Could not create OpenCL Kernel HistogramBlock: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		if (m_LogToFile) LogToFile(L"CreateKernels: Could not create OpenCL Kernel HistogramBlock: %ls", MsgMsg);
		return false;
	}

	//Create Join Histogram-Kernel
	ClJoinHistogramKernel = clCreateKernel(Program, "JoinHistogram", &cl_Error);
	if (cl_Error != CL_SUCCESS)
	{
		TCHAR MsgMsg[10];
		_itow_s((int)cl_Error, MsgMsg, 10, 10);
		if (m_DebugOn)
		{
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tCreateKernels: Could not create OpenCL Kernel JoinHistogram: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		if (m_LogToFile) LogToFile(L"CreateKernels: Could not create OpenCL Kernel JoinHistogram: %ls", MsgMsg);
		return false;
	}

	//Create Smoothen Histogram-Kernel
	ClSmoothenHistogramKernel = clCreateKernel(Program, "SmoothenHistogram", &cl_Error);
	if (cl_Error != CL_SUCCESS)
	{
		TCHAR MsgMsg[10];
		_itow_s((int)cl_Error, MsgMsg, 10, 10);
		if (m_DebugOn)
		{
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tCreateKernels: Could not create OpenCL Kernel SmoothenHistogram: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		if (m_LogToFile) LogToFile(L"CreateKernels: Could not create OpenCL Kernel SmoothenHistogram: %ls", MsgMsg);
		return false;
	}

	//Create Show Histogram-Kernel Version
	ClShowHistogramKernel = clCreateKernel(Program, "ShowHistogram", &cl_Error);
	if (cl_Error != CL_SUCCESS)
	{
		TCHAR MsgMsg[10];
		_itow_s((int)cl_Error, MsgMsg, 10, 10);
		if (m_DebugOn)
		{
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tCreateKernels: Could not create OpenCL Kernel ShowHistogram: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		if (m_LogToFile) LogToFile(L"CreateKernels: Could not create OpenCL Kernel ShowHistogram: %ls", MsgMsg);
		return false;
	}

	//Create Binarize-Kernel
	ClBinarizeKernel = clCreateKernel(Program, "Binarize", &cl_Error);
	if (cl_Error != CL_SUCCESS)
	{
		TCHAR MsgMsg[10];
		_itow_s((int)cl_Error, MsgMsg, 10, 10);
		if (m_DebugOn)
		{
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tCreateKernels: Could not create OpenCL Kernel Binarize: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		if (m_LogToFile) LogToFile(L"CreateKernels: Could not create OpenCL Kernel Binarize: %ls", MsgMsg);
		return false;
	}

	//Create Multi Color Histogram-Kernel
	ClColorHistogramKernel = clCreateKernel(Program, "ColorHistogram", &cl_Error);
	if (cl_Error != CL_SUCCESS)
	{
		TCHAR MsgMsg[10];
		_itow_s((int)cl_Error, MsgMsg, 10, 10);
		if (m_DebugOn)
		{
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tCreateKernels: Could not create OpenCL Kernel ColorHistogram: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		if (m_LogToFile) LogToFile(L"CreateKernels: Could not create OpenCL Kernel ColorHistogram: %ls", MsgMsg);
		return false;
	}

	//Create Multi Color Binarize-Kernel
	ClColorBinarizeKernel = clCreateKernel(Program, "ColorBinarize", &cl_Error);
	if (cl_Error != CL_SUCCESS)
	{
		TCHAR MsgMsg[10];
		_itow_s((int)cl_Error, MsgMsg, 10, 10);
		if (m_DebugOn)
		{
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tCreateKernels: Could not create OpenCL Kernel ColorBinarize: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		if (m_LogToFile) LogToFile(L"CreateKernels: Could not create OpenCL Kernel ColorBinarize: %ls", MsgMsg);
		return false;
	}

	//Create Multi Color Threshold-Lines-Kernel
	ClColorThresholdLinesKernel = clCreateKernel(Program, "ColorThresholdLines", &cl_Error);
	if (cl_Error != CL_SUCCESS)
	{
		TCHAR MsgMsg[10];
		_itow_s((int)cl_Error, MsgMsg, 10, 10);
		if (m_DebugOn)
		{
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tCreateKernels: Could not create OpenCL Kernel ColorThresholdLines: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		if (m_LogToFile) LogToFile(L"CreateKernels: Could not create OpenCL Kernel ColorThresholdLines: %ls", MsgMsg);
		return false;
	}

	//Create Multi Color Threshold-Averaging-Kernel
	ClColorThresholdAverageKernel = clCreateKernel(Program, "ColorThresholdAverage", &cl_Error);
	if (cl_Error != CL_SUCCESS)
	{
		TCHAR MsgMsg[10];
		_itow_s((int)cl_Error, MsgMsg, 10, 10);
		if (m_DebugOn)
		{
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tCreateKernels: Could not create OpenCL Kernel ColorThresholdAverage: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		if (m_LogToFile) LogToFile(L"CreateKernels: Could not create OpenCL Kernel ColorThresholdAverage: %ls", MsgMsg);
		return false;
	}

	//Create Multi Color Show-Histogramg-Kernel
	ClShowColorHistogramKernel = clCreateKernel(Program, "ColorShowHistogram", &cl_Error);
	if (cl_Error != CL_SUCCESS)
	{
		TCHAR MsgMsg[10];
		_itow_s((int)cl_Error, MsgMsg, 10, 10);
		if (m_DebugOn)
		{
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tCreateKernels: Could not create OpenCL Kernel ColorShowHistogram: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		if (m_LogToFile) LogToFile(L"CreateKernels: Could not create OpenCL Kernel ColorShowHistogram: %ls", MsgMsg);
		return false;
	}

	//Create RGB-Block Histogram-Kernel
	ClRGBBlockHistogramKernel = clCreateKernel(Program, "RGBBlockHistogram", &cl_Error);
	if (cl_Error != CL_SUCCESS)
	{
		TCHAR MsgMsg[10];
		_itow_s((int)cl_Error, MsgMsg, 10, 10);
		if (m_DebugOn)
		{
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tCreateKernels: Could not create OpenCL Kernel RGBBlockHistogram: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		if (m_LogToFile) LogToFile(L"CreateKernels: Could not create OpenCL Kernel RGBBlockHistogram: %ls", MsgMsg);
		return false;
	}

	//Create Show RGBHistogram-Kernel Version
	ClShowRGBHistogramKernel = clCreateKernel(Program, "ShowRGBHistogram", &cl_Error);
	if (cl_Error != CL_SUCCESS)
	{
		TCHAR MsgMsg[10];
		_itow_s((int)cl_Error, MsgMsg, 10, 10);
		if (m_DebugOn)
		{
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tCreateKernels: Could not create OpenCL Kernel ShowRGBHistogram: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		if (m_LogToFile) LogToFile(L"CreateKernels: Could not create OpenCL Kernel ShowRGBHistogram: %ls", MsgMsg);
		return false;
	}

	//Create Join RGBHistogram-Kernel
	ClJoinRGBHistogramKernel = clCreateKernel(Program, "JoinRGBHistogram", &cl_Error);
	if (cl_Error != CL_SUCCESS)
	{
		TCHAR MsgMsg[10];
		_itow_s((int)cl_Error, MsgMsg, 10, 10);
		if (m_DebugOn)
		{
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tCreateKernels: Could not create OpenCL Kernel JoinRGBHistogram: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		if (m_LogToFile) LogToFile(L"CreateKernels: Could not create OpenCL Kernel JoinRGBHistogram: %ls", MsgMsg);
		return false;
	}

	//Create Smoothen RGBHistogram-Kernel
	ClSmoothenRGBHistogramKernel = clCreateKernel(Program, "SmoothenRGBHistogram", &cl_Error);
	if (cl_Error != CL_SUCCESS)
	{
		TCHAR MsgMsg[10];
		_itow_s((int)cl_Error, MsgMsg, 10, 10);
		if (m_DebugOn)
		{
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tCreateKernels: Could not create OpenCL Kernel SmoothenRGBHistogram: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		if (m_LogToFile) LogToFile(L"CreateKernels: Could not create OpenCL Kernel SmoothenRGBHistogram: %ls", MsgMsg);
		return false;
	}

	//Create RGBColorize-Kernel
	ClColorizeRGBKernel = clCreateKernel(Program, "ColorizeRGB", &cl_Error);
	if (cl_Error != CL_SUCCESS)
	{
		TCHAR MsgMsg[10];
		_itow_s((int)cl_Error, MsgMsg, 10, 10);
		if (m_DebugOn)
		{
			m_MeasureTime = std::chrono::steady_clock::now();
			m_TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>(m_MeasureTime - m_DebugStartTime).count();
			printf("%6.6f\tCreateKernels: Could not create OpenCL Kernel RGBColorize: %ls\n", (float)m_TimeStamp / 1000000.0f, MsgMsg);
		}
		if (m_LogToFile) LogToFile(L"CreateKernels: Could not create OpenCL Kernel RGBColorize: %ls", MsgMsg);
		return false;
	}


	return true;
}


//Binarize

int C_rx_AlignFilter::CalcThreshold(UINT* Histogram, UINT* Peak_1_Val, UINT * Peak_1_Pos, UINT* Peak_2_Val, UINT* Peak_2_Pos, UINT* Threshold)
{
	//returns:
	//			 0	2 peaks and valley between found
	//			-1	no peak found, keep old threshold
	//			-2	only one peak found, threshold to middle of rising slope
	//			-3	at least one peak is 0, keep old threshold
	//			-4	no valley between peaks, use middle bewtween the peaks
	//			-5  only one peak found, but middle of rising slope (should not be possible)

	//find all Peaks and Valleys
	int Band = 3;
	bool PeakList[256] = { 0 };
	bool ValleyList[256] = { 0 };
	int NumPeaks = 0;
	int NumValleys = 0;
	bool SlopeRising = false;
	bool SlopeFalling = false;
	int SlopeRiseCount = 0;
	int SlopeFallCount = 0;
	int SlopeList[256] = { 0 };


	//A peak is the max Value between a positive and a negative Slope, a Valley the min Value between a negative and a positive Slope


	//First item
	for (int j = 0; j < Band; j++)
	{
		SlopeList[0] += (Histogram[j + 1] - Histogram[j]);
	}
	//If first item has a falling slope, it could be a peak
	if(SlopeList[0] < 0) SlopeRising = true;
	//If first item has a rising slope, it could be a valley
	else if (SlopeList[0] > 0) SlopeFalling = true;
	//find all slope, peaks and valleys
	for (int i = 0; i < 256; i++)
	{
		//Slope averaged over +/- Band
		for (int j = max(0, i - Band); j < min(i + Band, 254 - Band); j++)
		{
			SlopeList[i] += (Histogram[j + 1] - Histogram[j]);
		}

		//Count rising / falling steps
		if (SlopeList[i] > 0)
		{
			SlopeRiseCount++;
			SlopeFallCount = 0;
		}
		else if (SlopeList[i] < 0)
		{
			SlopeFallCount++;
			SlopeRiseCount = 0;
		}

		//Check for valid rising slope
		if (SlopeRiseCount >= Band)
		{
			SlopeRising = true;
			//Check for change of direction => valley
			if (SlopeFalling)
			{
				SlopeFalling = false;
				ValleyList[max(0, i - Band)] = true;
				NumValleys++;
			}
		}
		//Check for valid falling slope
		else if (SlopeFallCount >= Band)
		{
			SlopeFalling = true;
			//check for change of direction => peak
			if (SlopeRising)
			{
				SlopeRising = false;
				PeakList[max(0, i - Band)] = true;
				NumPeaks++;
			}
		}
	}
	//if last item has a rising slope, it could be a peak
	if (SlopeRising)
	{
		if (SlopeList[255] > 0)
		{
			PeakList[255] = true;
			NumPeaks++;
		}
		else if (SlopeList[255] == 0)
		{
			if (SlopeList[254] > 0)
			{
				PeakList[254] = true;
				NumPeaks++;
			}
			else if (SlopeList[254] == 0)
			{
				if (SlopeList[253] > 0)
				{
					PeakList[254] = true;
					NumPeaks++;
				}
				else if (SlopeList[253] == 0)
				{
					if (SlopeList[252] > 0)
					{
						PeakList[252] = true;
						NumPeaks++;
					}
				}
			}
		}
	}
	//if last item has a falling slope, it could be a valley
	if ((SlopeList[255] < 0) && SlopeFalling)
	{
		ValleyList[255] = true;
		NumValleys++;
	}

	//Weight of a Peak is the sum of Values between Valleys
	UINT WeightList[256];
	UINT Summe = 0;
	bool GotPeak = false;
	int LastPeak = 0;
	int WeightedPeaks = 0;

	//Find Weight of Peaks
	for (int i = 0; i < 256; i++)
	{
		WeightList[i] = 0;
		Summe += Histogram[i];

		//A valley is the "End" of a peak
		if (ValleyList[i])
		{
			if (GotPeak)
			{
				WeightList[LastPeak] = Summe;
				WeightedPeaks++;
			}
			Summe = 0;
			GotPeak = false;
		}
		if (PeakList[i])
		{
			GotPeak = true;
			LastPeak = i;
		}
	}
	//Last point could be a Peak
	if (GotPeak)
	{
		WeightList[LastPeak] = Summe;
		WeightedPeaks++;
	}

	//We need at least 2 Peaks
	if (WeightedPeaks == 0)
	{
		//No Peak found, keep old threshold
		return -1;
	}
	else if (WeightedPeaks < 2)
	{
		//only 1 Peak found, Threshold to middle of rising Slope
		UINT HalfPeak = Histogram[LastPeak] / 2;
		for (int i = 0; i < 256; i++)
		{
			if (Histogram[i] >= HalfPeak)
			{
				*Threshold = i;
				return -2;
			}
		}
		//That should not happen
		return -5;
	}

	//Find 2 heaviest peaks
	*Peak_1_Val = *Peak_2_Val = *Peak_1_Pos = *Peak_2_Pos = 0;

	for (int i = 0; i < 256; i++)
	{
		//New biggest peak
		if (PeakList[i] && WeightList[i] > *Peak_1_Val)
		{
			if (*Peak_1_Val > *Peak_2_Val)
			{
				*Peak_2_Val = *Peak_1_Val;
				*Peak_2_Pos = *Peak_1_Pos;
			}
			//last biggest becomes second
			*Peak_1_Val = WeightList[i];
			*Peak_1_Pos = i;
		}
		//New second biggest peak
		else if (PeakList[i] && WeightList[i] > *Peak_2_Val)
		{
			*Peak_2_Val = WeightList[i];
			*Peak_2_Pos = i;
		}
	}

	//If Peak values are zero we have no valid Peaks
	if (*Peak_1_Val == 0 || *Peak_2_Val == 0) //|| *Peak_1_Pos == 0 || *Peak_2_Pos == 0)
	{
		//keep old threshold
		return -3;
	}

	//Threshold is the deepest valley between the two heaviest Peaks
	UINT ValleyVal = *Peak_1_Val;
	int ValleyFound = -1;
	for (int i = *Peak_1_Pos; i != *Peak_2_Pos; i += ((*Peak_1_Pos < *Peak_2_Pos) ? 1 : -1))
	{
		if (ValleyList[i])
		{
			if (Histogram[i] < ValleyVal) ValleyVal = Histogram[i];
			*Threshold = i;
			ValleyFound = i;
		}
	}
	if (ValleyFound < 0)
	{
		//No Valley between peaks found, use middle bewtween the peaks
		*Threshold = (*Peak_1_Pos + *Peak_2_Pos) / 2;
		return -4;
	}
	else
	{
		*Threshold = ValleyFound;
		return 0;
	}

}

int C_rx_AlignFilter::CalcRGBThreshold(UINT* RGBHistogram, UINT* Peak_1_Val, UINT* Peak_1_Pos, UINT* Peak_2_Val, UINT* Peak_2_Pos, UINT* Threshold)
{
	UINT BHistogram[256];
	UINT GHistogram[256];
	UINT RHistogram[256];

	//Separate Histograms
	for (int i = 0; i < 256; i++)
	{
		BHistogram[i] = RGBHistogram[i];
		GHistogram[i] = RGBHistogram[i + 256];
		RHistogram[i] = RGBHistogram[i + 512];
	}

	CalcThreshold(BHistogram, &Peak_1_Val[0], &Peak_1_Pos[0], &Peak_2_Val[0], &Peak_2_Pos[0], &Threshold[0]);
	CalcThreshold(GHistogram, &Peak_1_Val[1], &Peak_1_Pos[1], &Peak_2_Val[1], &Peak_2_Pos[1], &Threshold[1]);
	CalcThreshold(RHistogram, &Peak_1_Val[2], &Peak_1_Pos[2], &Peak_2_Val[2], &Peak_2_Pos[2], &Threshold[2]);

	return 0;
}

