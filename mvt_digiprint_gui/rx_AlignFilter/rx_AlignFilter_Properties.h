//#pragma once
//
//
//class rx_AlignFilter_Properties : public CBasePropertyPage
//{
//
//public:
//
//	static CUnknown * WINAPI CreateInstance(LPUNKNOWN lpunk, HRESULT *phr);
//
//private:
//
//	//Dialog
//	rx_AlignFilter_Properties(LPUNKNOWN lpunk, HRESULT *phr);
//	INT_PTR OnReceiveMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
//	HRESULT OnConnect(IUnknown *pUnknown);
//	HRESULT OnDisconnect();
//	HRESULT OnActivate();
//	HRESULT OnDeactivate();
//	HRESULT OnApplyChanges();
//
//	//Dialog Values
//	void	GetControlValues();
//	void	GetDspTimingValue();
//	void	SetFrameTime(long FrameTime);
//
//	//Binarize
//	void	GetBinarizeValue();
//	void	GetAutomaticValue();
//	void	GetThresholdValueSlide();
//	void	GetThresholdValueText();
//	void	GetShowHistogramValue();
//	void	SetThresholdValueInt(UINT Threshold);
//
//	//Dilate/Erode
//	void	GetDilateErodesValueText();
//	void	GetExtraErodesValueText();
//	void	GetErodeSeedXValueText();
//	void	GetErodeSeedYValueText();
//
//	//Blob
//	void	GetCrossColorBlobDialog(HWND hwnd);
//
//	//Measurement
//	void	GetShowOriginalValue();
//	void	GetOverlayBlobsValue();
//	void	GetOverlayCentersValue();
//	void	GetOverlayValuesValue();
//	void	GetOverlayDistancesValue();
//	void	GetBlobAreaDivisorValueText();
//	void	GetBlobAspectLimitValueText();
//	void	GetCrossColorDialog(HWND hwnd);
//	void	GetTextColorDialog(HWND hwnd);
//	void	GetTextFontDialog(HWND hwnd);
//	void	GetBlobColorDialog(HWND hwnd);
//	void	GetMeasureModeValue(UINT Mode);
//	void	GetMeasurePixelsValue();
//
//	//Calibrate
//	void GetCalibrateDistanceValueText();
//	void GetCalibrateFreezeValue();
//	void SetCalibrationButtons(BOOL SwitchOn);
//
//	//Line Direction
//	void	GetAngleHoriValue();
//	void	GetStitchHoriValue();
//	void	GetRegisterHoriValue();
//
//
//	//Dialog
//	//rx_AlignFilter_Effect *m_pIPEffect;	// The custom interface on the filter
//	BOOL m_bIsInitialized;					// Used to ignore startup messages
//	HWND m_PropHwnd;						//Dialog Handle
//
//	//Info
//	LPCWSTR m_DeviceName;
//	BOOL m_DspTiming;
//	long m_FrameTime;
//
//	//Binarize
//	BOOL m_Binarize;
//	BOOL m_OldBinarize;
//	BOOL m_Automatic;
//	BOOL m_OldAutomatic;
//	UINT m_Threshold;
//	UINT m_OldThreshold;
//	BOOL m_ShowHistogram;
//	BOOL m_OldShowHistogram;
//
//	//Dilate/Erode
//	UINT m_DilateErodes;
//	UINT m_OldDilateErodes;
//	UINT m_ExtraErodes;
//	UINT m_OldExtraErodes;
//	UINT m_ErodeSeedX;	
//	UINT m_OldErodeSeedX;
//	UINT m_ErodeSeedY;
//	UINT m_OldErodeSeedY;
//	UINT m_ErodeCoreX;	
//	UINT m_OldErodeCoreX;
//	UINT m_ErodeCoreY;
//	UINT m_OldErodeCoreY;
//
//	//Blob
//	COLORREF m_CrossColorBlob;
//	COLORREF m_OldCrossColorBlob;
//
//	//Measurement
//	BOOL m_ShowOriginal;
//	BOOL m_OldShowOriginal;
//	BOOL m_OverlayBlobs;
//	BOOL m_OldOverlayBlobs;
//	BOOL m_OverlayCenters;
//	BOOL m_OldOverlayCenters;
//	BOOL m_OverlayValues;
//	BOOL m_OldOverlayValues;
//	BOOL m_OverlayDistances;
//	BOOL m_OldOverlayDistances;
//	UINT m_BlobAreaDivisor;
//	UINT m_OldBlobAreaDivisor;
//	UINT m_BlobAspectLimit;
//	UINT m_OldBlobAspectLimit;
//	COLORREF m_CrossColor;
//	COLORREF m_OldCrossColor;
//	COLORREF m_TextColor;
//	COLORREF m_OldTextColor;
//	HFONT m_TextFont;
//	HFONT m_OldTextFont;
//	COLORREF m_BlobColor;
//	COLORREF m_OldBlobColor;
//	UINT m_MeasureMode;
//	UINT m_OldMeasureMode;
//	BOOL m_MeasurePixels;
//	BOOL m_OldMeasurePixels;
//
//	//Line Direction
//	BOOL m_AngleHori;
//	BOOL m_OldAngleHori;
//	BOOL m_StitchHori;
//	BOOL m_OldStitchHori;
//	BOOL m_RegisterHori;
//	BOOL m_OldRegisterHori;
//
//	//Calibrate
//	bool m_Calibrate;
//	bool m_CalibrateFreeze;
//	UINT m_CalibrateDistance;
//};
//
//
