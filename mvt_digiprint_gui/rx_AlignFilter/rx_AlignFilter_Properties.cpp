//#pragma once
//
//#include <windows.h>
//#include <windowsx.h>
//#include <streams.h>
//#include <commctrl.h>
//#include <olectl.h>
//#include <memory.h>
//#include <stdlib.h>
//#include <stdio.h>
//#include <tchar.h>
//#include "stdafx.h"
//
//#include <chrono>
//#include <ctime>    
//
//#include <strmif.h>
//
//#include "resource.h"
//#include "rx_AlignFilter.h"
//#include "rx_AlignFilter_Properties.h"
//
//
//
//
//
//// CreateInstance, Used by the DirectShow base classes to create instances
//CUnknown *rx_AlignFilter_Properties::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr)
//{
//	ASSERT(phr);
//
//	CUnknown *punk = new rx_AlignFilter_Properties(lpunk, phr);
//
//	if (punk == NULL)
//	{
//		if (phr) *phr = E_OUTOFMEMORY;
//	}
//
//	return punk;
//}
//
//// Constructor
//rx_AlignFilter_Properties::rx_AlignFilter_Properties(LPUNKNOWN pUnk, HRESULT *phr) :
//	CBasePropertyPage(NAME("Full Alignment"), pUnk, IDD_rx_AlignFilter_Property, IDS_TITLE),
//	m_DspTiming(FALSE),
//	m_Calibrate(FALSE)
//{
//	ASSERT(phr);
//} // (Constructor)
//
//// OnReceiveMessage, Handles the messages for our property window
//INT_PTR rx_AlignFilter_Properties::OnReceiveMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
//{
//	switch (uMsg)
//	{
//		case WM_COMMAND:
//		{
//			if (m_bIsInitialized)
//			{
//				//Make Apply-Button visible
//				m_bDirty = TRUE;
//				if (m_pPageSite)
//				{
//					m_pPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);
//				}
//
//				//Dialog Items
//				switch (LOWORD(wParam))
//				{
//					//Dialog
//					case IDC_CHECK_DisplayTiming:
//					{
//						//GetDspTimingValue();
//						//CheckPointer(m_pIPEffect, E_POINTER);
//						//m_pIPEffect->put_DspTiming(m_DspTiming);
//						break;
//					}
//
//					//Binarize
//					case IDC_CHECK_Binarize:
//					{
//						//GetBinarizeValue();
//						//CheckPointer(m_pIPEffect, E_POINTER);
//						//m_pIPEffect->put_Binarize(m_Binarize);
//						break;
//					}
//					case IDC_CHECK_BinarizeAuto:
//					{
//						//GetAutomaticValue();
//						//CheckPointer(m_pIPEffect, E_POINTER);
//						//m_pIPEffect->put_BinarizeAuto(m_Automatic);
//						break;
//					}
//					case IDC_EDIT_Threshold:
//					{
//						//GetThresholdValueText();
//						//CheckPointer(m_pIPEffect, E_POINTER);
//						//m_pIPEffect->put_Threshold(m_Threshold);
//						break;
//					}
//					case IDC_CHECK_ShowHistogram:
//					{
//						//GetShowHistogramValue();
//						//CheckPointer(m_pIPEffect, E_POINTER);
//						//m_pIPEffect->put_ShowHistogram(m_ShowHistogram);
//						break;
//					}
//					case IDC_BUTTON_SaveHistogram:
//					{
//						//CheckPointer(m_pIPEffect, E_POINTER);
//						//m_pIPEffect->set_SaveHistogram();
//						break;
//					}
//
//					//Dilate/Erode
//					case IDC_EDIT_DilateErodes:
//					{
//						//GetDilateErodesValueText();
//						//CheckPointer(m_pIPEffect, E_POINTER);
//						//m_pIPEffect->set_DilateErodes(m_DilateErodes);
//						break;
//					}
//					case IDC_EDIT_ExtraErodes:
//					{
//						//GetExtraErodesValueText();
//						//CheckPointer(m_pIPEffect, E_POINTER);
//						//m_pIPEffect->set_ExtraErodes(m_ExtraErodes);
//						break;
//					}
//					case IDC_EDIT_ErodeSeedX:
//					{
//						//GetErodeSeedXValueText();
//						//CheckPointer(m_pIPEffect, E_POINTER);
//						//m_pIPEffect->set_ErodeSeedX(m_ErodeSeedX);
//						break;
//					}
//					case IDC_EDIT_ErodeSeedY:
//					{
//						//GetErodeSeedYValueText();
//						//CheckPointer(m_pIPEffect, E_POINTER);
//						//m_pIPEffect->set_ErodeSeedY(m_ErodeSeedY);
//						break;
//					}
//
//					//Blob
//					case IDC_BUTTON_CrossColorBlob:
//					{
//						//GetCrossColorBlobDialog(hwnd);
//						//CheckPointer(m_pIPEffect, E_POINTER);
//						//m_pIPEffect->set_CrossColorBlob(m_CrossColorBlob);
//						//InvalidateRect(m_Dlg, NULL, TRUE);
//						//UpdateWindow(m_Dlg);
//						break;
//					}
//
//					//Measurement
//					case IDC_CHECK_OvBlobs:
//					{
//						//GetOverlayBlobsValue();
//						//CheckPointer(m_pIPEffect, E_POINTER);
//						//m_pIPEffect->put_OverlayBlobs(m_OverlayBlobs);
//						break;
//					}
//					case IDC_CHECK_OvCenters:
//					{
//						//GetOverlayCentersValue();
//						//CheckPointer(m_pIPEffect, E_POINTER);
//						//m_pIPEffect->put_OverlayCenters(m_OverlayCenters);
//						break;
//					}
//					case IDC_CHECK_OvValues:
//					{
//						//GetOverlayValuesValue();
//						//CheckPointer(m_pIPEffect, E_POINTER);
//						//m_pIPEffect->put_OverlayValues(m_OverlayValues);
//						break;
//					}
//					case IDC_CHECK_OvDistances:
//					{
//						//GetOverlayDistancesValue();
//						//CheckPointer(m_pIPEffect, E_POINTER);
//						//m_pIPEffect->put_OverlayDistances(m_OverlayDistances);
//						break;
//					}
//					case IDC_RADIO_ShowOriginal:
//					{
//						//GetShowOriginalValue();
//						//CheckPointer(m_pIPEffect, E_POINTER);
//						//m_pIPEffect->put_ShowOriginal(m_ShowOriginal);
//						break;
//					}
//					case IDC_RADIO_ShowProcess:
//					{
//						//GetShowOriginalValue();
//						//CheckPointer(m_pIPEffect, E_POINTER);
//						//m_pIPEffect->put_ShowOriginal(m_ShowOriginal);
//						break;
//					}
//					case IDC_BUTTON_CrossColor:
//					{
//						//GetCrossColorDialog(hwnd);
//						//CheckPointer(m_pIPEffect, E_POINTER);
//						//m_pIPEffect->set_CrossColor(m_CrossColor);
//						//InvalidateRect(m_Dlg, NULL, TRUE);
//						//UpdateWindow(m_Dlg);
//						break;
//					}
//					case IDC_BUTTON_TextColor:
//					{
//						//GetTextColorDialog(hwnd);
//						//CheckPointer(m_pIPEffect, E_POINTER);
//						//m_pIPEffect->set_TextColor(m_TextColor);
//						//InvalidateRect(m_Dlg, NULL, TRUE);
//						//UpdateWindow(m_Dlg);
//						break;
//					}
//					case IDC_BUTTON_BlobColor:
//					{
//						//GetBlobColorDialog(hwnd);
//						//CheckPointer(m_pIPEffect, E_POINTER);
//						//m_pIPEffect->set_BlobColor(m_BlobColor);
//						//InvalidateRect(m_Dlg, NULL, TRUE);
//						//UpdateWindow(m_Dlg);
//						break;
//					}
//					case IDC_BUTTON_Font:
//					{
//						//GetTextFontDialog(hwnd);
//						//CheckPointer(m_pIPEffect, E_POINTER);
//						//m_pIPEffect->set_TextFont(m_TextFont);
//						break;
//					}
//					case IDC_EDIT_BlobAreaDivisor:
//					{
//						//GetBlobAreaDivisorValueText();
//						//CheckPointer(m_pIPEffect, E_POINTER);
//						//m_pIPEffect->set_BlobAreaDivisor(m_BlobAreaDivisor);
//						break;
//					}
//					case IDC_EDIT_BlobAspectLimit:
//					{
//						//GetBlobAspectLimitValueText();
//						//CheckPointer(m_pIPEffect, E_POINTER);
//						//m_pIPEffect->set_BlobAspectLimit(m_BlobAspectLimit);
//						break;
//					}
//					case IDC_RADIO_MeasureAllLines:
//					{
//						//GetMeasureModeValue(0);
//						//CheckPointer(m_pIPEffect, E_POINTER);
//						//m_pIPEffect->set_MeasureMode(m_MeasureMode);
//						break;
//					}
//					case IDC_RADIO_Measure2x4Lines:
//					{
//						//GetMeasureModeValue(1);
//						//CheckPointer(m_pIPEffect, E_POINTER);
//						//m_pIPEffect->set_MeasureMode(m_MeasureMode);
//						break;
//					}
//					case IDC_RADIO_Measure2Lines:
//					{
//						//GetMeasureModeValue(2);
//						//CheckPointer(m_pIPEffect, E_POINTER);
//						//m_pIPEffect->set_MeasureMode(m_MeasureMode);
//						break;
//					}
//					case IDC_RADIO_Measure3Lines:
//					{
//						//GetMeasureModeValue(3);
//						//CheckPointer(m_pIPEffect, E_POINTER);
//						//m_pIPEffect->set_MeasureMode(m_MeasureMode);
//						break;
//					}
//					case IDC_CHECK_MeasureInPixels:
//					{
//						//GetMeasurePixelsValue();
//						//CheckPointer(m_pIPEffect, E_POINTER);
//						//m_pIPEffect->put_MeasurePixels(m_MeasurePixels);
//						break;
//					}
//
//					//Calibrate
//					case IDC_BUTTON_CalibrateStart:
//					{
//						//SetCalibrationButtons(true);
//						//CheckPointer(m_pIPEffect, E_POINTER);
//						//m_pIPEffect->set_CalibrateStart();
//						break;
//					}
//					case IDC_BUTTON_CalibrateSave:
//					{
//						//SetCalibrationButtons(false);
//						//CheckPointer(m_pIPEffect, E_POINTER);
//						//m_pIPEffect->set_CalibrateSave(m_CalibrateDistance);
//						break;
//					}
//					case IDC_BUTTON_CalibrateCancel:
//					{
//						//SetCalibrationButtons(false);
//						//CheckPointer(m_pIPEffect, E_POINTER);
//						//m_pIPEffect->set_CalibrateCancel();
//						break;
//					}
//					case IDC_EDIT_CalibrateDistance:
//					{
//						//GetCalibrateDistanceValueText();
//						break;
//					}
//					case IDC_CHECK_CalibrateFreeze:
//					{
//						//GetCalibrateFreezeValue();
//						//CheckPointer(m_pIPEffect, E_POINTER);
//						//m_pIPEffect->set_CalibrateFreeze(m_CalibrateFreeze);
//						break;
//					}
//
//					//Line Direction
//					case IDC_RADIO_AngleHori:
//					{
//						//GetAngleHoriValue();
//						//CheckPointer(m_pIPEffect, E_POINTER);
//						//m_pIPEffect->set_AngleHori(m_AngleHori);
//						break;
//					}
//					case IDC_RADIO_AngleVerti:
//					{
//						//GetAngleHoriValue();
//						//CheckPointer(m_pIPEffect, E_POINTER);
//						//m_pIPEffect->set_AngleHori(m_AngleHori);
//						break;
//					}
//					case IDC_RADIO_StitchHori:
//					{
//						//GetStitchHoriValue();
//						//CheckPointer(m_pIPEffect, E_POINTER);
//						//m_pIPEffect->set_StitchHori(m_StitchHori);
//						break;
//					}
//					case IDC_RADIO_StitchVerti:
//					{
//						//GetStitchHoriValue();
//						//CheckPointer(m_pIPEffect, E_POINTER);
//						//m_pIPEffect->set_StitchHori(m_StitchHori);
//						break;
//					}
//					case IDC_RADIO_RegisterHori:
//					{
//						//GetRegisterHoriValue();
//						//CheckPointer(m_pIPEffect, E_POINTER);
//						//m_pIPEffect->set_RegisterHori(m_RegisterHori);
//						break;
//					}
//					case IDC_RADIO_RegisterVerti:
//					{
//						//GetRegisterHoriValue();
//						//CheckPointer(m_pIPEffect, E_POINTER);
//						//m_pIPEffect->set_RegisterHori(m_RegisterHori);
//						break;
//					}
//				}
//			}
//			break;
//		}
//		case WM_HSCROLL:
//		{
//			if (m_bIsInitialized)
//			{
//				////Damit der Apply-Button erscheint
//				//m_bDirty = TRUE;
//				//if (m_pPageSite)
//				//{
//				//	m_pPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);
//				//}
//
//				////Binarize Threshold Slider
//				//GetThresholdValueSlide();
//				//CheckPointer(m_pIPEffect, E_POINTER);
//				//m_pIPEffect->put_Threshold(m_Threshold);
//			}
//			break;
//		}
//		case WM_NOTIFY:
//		{
//			switch (LOWORD(wParam))
//			{
//				default:
//				{
//
//				}
//				break;
//			}
//			break;
//		}
//		case (WM_APP + 2019):
//		{
//			//Message from dll
//			if (wParam == 100)
//			{
//				//Set Threshold Value from dll
//				SetThresholdValueInt(LOWORD(lParam));
//			}
//			else if (wParam == 101)
//			{
//				//Display Frame Delay from dll
//				m_FrameTime = *((long*)lParam);
//				SetFrameTime(m_FrameTime);
//			}
//			break;
//		}
//		case WM_CTLCOLORBTN:
//		{
//			HWND hctrl = (HWND)lParam;
//			if (hctrl == GetDlgItem(m_Dlg, IDC_BUTTON_CrossColorDsp))
//			{
//				return (LRESULT)CreateSolidBrush(m_CrossColor);
//			}
//			else if (hctrl == GetDlgItem(m_Dlg, IDC_BUTTON_TextColorDsp))
//			{
//				return (LRESULT)CreateSolidBrush(m_TextColor);
//			}
//			else if (hctrl == GetDlgItem(m_Dlg, IDC_BUTTON_BlobColorDsp))
//			{
//				return (LRESULT)CreateSolidBrush(m_BlobColor);
//			}
//			if (hctrl == GetDlgItem(m_Dlg, IDC_BUTTON_CrossColorBlobDsp))
//			{
//				return (LRESULT)CreateSolidBrush(m_CrossColorBlob);
//			}
//
//		}
//	}
//
//	return CBasePropertyPage::OnReceiveMessage(hwnd, uMsg, wParam, lParam);
//}
//
//// OnConnect, Called when we connect to a transform filter
//HRESULT rx_AlignFilter_Properties::OnConnect(IUnknown *pUnknown)
//{
//	CheckPointer(pUnknown, E_POINTER);
//	ASSERT(m_pIPEffect == NULL);
//
//	HRESULT hr = pUnknown->QueryInterface(IID_rx_AlignFilter_Effect, reinterpret_cast<void **>(&m_pIPEffect));
//	if (FAILED(hr))
//	{
//		return E_NOINTERFACE;
//	}
//
//	//Device Name
//	CheckPointer(m_pIPEffect, E_FAIL);
//	m_pIPEffect->get_DeviceName(&m_DeviceName);
//
//	//Binarize
//
//	//Binarize
//	CheckPointer(m_pIPEffect, E_FAIL);
//	m_pIPEffect->get_Binarize(&m_Binarize);
//	m_OldBinarize = m_Binarize;
//
//	//Automatic
//	CheckPointer(m_pIPEffect, E_FAIL);
//	m_pIPEffect->get_BinarizeAuto(&m_Automatic);
//	m_OldAutomatic = m_Automatic;
//
//	//Threshold
//	CheckPointer(m_pIPEffect, E_FAIL);
//	m_pIPEffect->get_Threshold(&m_Threshold);
//	m_OldThreshold = m_Threshold;
//
//	//ShowHistogram
//	CheckPointer(m_pIPEffect, E_FAIL);
//	m_pIPEffect->get_ShowHistogram(&m_ShowHistogram);
//	m_OldShowHistogram = m_ShowHistogram;
//
//
//	//Dilate/Erode
//
//	//Num Erode-Dilate Iterations
//	CheckPointer(m_pIPEffect, E_FAIL);
//	m_pIPEffect->get_DilateErodes(&m_DilateErodes);
//	m_OldDilateErodes = m_DilateErodes;
//
//	//Num Extra Erodes
//	CheckPointer(m_pIPEffect, E_FAIL);
//	m_pIPEffect->get_ExtraErodes(&m_ExtraErodes);
//	m_OldExtraErodes = m_ExtraErodes;
//
//	//Erode Seed
//	CheckPointer(m_pIPEffect, E_FAIL);
//	m_pIPEffect->get_ErodeSeedX(&m_ErodeSeedX);
//	m_OldErodeSeedX = m_ErodeSeedX;
//	m_pIPEffect->get_ErodeSeedY(&m_ErodeSeedY);
//	m_OldErodeSeedY = m_ErodeSeedY;
//
//
//	//Blob
//
//	//CrossColor
//	CheckPointer(m_pIPEffect, E_FAIL);
//	m_pIPEffect->get_CrossColorBlob(&m_CrossColorBlob);
//	m_OldCrossColorBlob = m_CrossColorBlob;
//
//
//	//Measurement
//
//	//Select Image to display
//	CheckPointer(m_pIPEffect, E_FAIL);
//	m_pIPEffect->get_ShowOriginal(&m_ShowOriginal);
//	m_OldShowOriginal = m_ShowOriginal;
//
//	//Overlay Blob Outlines
//	CheckPointer(m_pIPEffect, E_FAIL);
//	m_pIPEffect->get_OverlayBlobs(&m_OverlayBlobs);
//	m_OldOverlayBlobs = m_OverlayBlobs;
//
//	//Overlay Blob Centers
//	CheckPointer(m_pIPEffect, E_FAIL);
//	m_pIPEffect->get_OverlayCenters(&m_OverlayCenters);
//	m_OldOverlayCenters = m_OverlayCenters;
//
//	//Overlay Blob Values
//	CheckPointer(m_pIPEffect, E_FAIL);
//	m_pIPEffect->get_OverlayValues(&m_OverlayValues);
//	m_OldOverlayValues = m_OverlayValues;
//
//	//Overlay Blob Distances
//	CheckPointer(m_pIPEffect, E_FAIL);
//	m_pIPEffect->get_OverlayDistances(&m_OverlayDistances);
//	m_OldOverlayDistances = m_OverlayDistances;
//
//	//BlobAreaDivisor
//	CheckPointer(m_pIPEffect, E_FAIL);
//	m_pIPEffect->get_BlobAreaDivisor(&m_BlobAreaDivisor);
//	m_OldBlobAreaDivisor = m_BlobAreaDivisor;
//
//	//BlobAspectLimit
//	CheckPointer(m_pIPEffect, E_FAIL);
//	m_pIPEffect->get_BlobAspectLimit(&m_BlobAspectLimit);
//	m_OldBlobAspectLimit = m_BlobAspectLimit;
//
//	//CrossColor
//	CheckPointer(m_pIPEffect, E_FAIL);
//	m_pIPEffect->get_CrossColor(&m_CrossColor);
//	m_OldCrossColor = m_CrossColor;
//
//	//BlobColor
//	CheckPointer(m_pIPEffect, E_FAIL);
//	m_pIPEffect->get_BlobColor(&m_BlobColor);
//	m_OldBlobColor = m_BlobColor;
//
//	//TextColor
//	CheckPointer(m_pIPEffect, E_FAIL);
//	m_pIPEffect->get_TextColor(&m_TextColor);
//	m_OldTextColor = m_TextColor;
//
//	//TextFont
//	CheckPointer(m_pIPEffect, E_FAIL);
//	m_pIPEffect->get_TextFont(&m_TextFont);
//	m_OldTextFont = m_TextFont;
//
//	//MeasureMode
//	CheckPointer(m_pIPEffect, E_FAIL);
//	m_pIPEffect->get_MeasureMode(&m_MeasureMode);
//	m_OldMeasureMode = m_MeasureMode;
//
//	//Measure in Pixels
//	CheckPointer(m_pIPEffect, E_FAIL);
//	m_pIPEffect->get_MeasurePixels(&m_MeasurePixels);
//	m_OldMeasurePixels = m_MeasurePixels;
//
//	//Line Direction
//	CheckPointer(m_pIPEffect, E_FAIL);
//	m_pIPEffect->get_AngleHori(&m_AngleHori);
//	m_OldAngleHori = m_AngleHori;
//
//	CheckPointer(m_pIPEffect, E_FAIL);
//	m_pIPEffect->get_StitchHori(&m_StitchHori);
//	m_OldStitchHori = m_StitchHori;
//
//	CheckPointer(m_pIPEffect, E_FAIL);
//	m_pIPEffect->get_RegisterHori(&m_RegisterHori);
//	m_OldRegisterHori = m_RegisterHori;
//
//
//	//Calibrate
//	CheckPointer(m_pIPEffect, E_FAIL);
//	m_Calibrate = false;
//
//
//	return NOERROR;
//}
//
//// OnDisconnect, Likewise called when we disconnect from a filter
//HRESULT rx_AlignFilter_Properties::OnDisconnect()
//{
//	//remove HWND from calling Process
//	m_PropHwnd = NULL;
//	CheckPointer(m_pIPEffect, E_FAIL);
//	m_pIPEffect->SendHandle(m_PropHwnd);
//
//	//Binarize
//
//	//Binarize
//	m_Binarize = m_OldBinarize;
//	CheckPointer(m_pIPEffect, E_FAIL);
//	m_pIPEffect->put_Binarize(m_Binarize);
//
//	//Automatic
//	m_Automatic = m_OldAutomatic;
//	CheckPointer(m_pIPEffect, E_FAIL);
//	m_pIPEffect->get_BinarizeAuto(&m_Automatic);
//
//	//Threshold
//	m_Threshold = m_OldThreshold;
//	CheckPointer(m_pIPEffect, E_FAIL);
//	m_pIPEffect->get_Threshold(&m_Threshold);
//
//	//ShowHistogram
//	m_ShowHistogram = m_OldShowHistogram;
//	CheckPointer(m_pIPEffect, E_FAIL);
//	m_pIPEffect->put_ShowHistogram(m_ShowHistogram);
//
//
//	//Dilate/Erode
//
//	//Num Erode-Dilate Iterations
//	m_DilateErodes = m_OldDilateErodes;
//	CheckPointer(m_pIPEffect, E_FAIL);
//	m_pIPEffect->set_DilateErodes(m_DilateErodes);
//
//	//Num Extra Erodes
//	m_ExtraErodes = m_OldExtraErodes;
//	CheckPointer(m_pIPEffect, E_FAIL);
//	m_pIPEffect->set_ExtraErodes(m_ExtraErodes);
//
//	//Erode Seed
//	m_ErodeSeedX = m_OldErodeSeedX;
//	CheckPointer(m_pIPEffect, E_FAIL);
//	m_pIPEffect->set_ErodeSeedX(m_ErodeSeedX);
//	m_ErodeSeedY = m_OldErodeSeedY;
//	CheckPointer(m_pIPEffect, E_FAIL);
//	m_pIPEffect->set_ErodeSeedY(m_ErodeSeedY);
//
//
//	//Blob
//
//	//CrossColor
//	m_CrossColorBlob = m_OldCrossColorBlob;
//	CheckPointer(m_pIPEffect, E_FAIL);
//	m_pIPEffect->set_CrossColorBlob(m_CrossColorBlob);
//
//
//	//Measurement
//
//	//Select Image to display
//	m_ShowOriginal = m_OldShowOriginal;
//	CheckPointer(m_pIPEffect, E_FAIL);
//	m_pIPEffect->put_ShowOriginal(m_ShowOriginal);
//
//	//Overlay Blob Outlines
//	m_OverlayBlobs = m_OldOverlayBlobs;
//	CheckPointer(m_pIPEffect, E_FAIL);
//	m_pIPEffect->put_OverlayBlobs(m_OverlayBlobs);
//
//	//Overlay Blob Centers
//	m_OverlayCenters = m_OldOverlayCenters;
//	CheckPointer(m_pIPEffect, E_FAIL);
//	m_pIPEffect->put_OverlayCenters(m_OverlayCenters);
//
//	//Overlay Blob Values
//	m_OverlayValues = m_OldOverlayValues;
//	CheckPointer(m_pIPEffect, E_FAIL);
//	m_pIPEffect->put_OverlayValues(m_OverlayValues);
//
//	//Overlay Blob Distances
//	m_OverlayDistances = m_OldOverlayDistances;
//	CheckPointer(m_pIPEffect, E_FAIL);
//	m_pIPEffect->put_OverlayDistances(m_OverlayDistances);
//
//	//BlobAreaDivisor
//	m_BlobAreaDivisor = m_OldBlobAreaDivisor;
//	CheckPointer(m_pIPEffect, E_FAIL);
//	m_pIPEffect->set_BlobAreaDivisor(m_BlobAreaDivisor);
//
//	//BlobAspectLimit
//	m_BlobAspectLimit = m_OldBlobAspectLimit;
//	CheckPointer(m_pIPEffect, E_FAIL);
//	m_pIPEffect->set_BlobAspectLimit(m_BlobAspectLimit);
//
//	//CrossColor
//	m_CrossColor = m_OldCrossColor;
//	CheckPointer(m_pIPEffect, E_FAIL);
//	m_pIPEffect->set_CrossColor(m_CrossColor);
//
//	//BlobColor
//	m_BlobColor = m_OldBlobColor;
//	CheckPointer(m_pIPEffect, E_FAIL);
//	m_pIPEffect->set_BlobColor(m_BlobColor);
//
//	//TextColor
//	m_TextColor = m_OldTextColor;
//	CheckPointer(m_pIPEffect, E_FAIL);
//	m_pIPEffect->set_TextColor(m_TextColor);
//
//	//TextFont
//	m_TextFont = m_OldTextFont;
//	CheckPointer(m_pIPEffect, E_FAIL);
//	m_pIPEffect->set_TextFont(m_TextFont);
//
//	//MeasureMode
//	m_MeasureMode = m_OldMeasureMode;
//	CheckPointer(m_pIPEffect, E_FAIL);
//	m_pIPEffect->set_MeasureMode(m_MeasureMode);
//
//	//Measure in Pixels
//	m_MeasurePixels = m_OldMeasurePixels;
//	CheckPointer(m_pIPEffect, E_FAIL);
//	m_pIPEffect->put_MeasurePixels(m_MeasurePixels);
//
//	//Line Direction
//	m_AngleHori = m_OldAngleHori;
//	CheckPointer(m_pIPEffect, E_FAIL);
//	m_pIPEffect->set_AngleHori(m_AngleHori);
//
//	m_StitchHori = m_OldStitchHori;
//	CheckPointer(m_pIPEffect, E_FAIL);
//	m_pIPEffect->set_StitchHori(m_StitchHori);
//
//	m_RegisterHori = m_OldRegisterHori;
//	CheckPointer(m_pIPEffect, E_FAIL);
//	m_pIPEffect->set_RegisterHori(m_RegisterHori);
//
//
//	// Release of Interface after setting the appropriate old effect value
//	if (m_pIPEffect)
//	{
//		m_pIPEffect->Release();
//		m_pIPEffect = NULL;
//	}
//	return NOERROR;
//}
//
//// OnActivate, We are being activated
//HRESULT rx_AlignFilter_Properties::OnActivate(void)
//{
//	wchar_t EffVal[STR_MAX_LENGTH];
//
//	//Device Name
//	SetDlgItemText(m_Dlg, IDC_STATIC_Device, m_DeviceName);
//
//	//Binarize
//
//	//Binarize
//	CheckDlgButton(m_Dlg, IDC_CHECK_Binarize, m_Binarize);
//
//	//BinarizeAutomatic
//	if (m_Binarize)
//	{
//		Button_Enable(GetDlgItem(m_Dlg, IDC_CHECK_BinarizeAuto), 1);
//	}
//	CheckDlgButton(m_Dlg, IDC_CHECK_BinarizeAuto, m_Automatic);
//
//	//Threshold
//	if (m_Binarize && !m_Automatic)
//	{
//		Button_Enable(GetDlgItem(m_Dlg, IDC_SPIN_Threshold), 1);
//		Button_Enable(GetDlgItem(m_Dlg, IDC_EDIT_Threshold), 1);
//		Button_Enable(GetDlgItem(m_Dlg, IDC_SLIDER_Threshold), 1);
//	}
//	_itow_s(m_Threshold, EffVal, 10);
//	SetDlgItemText(m_Dlg, IDC_EDIT_Threshold, (LPCWSTR)EffVal);
//	PostMessage(GetDlgItem(m_Dlg, IDC_SPIN_Threshold), UDM_SETRANGE, TRUE, MAKELPARAM(255, 0));
//	PostMessage(GetDlgItem(m_Dlg, IDC_SPIN_Threshold), UDM_SETBUDDY, (WPARAM)GetDlgItem(m_Dlg, IDC_EDIT_Threshold), 0);
//	//Slider
//	PostMessage(GetDlgItem(m_Dlg, IDC_SLIDER_Threshold), TBM_SETRANGEMIN, TRUE, 1);
//	PostMessage(GetDlgItem(m_Dlg, IDC_SLIDER_Threshold), TBM_SETRANGEMAX, TRUE, 255);
//	PostMessage(GetDlgItem(m_Dlg, IDC_SLIDER_Threshold), TBM_SETPOS, TRUE, m_Threshold);
//	PostMessage(GetDlgItem(m_Dlg, IDC_SLIDER_Threshold), TBM_SETPAGESIZE, TRUE, 10);
//	//Text
//	if (m_Threshold == 0)m_Threshold = 1;
//	_itow_s(m_Threshold, EffVal, 10);
//	SetDlgItemText(m_Dlg, IDC_EDIT_Threshold, (LPCWSTR)EffVal);
//
//	//ShowHistogram
//	CheckDlgButton(m_Dlg, IDC_CHECK_ShowHistogram, m_ShowHistogram);
//
//	//Dilate/Erode
//
//	//Num Erode-DilateIterations
//	_itow_s(m_DilateErodes, EffVal, 10);
//	SetDlgItemText(m_Dlg, IDC_EDIT_DilateErodes, (LPCWSTR)EffVal);
//	PostMessage(GetDlgItem(m_Dlg, IDC_SPIN_DilateErodes), UDM_SETRANGE, TRUE, MAKELPARAM(255, 0));
//	PostMessage(GetDlgItem(m_Dlg, IDC_SPIN_DilateErodes), UDM_SETBUDDY, (WPARAM)GetDlgItem(m_Dlg, IDC_EDIT_DilateErodes), 0);
//
//	//Num Extra Erodes
//	_itow_s(m_ExtraErodes, EffVal, 10);
//	SetDlgItemText(m_Dlg, IDC_EDIT_ExtraErodes, (LPCWSTR)EffVal);
//	PostMessage(GetDlgItem(m_Dlg, IDC_SPIN_ExtraErodes), UDM_SETRANGE, TRUE, MAKELPARAM(255, 0));
//	PostMessage(GetDlgItem(m_Dlg, IDC_SPIN_ExtraErodes), UDM_SETBUDDY, (WPARAM)GetDlgItem(m_Dlg, IDC_EDIT_ExtraErodes), 0);
//
//	//Erode Seed X
//	_itow_s(m_ErodeSeedX, EffVal, 10);
//	SetDlgItemText(m_Dlg, IDC_EDIT_ErodeSeedX, (LPCWSTR)EffVal);
//	PostMessage(GetDlgItem(m_Dlg, IDC_SPIN_ErodeSeedX), UDM_SETRANGE, TRUE, MAKELPARAM(255, 0));
//	PostMessage(GetDlgItem(m_Dlg, IDC_SPIN_ErodeSeedX), UDM_SETBUDDY, (WPARAM)GetDlgItem(m_Dlg, IDC_EDIT_ErodeSeedX), 0);
//
//	//Erode Seed Y
//	_itow_s(m_ErodeSeedY, EffVal, 10);
//	SetDlgItemText(m_Dlg, IDC_EDIT_ErodeSeedY, (LPCWSTR)EffVal);
//	PostMessage(GetDlgItem(m_Dlg, IDC_SPIN_ErodeSeedY), UDM_SETRANGE, TRUE, MAKELPARAM(255, 0));
//	PostMessage(GetDlgItem(m_Dlg, IDC_SPIN_ErodeSeedY), UDM_SETBUDDY, (WPARAM)GetDlgItem(m_Dlg, IDC_EDIT_ErodeSeedY), 0);
//
//
//	//Blob
//
//	//Measurement
//
//	//Select Image to display
//	CheckDlgButton(m_Dlg, IDC_RADIO_ShowOriginal, m_ShowOriginal);
//	CheckDlgButton(m_Dlg, IDC_RADIO_ShowProcess, !m_ShowOriginal);
//
//	//m_ShowOriginal
//	if (m_ShowOriginal)
//	{
//		Button_Enable(GetDlgItem(m_Dlg, IDC_CHECK_OvBlobs), 1);
//		Button_Enable(GetDlgItem(m_Dlg, IDC_CHECK_OvCenters), 1);
//		Button_Enable(GetDlgItem(m_Dlg, IDC_CHECK_OvValues), 1);
//		Button_Enable(GetDlgItem(m_Dlg, IDC_CHECK_OvDistances), 1);
//		Button_Enable(GetDlgItem(m_Dlg, IDC_BUTTON_BlobColor), 1);
//		Button_Enable(GetDlgItem(m_Dlg, IDC_BUTTON_CrossColor), 1);
//		Button_Enable(GetDlgItem(m_Dlg, IDC_BUTTON_TextColor), 1);
//		Button_Enable(GetDlgItem(m_Dlg, IDC_BUTTON_Font), 1);
//		Button_Enable(GetDlgItem(m_Dlg, IDC_EDIT_BlobAreaDivisor), 1);
//		Button_Enable(GetDlgItem(m_Dlg, IDC_EDIT_BlobAspectLimit), 1);
//		Button_Enable(GetDlgItem(m_Dlg, IDC_SPIN_BlobAreaDivisor), 1);
//		Button_Enable(GetDlgItem(m_Dlg, IDC_SPIN_BlobAspectLimit), 1);
//	}
//	else
//	{
//		Button_Enable(GetDlgItem(m_Dlg, IDC_CHECK_OvBlobs), 0);
//		Button_Enable(GetDlgItem(m_Dlg, IDC_CHECK_OvCenters), 0);
//		Button_Enable(GetDlgItem(m_Dlg, IDC_CHECK_OvValues), 0);
//		Button_Enable(GetDlgItem(m_Dlg, IDC_CHECK_OvDistances), 0);
//		Button_Enable(GetDlgItem(m_Dlg, IDC_BUTTON_BlobColor), 0);
//		Button_Enable(GetDlgItem(m_Dlg, IDC_BUTTON_CrossColor), 0);
//		Button_Enable(GetDlgItem(m_Dlg, IDC_BUTTON_TextColor), 0);
//		Button_Enable(GetDlgItem(m_Dlg, IDC_BUTTON_Font), 0);
//		Button_Enable(GetDlgItem(m_Dlg, IDC_EDIT_BlobAreaDivisor), 0);
//		Button_Enable(GetDlgItem(m_Dlg, IDC_EDIT_BlobAspectLimit), 0);
//		Button_Enable(GetDlgItem(m_Dlg, IDC_SPIN_BlobAreaDivisor), 0);
//		Button_Enable(GetDlgItem(m_Dlg, IDC_SPIN_BlobAspectLimit), 0);
//	}
//
//	//Overlay Blob Outlines
//	CheckDlgButton(m_Dlg, IDC_CHECK_OvBlobs, m_OverlayBlobs);
//
//	//Overlay Blob Centers
//	CheckDlgButton(m_Dlg, IDC_CHECK_OvCenters, m_OverlayCenters);
//
//	//Overlay Blob Values
//	CheckDlgButton(m_Dlg, IDC_CHECK_OvValues, m_OverlayValues);
//
//	//Overlay Blob Distances
//	CheckDlgButton(m_Dlg, IDC_CHECK_OvDistances, m_OverlayDistances);
//
//	//BlobAreaDivisor
//	_itow_s(m_BlobAreaDivisor, EffVal, 10);
//	SetDlgItemText(m_Dlg, IDC_EDIT_BlobAreaDivisor, (LPCWSTR)EffVal);
//	PostMessage(GetDlgItem(m_Dlg, IDC_SPIN_BlobAreaDivisor), UDM_SETRANGE, TRUE, MAKELPARAM(255, 0));
//	PostMessage(GetDlgItem(m_Dlg, IDC_SPIN_BlobAreaDivisor), UDM_SETBUDDY, (WPARAM)GetDlgItem(m_Dlg, IDC_EDIT_BlobAreaDivisor), 0);
//
//	//BlobAspectLimit
//	_itow_s(m_BlobAspectLimit, EffVal, 10);
//	SetDlgItemText(m_Dlg, IDC_EDIT_BlobAspectLimit, (LPCWSTR)EffVal);
//	PostMessage(GetDlgItem(m_Dlg, IDC_SPIN_BlobAspectLimit), UDM_SETRANGE, TRUE, MAKELPARAM(255, 0));
//	PostMessage(GetDlgItem(m_Dlg, IDC_SPIN_BlobAspectLimit), UDM_SETBUDDY, (WPARAM)GetDlgItem(m_Dlg, IDC_EDIT_BlobAspectLimit), 0);
//
//	//MeasureMode
//
//	CheckDlgButton(m_Dlg, IDC_RADIO_MeasureAllLines, FALSE);
//	CheckDlgButton(m_Dlg, IDC_RADIO_Measure2x4Lines, FALSE);
//	CheckDlgButton(m_Dlg, IDC_RADIO_Measure2Lines, FALSE);
//	CheckDlgButton(m_Dlg, IDC_RADIO_MeasureX, FALSE);
//	switch (m_MeasureMode)
//	{
//	case 0:
//		CheckDlgButton(m_Dlg, IDC_RADIO_MeasureAllLines, TRUE);
//		break;
//	case 1:
//		CheckDlgButton(m_Dlg, IDC_RADIO_Measure2x4Lines, TRUE);
//		break;
//	case 2:
//		CheckDlgButton(m_Dlg, IDC_RADIO_Measure2Lines, TRUE);
//		break;
//	case 3:
//		CheckDlgButton(m_Dlg, IDC_RADIO_MeasureX, TRUE);
//		break;
//	}
//
//	//Line Direction
//	CheckDlgButton(m_Dlg, IDC_RADIO_AngleHori, m_AngleHori);
//	CheckDlgButton(m_Dlg, IDC_RADIO_AngleVerti, !m_AngleHori);
//	CheckDlgButton(m_Dlg, IDC_RADIO_StitchHori, m_StitchHori);
//	CheckDlgButton(m_Dlg, IDC_RADIO_StitchVerti, !m_StitchHori);
//	CheckDlgButton(m_Dlg, IDC_RADIO_RegisterHori, m_RegisterHori);
//	CheckDlgButton(m_Dlg, IDC_RADIO_RegisterVerti, !m_RegisterHori);
//
//	//Calibrate
//	_itow_s(0, EffVal, 10);
//	SetDlgItemText(m_Dlg, IDC_EDIT_CalibrateDistance, (LPCWSTR)EffVal);
//	Button_Enable(GetDlgItem(m_Dlg, IDC_EDIT_CalibrateDistance), 0);
//	Button_Enable(GetDlgItem(m_Dlg, IDC_BUTTON_CalibrateCancel), 0);
//	Button_Enable(GetDlgItem(m_Dlg, IDC_BUTTON_CalibrateSave), 0);
//
//
//	//Send HWND to calling Process
//	m_PropHwnd = m_Dlg;
//	m_pIPEffect->SendHandle(m_PropHwnd);
//
//	m_bIsInitialized = TRUE;
//
//	return NOERROR;
//}
//
//// OnDeactivate, We are being deactivated
//HRESULT rx_AlignFilter_Properties::OnDeactivate(void)
//{
//	ASSERT(m_pIPEffect);
//
//	m_bIsInitialized = FALSE;
//
//	return NOERROR;
//}
//
//// OnApplyChanges, Apply any changes so far made
//HRESULT rx_AlignFilter_Properties::OnApplyChanges()
//{
//	GetControlValues();
//
//	//Binarize
//
//	//Binarize
//	CheckPointer(m_pIPEffect, E_POINTER);
//	m_pIPEffect->put_Binarize(m_Binarize);
//	m_OldBinarize = m_Binarize;
//
//	//BinarizeAutomatic
//	CheckPointer(m_pIPEffect, E_POINTER);
//	m_pIPEffect->put_BinarizeAuto(m_Automatic);
//	m_OldAutomatic = m_Automatic;
//
//	//ShowHistogram
//	CheckPointer(m_pIPEffect, E_POINTER);
//	m_pIPEffect->put_ShowHistogram(m_ShowHistogram);
//	m_ShowHistogram = m_ShowHistogram;
//
//
//	//Dilate/Erode
//
//	//Num Erode-Dilate Iterations
//	CheckPointer(m_pIPEffect, E_FAIL);
//	m_pIPEffect->set_DilateErodes(m_DilateErodes);
//	m_OldDilateErodes = m_DilateErodes;
//
//	//Num Extra Erodes
//	CheckPointer(m_pIPEffect, E_FAIL);
//	m_pIPEffect->set_ExtraErodes(m_ExtraErodes);
//	m_OldExtraErodes = m_ExtraErodes;
//
//	//Erode Seed
//	CheckPointer(m_pIPEffect, E_FAIL);
//	m_pIPEffect->set_ErodeSeedX(m_ErodeSeedX);
//	m_OldErodeSeedX = m_ErodeSeedX;
//	m_pIPEffect->set_ErodeSeedY(m_ErodeSeedY);
//	m_OldErodeSeedY = m_ErodeSeedY;
//
//	
//	//Blob
//
//	//CrossColor
//	CheckPointer(m_pIPEffect, E_FAIL);
//	m_pIPEffect->get_CrossColorBlob(&m_CrossColorBlob);
//	m_OldCrossColorBlob = m_CrossColorBlob;
//
//
//	//Measurement
//
//	//Select Image to display
//	CheckPointer(m_pIPEffect, E_FAIL);
//	m_pIPEffect->put_ShowOriginal(m_ShowOriginal);
//	m_OldShowOriginal = m_ShowOriginal;
//
//	//Overlay Blob Outlines
//	CheckPointer(m_pIPEffect, E_FAIL);
//	m_pIPEffect->put_OverlayBlobs(m_OverlayBlobs);
//	m_OldOverlayBlobs = m_OverlayBlobs;
//
//	//Overlay Blob Centers
//	CheckPointer(m_pIPEffect, E_FAIL);
//	m_pIPEffect->put_OverlayCenters(m_OverlayCenters);
//	m_OldOverlayCenters = m_OverlayCenters;
//
//	//Overlay Blob Values
//	CheckPointer(m_pIPEffect, E_FAIL);
//	m_pIPEffect->put_OverlayValues(m_OverlayValues);
//	m_OldOverlayValues = m_OverlayValues;
//
//	//Overlay Blob Distances
//	CheckPointer(m_pIPEffect, E_FAIL);
//	m_pIPEffect->put_OverlayDistances(m_OverlayDistances);
//	m_OldOverlayDistances = m_OverlayDistances;
//
//	//BlobAreaDivisor
//	CheckPointer(m_pIPEffect, E_FAIL);
//	m_pIPEffect->set_BlobAreaDivisor(m_BlobAreaDivisor);
//	m_OldBlobAreaDivisor = m_BlobAreaDivisor;
//
//	//BlobAspectLimit
//	CheckPointer(m_pIPEffect, E_FAIL);
//	m_pIPEffect->set_BlobAspectLimit(m_BlobAspectLimit);
//	m_OldBlobAspectLimit = m_BlobAspectLimit;
//
//	//CrossColor
//	CheckPointer(m_pIPEffect, E_FAIL);
//	m_pIPEffect->set_CrossColor(m_CrossColor);
//	m_OldCrossColor = m_CrossColor;
//
//	//BlobColor
//	CheckPointer(m_pIPEffect, E_FAIL);
//	m_pIPEffect->set_BlobColor(m_BlobColor);
//	m_OldBlobColor = m_BlobColor;
//
//	//TextColor
//	CheckPointer(m_pIPEffect, E_FAIL);
//	m_pIPEffect->set_TextColor(m_TextColor);
//	m_OldTextColor = m_TextColor;
//
//	//TextFont
//	CheckPointer(m_pIPEffect, E_FAIL);
//	m_pIPEffect->set_TextFont(m_TextFont);
//	m_OldTextFont = m_TextFont;
//
//	//MeasureMode
//	CheckPointer(m_pIPEffect, E_FAIL);
//	m_pIPEffect->set_MeasureMode(m_MeasureMode);
//	m_OldMeasureMode = m_MeasureMode;
//
//	//Measure in Pixels
//	CheckPointer(m_pIPEffect, E_FAIL);
//	m_pIPEffect->put_MeasurePixels(m_MeasurePixels);
//	m_OldMeasurePixels = m_MeasurePixels;
//
//
//	//Line Direction
//	CheckPointer(m_pIPEffect, E_FAIL);
//	m_pIPEffect->set_AngleHori(m_AngleHori);
//	m_OldAngleHori = m_AngleHori;
//
//	CheckPointer(m_pIPEffect, E_FAIL);
//	m_pIPEffect->set_StitchHori(m_StitchHori);
//	m_OldStitchHori = m_StitchHori;
//
//	CheckPointer(m_pIPEffect, E_FAIL);
//	m_pIPEffect->set_RegisterHori(m_RegisterHori);
//	m_OldRegisterHori = m_RegisterHori;
//
//	return NOERROR;
//}
//
//
//
////Dialog Values
//void rx_AlignFilter_Properties::GetControlValues()
//{
//	GetThresholdValueText();
//
//	GetDilateErodesValueText();
//	GetExtraErodesValueText();
//	GetErodeSeedXValueText();
//	GetErodeSeedYValueText();
//	
//	GetBlobAreaDivisorValueText();
//	GetBlobAspectLimitValueText();
//}
//
//void rx_AlignFilter_Properties::GetDspTimingValue()
//{
//	m_DspTiming = IsDlgButtonChecked(m_Dlg, IDC_CHECK_DisplayTiming);
//}
//
//void rx_AlignFilter_Properties::SetFrameTime(long FrameTime)
//{
//	//Set Text for Frame Time
//	wchar_t EffVal[STR_MAX_LENGTH];
//	swprintf_s(EffVal, L"%1.3f ms", (float)((float)FrameTime / 1000.0f));
//	Edit_SetText(GetDlgItem(m_Dlg, IDC_EDIT_FrameTime), EffVal);
//}
//
//
////Binarize
//void rx_AlignFilter_Properties::GetBinarizeValue()
//{
//	m_Binarize = IsDlgButtonChecked(m_Dlg, IDC_CHECK_Binarize);
//	if (m_Binarize)
//	{
//		Button_Enable(GetDlgItem(m_Dlg, IDC_CHECK_BinarizeAuto), 1);
//		if (!m_Automatic)
//		{
//			Button_Enable(GetDlgItem(m_Dlg, IDC_SPIN_Threshold), 1);
//			Button_Enable(GetDlgItem(m_Dlg, IDC_EDIT_Threshold), 1);
//			Button_Enable(GetDlgItem(m_Dlg, IDC_SLIDER_Threshold), 1);
//		}
//		else
//		{
//			Button_Enable(GetDlgItem(m_Dlg, IDC_SPIN_Threshold), 0);
//			Button_Enable(GetDlgItem(m_Dlg, IDC_EDIT_Threshold), 0);
//			Button_Enable(GetDlgItem(m_Dlg, IDC_SLIDER_Threshold), 0);
//		}
//	}
//	else
//	{
//		Button_Enable(GetDlgItem(m_Dlg, IDC_CHECK_BinarizeAuto), 0);
//		Button_Enable(GetDlgItem(m_Dlg, IDC_SPIN_Threshold), 0);
//		Button_Enable(GetDlgItem(m_Dlg, IDC_EDIT_Threshold), 0);
//		Button_Enable(GetDlgItem(m_Dlg, IDC_SLIDER_Threshold), 0);
//	}
//}
//
//void rx_AlignFilter_Properties::GetAutomaticValue()
//{
//	m_Automatic = IsDlgButtonChecked(m_Dlg, IDC_CHECK_BinarizeAuto);
//	if (m_Binarize && !m_Automatic)
//	{
//		Button_Enable(GetDlgItem(m_Dlg, IDC_SPIN_Threshold), 1);
//		Button_Enable(GetDlgItem(m_Dlg, IDC_EDIT_Threshold), 1);
//		Button_Enable(GetDlgItem(m_Dlg, IDC_SLIDER_Threshold), 1);
//	}
//	else
//	{
//		Button_Enable(GetDlgItem(m_Dlg, IDC_SPIN_Threshold), 0);
//		Button_Enable(GetDlgItem(m_Dlg, IDC_EDIT_Threshold), 0);
//		Button_Enable(GetDlgItem(m_Dlg, IDC_SLIDER_Threshold), 0);
//	}
//}
//
//void rx_AlignFilter_Properties::GetThresholdValueSlide()
//{
//	//Get Value from Slider
//	m_Threshold = (UINT)SendMessage(GetDlgItem(m_Dlg, IDC_SLIDER_Threshold), TBM_GETPOS, 0, 0);
//	if (m_Threshold == 0)m_Threshold = 1;
//	if (m_Threshold > 255)m_Threshold = 255;
//
//	//Set Text according Slider
//	wchar_t EffVal[STR_MAX_LENGTH];
//	_itow_s(m_Threshold, EffVal, 10);
//	Edit_SetText(GetDlgItem(m_Dlg, IDC_EDIT_Threshold), EffVal);
//}
//
//void rx_AlignFilter_Properties::GetThresholdValueText()
//{
//	//Get Value from Text
//	TCHAR szThreshold[STR_MAX_LENGTH];
//	Edit_GetText(GetDlgItem(m_Dlg, IDC_EDIT_Threshold), szThreshold, STR_MAX_LENGTH);
//	// Convert Multibyte string to ANSI
//	char szANSI[STR_MAX_LENGTH];
//	int rc = WideCharToMultiByte(CP_ACP, 0, szThreshold, -1, szANSI, STR_MAX_LENGTH, NULL, NULL);
//	m_Threshold = (UINT)atof(szANSI);
//	if (m_Threshold == 0)m_Threshold = 1;
//	if (m_Threshold > 255)m_Threshold = 255;
//
//	//Set Slider according Text
//	PostMessage(GetDlgItem(m_Dlg, IDC_SLIDER_Threshold), TBM_SETPOS, TRUE, m_Threshold);
//}
//
//void rx_AlignFilter_Properties::SetThresholdValueInt(UINT Threshold)
//{
//	//Set Threshold from dll
//	if (m_Binarize && m_Automatic)
//	{
//		m_Threshold = Threshold;
//
//		//Text
//		wchar_t EffVal[STR_MAX_LENGTH];
//		_itow_s(m_Threshold, EffVal, 10);
//		SetDlgItemText(m_Dlg, IDC_EDIT_Threshold, (LPCWSTR)EffVal);
//		//Slider
//		PostMessage(GetDlgItem(m_Dlg, IDC_SLIDER_Threshold), TBM_SETPOS, TRUE, m_Threshold);
//	}
//
//}
//
//void rx_AlignFilter_Properties::GetShowHistogramValue()
//{
//	m_ShowHistogram = IsDlgButtonChecked(m_Dlg, IDC_CHECK_ShowHistogram);
//}
//
//
////Dilate/Erode
//void rx_AlignFilter_Properties::GetDilateErodesValueText()
//{
//	//Get Value from Text
//	TCHAR szDilateErodes[STR_MAX_LENGTH];
//	Edit_GetText(GetDlgItem(m_Dlg, IDC_EDIT_DilateErodes), szDilateErodes, STR_MAX_LENGTH);
//	// Convert Multibyte string to ANSI
//	char szANSI[STR_MAX_LENGTH];
//	int rc = WideCharToMultiByte(CP_ACP, 0, szDilateErodes, -1, szANSI, STR_MAX_LENGTH, NULL, NULL);
//	m_DilateErodes = (UINT)atof(szANSI);
//}
//
//void rx_AlignFilter_Properties::GetExtraErodesValueText()
//{
//	//Get Value from Text
//	TCHAR szExtraErodes[STR_MAX_LENGTH];
//	Edit_GetText(GetDlgItem(m_Dlg, IDC_EDIT_ExtraErodes), szExtraErodes, STR_MAX_LENGTH);
//	// Convert Multibyte string to ANSI
//	char szANSI[STR_MAX_LENGTH];
//	int rc = WideCharToMultiByte(CP_ACP, 0, szExtraErodes, -1, szANSI, STR_MAX_LENGTH, NULL, NULL);
//	m_ExtraErodes = (UINT)atof(szANSI);
//}
//
//void rx_AlignFilter_Properties::GetErodeSeedXValueText()
//{
//	//Get Value from Text
//	TCHAR szErodeSeedX[STR_MAX_LENGTH];
//	Edit_GetText(GetDlgItem(m_Dlg, IDC_EDIT_ErodeSeedX), szErodeSeedX, STR_MAX_LENGTH);
//	// Convert Multibyte string to ANSI
//	char szANSI[STR_MAX_LENGTH];
//	int rc = WideCharToMultiByte(CP_ACP, 0, szErodeSeedX, -1, szANSI, STR_MAX_LENGTH, NULL, NULL);
//	m_ErodeSeedX = (UINT)atof(szANSI);
//}
//
//void rx_AlignFilter_Properties::GetErodeSeedYValueText()
//{
//	//Get Value from Text
//	TCHAR szErodeSeedY[STR_MAX_LENGTH];
//	Edit_GetText(GetDlgItem(m_Dlg, IDC_EDIT_ErodeSeedY), szErodeSeedY, STR_MAX_LENGTH);
//	// Convert Multibyte string to ANSI
//	char szANSI[STR_MAX_LENGTH];
//	int rc = WideCharToMultiByte(CP_ACP, 0, szErodeSeedY, -1, szANSI, STR_MAX_LENGTH, NULL, NULL);
//	m_ErodeSeedY = (UINT)atof(szANSI);
//}
//
////Blob
//void rx_AlignFilter_Properties::GetCrossColorBlobDialog(HWND hwnd)
//{
//	CHOOSECOLOR cc;                 // common dialog box structure 
//	static COLORREF acrCustClr[16]; // array of custom colors 
//	HBRUSH hbrush;                  // brush handle
//
//	// Initialize CHOOSECOLOR 
//	ZeroMemory(&cc, sizeof(cc));
//	cc.lStructSize = sizeof(cc);
//	cc.hwndOwner = hwnd;
//	cc.lpCustColors = (LPDWORD)acrCustClr;
//	cc.rgbResult = m_CrossColorBlob;
//	cc.Flags = CC_FULLOPEN | CC_RGBINIT;
//
//	if (ChooseColor(&cc) == TRUE)
//	{
//		hbrush = CreateSolidBrush(cc.rgbResult);
//		m_CrossColorBlob = cc.rgbResult;
//	}
//}
//
////Measurement
//void rx_AlignFilter_Properties::GetShowOriginalValue()
//{
//	m_ShowOriginal = IsDlgButtonChecked(m_Dlg, IDC_RADIO_ShowOriginal);
//	if (m_ShowOriginal)
//	{
//		Button_Enable(GetDlgItem(m_Dlg, IDC_CHECK_OvBlobs), 1);
//		Button_Enable(GetDlgItem(m_Dlg, IDC_CHECK_OvCenters), 1);
//		Button_Enable(GetDlgItem(m_Dlg, IDC_CHECK_OvValues), 1);
//		Button_Enable(GetDlgItem(m_Dlg, IDC_BUTTON_BlobColor), 1);
//		Button_Enable(GetDlgItem(m_Dlg, IDC_BUTTON_CrossColor), 1);
//		Button_Enable(GetDlgItem(m_Dlg, IDC_BUTTON_TextColor), 1);
//		Button_Enable(GetDlgItem(m_Dlg, IDC_BUTTON_Font), 1);
//		Button_Enable(GetDlgItem(m_Dlg, IDC_EDIT_BlobAreaDivisor), 1);
//		Button_Enable(GetDlgItem(m_Dlg, IDC_EDIT_BlobAspectLimit), 1);
//		Button_Enable(GetDlgItem(m_Dlg, IDC_SPIN_BlobAreaDivisor), 1);
//		Button_Enable(GetDlgItem(m_Dlg, IDC_SPIN_BlobAspectLimit), 1);
//	}
//	else
//	{
//		Button_Enable(GetDlgItem(m_Dlg, IDC_CHECK_OvBlobs), 0);
//		Button_Enable(GetDlgItem(m_Dlg, IDC_CHECK_OvCenters), 0);
//		Button_Enable(GetDlgItem(m_Dlg, IDC_CHECK_OvValues), 0);
//		Button_Enable(GetDlgItem(m_Dlg, IDC_BUTTON_BlobColor), 0);
//		Button_Enable(GetDlgItem(m_Dlg, IDC_BUTTON_CrossColor), 0);
//		Button_Enable(GetDlgItem(m_Dlg, IDC_BUTTON_TextColor), 0);
//		Button_Enable(GetDlgItem(m_Dlg, IDC_BUTTON_Font), 0);
//		Button_Enable(GetDlgItem(m_Dlg, IDC_EDIT_BlobAreaDivisor), 0);
//		Button_Enable(GetDlgItem(m_Dlg, IDC_EDIT_BlobAspectLimit), 0);
//		Button_Enable(GetDlgItem(m_Dlg, IDC_SPIN_BlobAreaDivisor), 0);
//		Button_Enable(GetDlgItem(m_Dlg, IDC_SPIN_BlobAspectLimit), 0);
//	}
//}
//
//void rx_AlignFilter_Properties::GetOverlayBlobsValue()
//{
//	m_OverlayBlobs = IsDlgButtonChecked(m_Dlg, IDC_CHECK_OvBlobs);
//}
//
//void rx_AlignFilter_Properties::GetOverlayCentersValue()
//{
//	m_OverlayCenters = IsDlgButtonChecked(m_Dlg, IDC_CHECK_OvCenters);
//}
//
//void rx_AlignFilter_Properties::GetOverlayValuesValue()
//{
//	m_OverlayValues = IsDlgButtonChecked(m_Dlg, IDC_CHECK_OvValues);
//}
//
//void rx_AlignFilter_Properties::GetOverlayDistancesValue()
//{
//	m_OverlayDistances = IsDlgButtonChecked(m_Dlg, IDC_CHECK_OvDistances);
//}
//
//void rx_AlignFilter_Properties::GetCrossColorDialog(HWND hwnd)
//{
//	CHOOSECOLOR cc;                 // common dialog box structure 
//	static COLORREF acrCustClr[16]; // array of custom colors 
//	HBRUSH hbrush;                  // brush handle
//
//	// Initialize CHOOSECOLOR 
//	ZeroMemory(&cc, sizeof(cc));
//	cc.lStructSize = sizeof(cc);
//	cc.hwndOwner = hwnd;
//	cc.lpCustColors = (LPDWORD)acrCustClr;
//	cc.rgbResult = m_CrossColor;
//	cc.Flags = CC_FULLOPEN | CC_RGBINIT;
//
//	if (ChooseColor(&cc) == TRUE)
//	{
//		hbrush = CreateSolidBrush(cc.rgbResult);
//		m_CrossColor = cc.rgbResult;
//	}
//}
//
//void rx_AlignFilter_Properties::GetTextColorDialog(HWND hwnd)
//{
//	CHOOSECOLOR cc;                 // common dialog box structure 
//	static COLORREF acrCustClr[16]; // array of custom colors 
//	HBRUSH hbrush;                  // brush handle
//
//	// Initialize CHOOSECOLOR 
//	ZeroMemory(&cc, sizeof(cc));
//	cc.lStructSize = sizeof(cc);
//	cc.hwndOwner = hwnd;
//	cc.lpCustColors = (LPDWORD)acrCustClr;
//	cc.rgbResult = m_TextColor;
//	cc.Flags = CC_FULLOPEN | CC_RGBINIT;
//
//	if (ChooseColor(&cc) == TRUE)
//	{
//		hbrush = CreateSolidBrush(cc.rgbResult);
//		m_TextColor = cc.rgbResult;
//	}
//}
//
//void rx_AlignFilter_Properties::GetTextFontDialog(HWND hwnd)
//{
//	CHOOSEFONT cf;            // common dialog box structure
//	LOGFONT lf;        // logical font structure
//
//	GetObject(m_TextFont, sizeof(LOGFONT), &lf);
//
//	// Initialize CHOOSEFONT
//	ZeroMemory(&cf, sizeof(cf));
//	cf.lStructSize = sizeof(cf);
//	cf.hwndOwner = hwnd;
//	cf.lpLogFont = &lf;
//	cf.rgbColors = m_TextColor;
//	cf.Flags = CF_INITTOLOGFONTSTRUCT | CF_FIXEDPITCHONLY | CF_SCREENFONTS; // CF_NOSCRIPTSEL;
//
//	if (ChooseFont(&cf) == TRUE)
//	{
//		m_TextFont = CreateFontIndirect(cf.lpLogFont);
//	}
//}
//
//void rx_AlignFilter_Properties::GetBlobColorDialog(HWND hwnd)
//{
//	CHOOSECOLOR cc;                 // common dialog box structure 
//	static COLORREF acrCustClr[16]; // array of custom colors 
//	HBRUSH hbrush;                  // brush handle
//
//	// Initialize CHOOSECOLOR 
//	ZeroMemory(&cc, sizeof(cc));
//	cc.lStructSize = sizeof(cc);
//	cc.hwndOwner = hwnd;
//	cc.lpCustColors = (LPDWORD)acrCustClr;
//	cc.rgbResult = m_BlobColor;
//	cc.Flags = CC_FULLOPEN | CC_RGBINIT;
//
//	if (ChooseColor(&cc) == TRUE)
//	{
//		hbrush = CreateSolidBrush(cc.rgbResult);
//		m_BlobColor = cc.rgbResult;
//	}
//}
//
//void rx_AlignFilter_Properties::GetBlobAreaDivisorValueText()
//{
//	//Get Value from Text
//	TCHAR szAreaDivisor[STR_MAX_LENGTH];
//	Edit_GetText(GetDlgItem(m_Dlg, IDC_EDIT_BlobAreaDivisor), szAreaDivisor, STR_MAX_LENGTH);
//	// Convert Multibyte string to ANSI
//	char szANSI[STR_MAX_LENGTH];
//	int rc = WideCharToMultiByte(CP_ACP, 0, szAreaDivisor, -1, szANSI, STR_MAX_LENGTH, NULL, NULL);
//	m_BlobAreaDivisor = (UINT)atof(szANSI);
//	if (m_BlobAreaDivisor < 0)m_BlobAreaDivisor = 0;
//}
//
//void rx_AlignFilter_Properties::GetBlobAspectLimitValueText()
//{
//	//Get Value from Text
//	TCHAR szAspectLimit[STR_MAX_LENGTH];
//	Edit_GetText(GetDlgItem(m_Dlg, IDC_EDIT_BlobAspectLimit), szAspectLimit, STR_MAX_LENGTH);
//	// Convert Multibyte string to ANSI
//	char szANSI[STR_MAX_LENGTH];
//	int rc = WideCharToMultiByte(CP_ACP, 0, szAspectLimit, -1, szANSI, STR_MAX_LENGTH, NULL, NULL);
//	m_BlobAspectLimit = (UINT)atof(szANSI);
//	if (m_BlobAspectLimit < 0)m_BlobAspectLimit = 0;
//}
//
//void rx_AlignFilter_Properties::GetMeasureModeValue(UINT Mode)
//{
//	m_MeasureMode = Mode;
//}
//
//void rx_AlignFilter_Properties::GetMeasurePixelsValue()
//{
//	m_MeasurePixels = IsDlgButtonChecked(m_Dlg, IDC_CHECK_MeasureInPixels);
//}
//
////Calibrate
//void rx_AlignFilter_Properties::GetCalibrateDistanceValueText()
//{
//	//Get Value from Text
//	TCHAR szCalibrateDistance[STR_MAX_LENGTH];
//	Edit_GetText(GetDlgItem(m_Dlg, IDC_EDIT_CalibrateDistance), szCalibrateDistance, STR_MAX_LENGTH);
//	// Convert Multibyte string to ANSI
//	char szANSI[STR_MAX_LENGTH];
//	int rc = WideCharToMultiByte(CP_ACP, 0, szCalibrateDistance, -1, szANSI, STR_MAX_LENGTH, NULL, NULL);
//	m_CalibrateDistance = (UINT)atof(szANSI);
//}
//
//void rx_AlignFilter_Properties::GetCalibrateFreezeValue()
//{
//	m_CalibrateFreeze = IsDlgButtonChecked(m_Dlg, IDC_CHECK_CalibrateFreeze);
//}
//
//void rx_AlignFilter_Properties::SetCalibrationButtons(BOOL SwitchOn)
//{
//	Button_Enable(GetDlgItem(m_Dlg, IDC_EDIT_CalibrateDistance), SwitchOn);
//	Button_Enable(GetDlgItem(m_Dlg, IDC_BUTTON_CalibrateCancel), SwitchOn);
//	Button_Enable(GetDlgItem(m_Dlg, IDC_BUTTON_CalibrateSave), SwitchOn);
//	Button_SetCheck(GetDlgItem(m_Dlg, IDC_CHECK_CalibrateFreeze), BST_UNCHECKED);
//	Button_Enable(GetDlgItem(m_Dlg, IDC_CHECK_CalibrateFreeze), SwitchOn);
//}
//
////Line Direction
//void rx_AlignFilter_Properties::GetAngleHoriValue()
//{
//	m_AngleHori = IsDlgButtonChecked(m_Dlg, IDC_RADIO_AngleHori);
//}
//
//void rx_AlignFilter_Properties::GetStitchHoriValue()
//{
//	m_StitchHori = IsDlgButtonChecked(m_Dlg, IDC_RADIO_StitchHori);
//}
//
//void rx_AlignFilter_Properties::GetRegisterHoriValue()
//{
//	m_RegisterHori = IsDlgButtonChecked(m_Dlg, IDC_RADIO_RegisterHori);
//}
//
//
//
//
//
