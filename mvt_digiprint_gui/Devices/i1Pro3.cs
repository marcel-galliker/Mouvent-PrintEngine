using RX_DigiPrint.Models;
using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;
using System.Windows.Forms;

namespace RX_DigiPrint.Devices
{
	public class I1Pro3 : IDisposable
    {
        #region Definitions
        
		private ColorConversion.SpectroResultStruct SpectroResult = new ColorConversion.SpectroResultStruct();

        private static bool sdkInitialized = false;
        private static Dictionary<IntPtr, I1Pro3> deviceMap = new Dictionary<IntPtr, I1Pro3>();
        private static DeviceEventCallback callbackFunction;

        #endregion

        #region Enums and Structs

        /// <summary>
        ///   Result enumeration
        /// </summary>
        public enum Result
        {
            e3NoError = 0,        /**< no error, success */

            /* wrong usage of functions, wrong mode, parameters, ... Fix the program flow in your app */
            e3Exception = 1,        /**< internal exception */
            e3BadBuffer = 2,        /**< size of the buffer is not large enough for the data */
            e3InvalidHandle = 9,        /**< I1_DeviceHandle is no longer valid, no device associated to this handle (device unplugged) */
            e3InvalidArgument = 10,       /**< a passed method argument is invalid (e.g. NULL) */
            e3DeviceNotOpen = 11,       /**< the device is not open. Open device first */
            e3DeviceNotConnected = 12,       /**< the device is not physically attached to the computer */
            e3DeviceNotCalibrated = 13,       /**< the device has not been calibrated or the calibration has expired */
            e3NoDataAvailable = 14,       /**< measurement not triggered, index out of range (in scan mode) */
            e3NoMeasureModeSet = 15,       /**< no measure mode has been set */
            e3NoReferenceChartLine = 17,       /**< no reference chart line for correlation set */
            e3NoSubstrateWhite = 18,       /**< no substrate white reference set */
            e3NotLicensed = 19,       /**< function not licensed (available) for this device */
            e3DeviceAlreadyOpen = 20,       /**< the device has been opened already */
            e3StandardHeadNotMounted = 21,       /**< the Standard Head is not mounted */
            e3M3HeadNotMounted = 22,       /**< the M3 Head is not mounted */
            e3AmbientHeadNotMounted = 23,       /**< the Ambient Head is not mounted */
            e3RawValuesTooLow = 24,       /**< the raw values on the white tile are lower than expected */
            e3RawValuesTooHigh = 25,       /**< the raw values on the white tile are higher than expected */

            /* user device handling error. Instruct user what to do */
            e3DeviceAlreadyInUse = 51,       /**< the device is already in use by another application */
            e3DeviceCommunicationError = 52,       /**< a USB communication error occurred, try to disconnect and reconnect the device */
            e3USBPowerProblem = 53,       /**< a USB power problem was detected. If you run the instrument on a self-powered USB hub, check the hub's power supply. If you run the instrument on a bus-powered USB hub, reduce the number of devices on the hub or switch to a self-powered USB hub */
            e3NotOnWhiteTile = 54,       /**< calibration failed because the device might not be on its white tile or the protective white tile slider is closed */

            e3StripRecognitionFailed = 60,       /**< recognition is enabled and failed. Scan again */
            e3ChartCorrelationFailed = 61,       /**< could not map scanned data to the reference chart. Scan again */
            e3InsufficientMovement = 62,       /**< distance of movement too short on the i1Pro2 ruler during scan. The device didn't move. Scan again */
            e3ExcessiveMovement = 63,       /**< distance of movement exceeds licensed i1Pro2 ruler length. Print shorter patch lines */
            e3EarlyScanStart = 64,       /**< missed patches at the beginning of a scan. The user must wait at least 500 milliseconds between pressing the button and starting to move the device */
            e3UserTimeout = 65,       /**< the user action took too long, try again quicker */
            e3IncompleteScan = 66,       /**< the user did not scan over all patches */
            e3DeviceNotMoved = 67,       /**< the user did not move during scan measurement (no Zebra Ruler data received). May be the user lifted the device */

            /* device may be corrupt. Tell user she/he should contact customer support */
            e3DeviceCorrupt = 71,       /**< an internal diagnostic detected a problem with the instruments data. Please check using i1Diagnostics to obtain more information */
            e3WavelengthShift = 72,       /**< an internal diagnostic of wavelength shift detected a problem with spectral sensor. Please check with i1Diagnostics to obtain more information */


            /*i1Io*/
            e3PositionNotReached = 2002,     /**< end position not reached (blocked by user?) */
            e3IllegalPosition = 2003,     /**< position is out of range */
            e3I1Pro3NotPlacedProperly = 2004,     /**< I1Pro3 device not placed properly in iO3 */
            e3IOWhiteBaseDirty = 2005,     /**< iO3 white tile is dirty */
            e3I1SDKVersionRequirement = 2006,     /**< i1iO3 library does not meet the I1Pro3 library requirements. Use I1Pro3 library supplied in i1iO package */
            e3Handshake = 2007,     /**< i1iO3 and I1Pro3 can't communicate together. Use I1Pro3 library supplied in i1iO3 package */
            e3PatchLength = 2008,     /**< supplied or calculated patch length does not meet the chart requirements */
            e3GapLength = 2009,     /**< supplied gap length is invalid */

            e3ScanAbort = 2110,     /**< return value in user callbacks to signal #I1PRO3_Scan to abort a scan */
            e3ScanRetry = 2111,     /**< return value in user callbacks to signal #I1PRO3_Scan to retry a scan row */
            e3ScanSkipRow = 2112,     /**< return value in user callbacks to signal #I1PRO3_Scan to skip this row and start with next row */
        }

        /// <summary>
        ///   I1 Event enumeration
        /// </summary>
        private enum I1Event
        {
            eI1Pro3Arrival = 0x11,     //< i1Pro plugged-in
            eI1Pro3Departure = 0x12,     //< i1Pro unplugged 

            eI1Pro3ButtonPressed = 0x01,     //< measure button pressed on i1Pro
            eI1Pro3ScanReadyToMove = 0x02,     //< in scan mode: I1Pro3 can be moved now. Use this event to beep, flash screen, etc. to signal user that he now can start to move the device */
            eI1Pro3HeadChanged = 0x03,     //< I1Pro3 device head was changed. When this event occurred use I1PRO3_DEVICE_HEAD_SENSOR_KEY to determine the new head configuration */

            e3I1Pro3IOArrival = 0x1011,   /**< i1iO3 plugged-in */
            e3I1Pro3IODeparture = 0x1012    /**< i1iO3 unplugged */
        }

        /// <summary>
        ///   Measurement mode enumeration
        /// </summary>
        public enum MeasurementModeType
        {
            e3Undefined,
            e3ReflectanceSpot,
            e3ReflectanceScan,
            e3ReflectanceM3Spot,
            e3ReflectanceM3Scan,
            e3EmissionSpot,
            e3EmissionScan,
            e3AmbientLightSpot
        }

        /// <summary>
        /// ButtonStatus
        /// </summary>
        public enum ButtonStatusType
        {
            e3ButtonIsPressed = 1000,
            e3ButtonNotPressed = 1001
        }

        /// <summary>
        /// result from I1PRO3_GetConnectionStatus()
        /// </summary>
        public enum ConnectionStatus
        {
            unknown = -1,
            e3InvalidConnectionHandle = 0x00,   /**< I1PRO3_DeviceHandle is no longer valid, no device associated to this handle (device unplugged) */
            eI1Pro3Closed = 0x01,   /**< I1Pro3 is plugged in, communication to device not established */
            eI1Pro3Open = 0x03,    /**< I1Pro3 is open and ready to use, communication to device established */
            e3I1Pro3IOClosed_NoI1Pro3 = 0x04,   /**< i1iO3 is connected without an I1Pro3 inside. The i1iO3 is closed */
            e3I1Pro3IOClosed_I1Pro3Closed = 0x05,   /**< i1iO3 with an I1Pro3 is connected. Both devices are closed */
            e3I1Pro3IOOpen_NoI1Pro3 = 0x0C,   /**< i1iO3 is connected without an I1Pro3 inside. The i1iO3 is open. Limited i1iO3 functionality is available, like i1iO3 Serial */
            e3I1Pro3IOOpen_I1Pro3Closed = 0x0D,   /**< i1iO3 with an I1Pro3 is connected. The i1iO3 is open. Limited i1iO3 functionality is available, like i1iO3 Serial*/
            e3I1Pro3IOOpen_I1Pro3Open = 0x0F,    /**< i1iO3 with an I1Pro3 is connected. Both devices are open and ready to use */
        }

        /// <summary>
        /// Used with SetLed()
        /// </summary>
        public enum LedActionEnum
        {
            IndicatorLedKey,        //constant white on
            IndicatorLedSucceeded,  //2 green flashes
            IndicatorLedFailed,     //2 red flashes
            IndicatorLedWrongRow,   //1 green, then coninuous red flashes
            IndicatorLedWait4Scan,  //continuous white flashes
            IndicatorLedOff,        //LED Off
            IndicatorLedIOPositionAccept,   //2 green flashes
        }

        #endregion


        #region I1Pro364.DLL Calls

        #region General

        /// <summary>
        /// Get a pointer to an array of device handles for devices currently connected to the computer
        /// </summary>
        /// <param name="pdevices">a pointer to an internal static array of device handles</param>
        /// <param name="count">number of entries in the array</param>
        /// <returns>e3NoError on success</returns>
        //I1PRO3_ResultType I1PRO3_GetDevices(I1PRO3_DeviceHandle** devices, I1PRO3_UInteger* count);
        [DllImport("i1Pro364.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern Result I1PRO3_GetDevices(ref IntPtr pdevices, ref UInt32 count);
        //The following original code from XRite works only once, as the whole array is marshalled into c# the dll cannot release the array anymore:
        //private static extern Result I1PRO3_GetDevices([MarshalAs(UnmanagedType.LPArray)] ref IntPtr[] devices, ref UInt32 count);
        //instead each element of the array has to be copied separately

        /// <summary>
        /// This function opens a device via a handle received from #I1PRO3_GetDevices.
        /// It is required to open a device handle before you can call any other function on it.
        /// Once you have finished using the device, you can close it with a call to I1PRO3_CloseDevice
        /// </summary>
        /// <param name="handle">handle to the device to be opened</param>
        /// <returns>e3NoError on success</returns>
        //I1PRO3_ResultType I1PRO3_OpenDevice(I1_DeviceHandle devHndl);
        [DllImport("i1Pro364.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern Result I1PRO3_OpenDevice(IntPtr handle);

        /// <summary>
        /// Closes the device associated with the handle, 
        /// You should only call I1PRO3_CloseDevice on a handle that has previously been opened by a call to #I1PRO3_OpenDevice.
        /// All internal buffers associated with the object are flushed and freed, and
        /// the USB connection is closed, even if the call fails.
        /// Upon return, the handle referenced by devHndl is no longer valid.
        /// Attempting to use it will result in undefined behavior.
        /// All devices are closed as well when the library is unloaded (e.g.at application exit)
        /// </summary>
        /// <param name="handle">handle to the device</param>
        /// <returns>e3NoError on success</returns>
        //I1PRO3_ResultType I1PRO3_CloseDevice (I1_DeviceHandle devHndl);
        [DllImport("i1Pro364.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern Result I1PRO3_CloseDevice(IntPtr handle);

        /// <summary>
        /// Set a global option for all devices
        /// </summary>
        /// <param name="key">a null terminated string. Must not be empty or null</param>
        /// <param name="value">a null terminated string. Must not be empty or null</param>
        /// <returns>e3NoError on success</returns>
        //I1PRO3_ResultType I1PRO3_SetGlobalOption(const char *key, const char *value);
        [DllImport("i1Pro364.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern Result I1PRO3_SetGlobalOption(string key, string value);

        /// <summary>
        /// Get a global option, writing the string to the user provided buffer
        /// If buffer is big enough, writes the result into buffer. 
        /// If the provided  buffer size is smaller than the result, the buffer is not modified and eBadBuffer is returned.
        /// To query the buffer size, pass a null buffer. It is not guaranteed that identical function calls need/return the same buffer size.
        /// A good default buffer size is 256 bytes. Keys which may require a larger buffer are marked.
        /// The buffer must be big enough to hold the trailing null character.
        /// </summary>
        /// <param name="key">a null terminated string. Must not be empty or null</param>
        /// <param name="buffer">memory location of where to store the result. May be null to query the size</param>
        /// <param name="bufferSize">a pointer to the size of the buffer. if buffer is null, a pointer to the size required to fit the option string, including the null terminator. Must not be null</param>
        /// <returns>e3NoError on success</returns>
        //I1PRO3_ResultType I1PRO3_GetGlobalOption(const char *key, char *buffer, I1_UInteger *size);
        [DllImport("i1Pro364.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern Result I1PRO3_GetGlobalOption(string key, StringBuilder buffer, ref uint bufferSize);

        /// <summary>
        /// Set a device-specific option
        /// </summary>
        /// <param name="handle">handle to the device. Must not be null</param>
        /// <param name="key">a null terminated string. Must not be empty or null</param>
        /// <param name="value">a null terminated string. Must not be empty or null</param>
        /// <returns>e3NoError on success</returns>
        //I1PRO3_ResultType I1PRO3_SetOption(I1_DeviceHandle devHndl, const char *key, const char *value);
        [DllImport("i1Pro364.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern Result I1PRO3_SetOption(IntPtr handle, string key, string value);

        /// <summary>
        /// Get a device-specific option, writing the string to a user provided buffer
        /// If the buffer is large enough, this call writes the results into buffer.
        /// If the provided buffer size is smaller than the result, the buffer is not modified and #e3BadBuffer is returned.
        /// To query the buffer size, pass a null buffer.
        /// There is an inherent race condition implied, so it is not guaranteed that same function call needs a buffer of the same size.
        /// A good default buffer size is 256 bytes. Keys which may require a bigger buffer are documented.
        /// The buffer must be big enough to hold the trailing null character.
        /// </summary>
        /// <param name="handle">handle to the device. Must not be null</param>
        /// <param name="key">a null terminated string. Must not be empty or null</param>
        /// <param name="buffer">buffer to hold the result. May be null to query the size</param>
        /// <param name="bufferSize">a pointer to the size of the buffer pointed to by buffer. if buffer is null, a pointer to the size of the option string including the null terminator. Must not be null</param>
        /// <returns>e3NoError on success</returns>
        //I1PRO3_ResultType I1PRO3_GetOption(I1_DeviceHandle devHndl, const char *key, char *buffer, I1_UInteger *size);
        [DllImport("i1Pro364.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern Result I1PRO3_GetOption(IntPtr handle, string key, StringBuilder buffer, ref uint bufferSize);

        /// <summary>
        /// Get a device-specific option as a string, for debugging purposes only
        /// </summary>
        /// <param name="handle"></param>
        /// <param name="key"></param>
        /// <returns></returns>
        //I1PRO3_ResultType I1PRO3_GetOptionD(I1PRO3_DeviceHandle devHndl, const char *key);
        [DllImport("i1Pro364.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern Result I1PRO3_GetOptionD(IntPtr handle, string key);

        /// <summary>
        /// Test status of device handle. The handle must not be opened before.
        /// eInvalidConnectionHandle is returned if the device is no longer connected to the computer.
        /// This function will not change the status of any associated device.
        /// </summary>
        /// <param name="handle">handle to the device. Must not be null</param>
        /// <returns>eInvalidConnectionHandle, eI1Pro3Closed, eI1Pro3Open</returns>
        //I1PRO3_ConnectionStatusType I1PRO3_GetConnectionStatus(I1PRO3_DeviceHandle devHndl);
        [DllImport("i1Pro364.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern ConnectionStatus I1PRO3_GetConnectionStatus(IntPtr handle);

        /// <summary>
        /// Test if the button on the I1Pro3 device has been pressed by the user. for debugging purposes.
        /// </summary>
        /// <param name="handle"></param>
        /// <returns></returns>
        //I1PRO3_ButtonStatusType I1PRO3_GetButtonStatusD(I1PRO3_DeviceHandle devHndl);
        [DllImport("i1Pro364.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern ButtonStatusType I1PRO3_GetButtonStatusD(IntPtr handle);

        #endregion

        #region Measurement

        /// <summary>
        /// Calibrates the i1 hardware device in the current measurement mode
        /// Before any calibration is done, you should select your desired measurement 
        /// mode via #I1PRO3_SetOption (I1PRO3_MEASUREMENT_MODE, mode). A calibration is only 
        /// applied to the current measurement mode.After switching to another mode,
        /// the previous calibration will be saved and restored after switching back.
        /// If the wavelength LED is licensed, reflectance spot mode is calibrated
        /// automatically in the background for non-reflectance spot modes.
        /// Calibration will be lost after a reset, disconnect or after library is 
        /// unloaded.If a calibration fails in one mode, all other modes of the device
        /// will get uncalibrated.Keep in mind that a calibration is not sustainable 
        /// for too long due to temperature changes and other factors. You can get the
        /// maximum time in seconds until the calibration expires with
        /// #I1PRO3_TIME_UNTIL_CALIBRATION_EXPIRE (assuming perfect conditions).
        /// #I1PRO3_TIME_SINCE_LAST_CALIBRATION returns the time in seconds
        /// since the last calibration. An uncalibrated measure mode returns "-1" for 
        /// both keys. A calibration is valid for a maximum of about four hours except
        /// when in Ambient-mode where it is valid for 31 days.Every calibration
        /// requires that the sensor is placed on its own white tile with the
        /// protective slider open.To check which measurement modes are available for 
        /// this device, use the #I1PRO3_AVAILABLE_MEASUREMENT_MODES option.
        /// </summary>
        /// <param name="handle">handle to the device. Must not be null</param>
        /// <returns>e3NoError on success or eNotOnWhiteTile if device was not placed on  its white tile(or white tile slider is closed).</returns>
        //I1PRO3_ResultType I1PRO3_Calibrate (I1_DeviceHandle devHndl);
        [DllImport("i1Pro364.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern Result I1PRO3_Calibrate(IntPtr handle);

        /// <summary>
        /// Triggers a measurement depending on the current measurement mode
        /// Before a measurement can be triggered, the device must be calibrated in the 
        /// desired measurement mode.In #I1PRO3_REFLECTANCE_SCAN mode, a
        /// patch recognition or correlation can be activated(see
        /// # I1PRO3_PATCH_RECOGNITION_KEY). Use #I1PRO3_GetSpectrum, #I1PRO3_GetTriStimulus or 
        /// # I1PRO3_GetDensity to fetch the results.
        /// If you enabled the correlation algorithm in the reflectance scan mode and
        /// you receive an #e3ChartCorrelationFailed error, you can check
        /// if the user moved the device too fast over the strip by getting the property
        /// #I1PRO3_PATCH_RECOGNITION_RECOGNIZED_PATCHES. This will return the
        /// number of patches recognized during the scan, and if it is less than the
        /// number of expected patches it usually indicates the scan should be repeated
        /// with a lower speed.
        /// </summary>
        /// <param name="handle">handle to the device. Must not be null</param>
        /// <returns>may return #e3DeviceNotCalibrated if a (re)calibration is necessary.</returns>
        //I1PRO3_ResultType I1PRO3_TriggerMeasurement (I1_DeviceHandle devHndl);
        [DllImport("i1Pro364.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern Result I1PRO3_TriggerMeasurement(IntPtr handle);

        /// <summary>
        /// Returns the number of available samples for the current measurement mode and result index
        /// </summary>
        /// <param name="handle">handle to the device. Must not be null</param>
        /// <returns>number of cached samples available in #I1PRO3_GetSpectrum, #I1PRO3_GetTriStimulus or #I1PRO3_GetDensity.
        /// - 0 if no measurement has been triggered
        /// - 1 if the measurement is set to spot
        /// - number of scanned samples in scan mode
        /// - -1 on failure</returns>
        //I1PRO3_Integer I1PRO3_GetNumberOfAvailableSamples (I1_DeviceHandle devHndl);
        [DllImport("i1Pro364.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern int I1PRO3_GetNumberOfAvailableSamples(IntPtr handle);


        /// Retrieve measurement results from a previously triggered measurement.
        /// To fetch the result of a previously triggered spot measurement, use 0as the Index.
        /// To fetch the result of a previously triggered scan, specify an index 
        /// between 0 and(I1PRO3_GetNumberOfAvailableSamples() - 1). If no measurement has
        /// been triggered or if the specified index is out of range, eNoDataAvailable 
        /// is returned.


        /// <summary>
        /// Get the spectrum of a previously triggered measurement
        /// </summary>
        /// <param name="handle">handle to the device. Must not be null</param>
        /// <param name="haspectrumndle">pointer to the resulting array. Must be large enoughto hold 36 floating-point values, and cannot be null</param>
        /// <param name="index">zero-based index of the spectra. For spot measurement results use zero</param>
        /// <returns>e3NoError on success</returns>
        //I1PRO3_ResultType I1PRO3_GetSpectrum(I1_DeviceHandle devHndl, float spectrum[SPECTRUM_SIZE], I1_Integer index);
        [DllImport("i1Pro364.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern Result I1PRO3_GetSpectrum(IntPtr handle, float[] spectrum, int index);

        /// <summary>
        /// Get the color vector of a previous triggered measurement
        /// </summary>
        /// <param name="handle">handle to the device. Must not be null</param>
        /// <param name="tristimulus">pointer to the resulting array. Must be large enoughto hold 3 floating-point values, and cannot be null</param>
        /// <param name="index">zero-based index of the tristimuli. For spot measurement results use zero</param>
        /// <returns>e3NoError on success</returns>
        //I1PRO3_ResultType I1PRO3_GetTriStimulus(I1PRO3_DeviceHandle devHndl, float tristimulus[TRISTIMULUS_SIZE], I1PRO3_Integer index);
        [DllImport("i1Pro364.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern Result I1PRO3_GetTriStimulus(IntPtr handle, float[] tristimulus, int index);

        /// <summary>
        /// Get all densities (CMYK) of a previously triggered measurement
        /// </summary>
        /// <param name="handle">handle to the device. Must not be null</param>
        /// <param name="densities">densities pointer to the resulting array. Must be large enough to hold 4 floating-point values, and cannot be null</param>
        /// <param name="autoDensityIndex">pointer to the autoDensityIndex which will yield the actual auto density.If null is passed as input, the auto density will not be calculated</param>
        /// <param name="index">zero-based index of the densities. For spot measurement results use zero</param>
        /// <returns>e3NoError on success</returns>
        //I1PRO3_ResultType I1PRO3_GetDensities(I1PRO3_DeviceHandle devHndl, float densities[DENSITY_SIZE], I1PRO3_Integer *autoDensityIndex, I1PRO3_Integer index);
        [DllImport("i1Pro364.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern Result I1PRO3_GetDensities(IntPtr handle, float[] densities, ref int autoDensityIndex, int index);

        /// <summary>
        /// Get the density of a previously triggered measurement
        /// </summary>
        /// <param name="handle">handle to the device. Must not be null</param>
        /// <param name="density">pointer to the resulting array. Must be large enough to hold 1 floating-point value, and cannot be null</param>
        /// <param name="index">index zero-based index of the density to retrieve. For spot measurement results use zero</param>
        /// <returns></returns>
        //I1PRO3_ResultType I1PRO3_GetDensity(I1PRO3_DeviceHandle devHndl, float *density, I1PRO3_Integer index);
        [DllImport("i1Pro364.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern Result I1PRO3_GetDensity(IntPtr handle, float[] density, int index);

        /// <summary>
        /// Set the substrate reference spectrum for tristimulus and density calculations
        /// </summary>
        /// <param name="handle">handle to the device. Must not be null</param>
        /// <param name="spectrum">spectrum of the substrate. Must not be null</param>
        /// <returns>e3NoError on success</returns>
        //I1PRO3_ResultType I1PRO3_SetSubstrate(I1PRO3_DeviceHandle devHndl, const float spectrum[SPECTRUM_SIZE]);
        [DllImport("i1Pro364.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern Result I1PRO3_SetSubstrate(IntPtr handle, float[] spectrum);


        /// <summary>
        /// Set the reference data row of a chart for the next chart correlation
        /// Sets the reference values for the next patch correlation if the correlation
        /// algorithm is activated(#I1PRO3_PATCH_RECOGNITION_CORRELATION).
        /// This method can be used only in #I1PRO3_REFLECTANCE_SCAN
        /// measurement mode with recognition correlation algorithm activated.The
        /// reference line must be set before #I1PRO3_TriggerMeasurement is
        /// called. I1PRO3_TriggerMeasurement will then try to correlate the patches. So the
        /// order of patches may be reversed when fetching the spectra/tristimulus/density
        /// values. Use the #I1PRO3_REFERENCE_CHART_COLOR_SPACE_KEY key to set
        /// the color space for the reference line data. The color space may be RGB, Lab
        /// or CMYK.This must be done before setting the reference line.For
        /// Tristimulus reference data, the array size must be 3 times the number of
        /// reference patches. For density, the array size must be 4 times the number of
        /// reference patches.
        /// </summary>
        /// <param name="handle">handle to the device. Must not be null</param>
        /// <param name="referenceChartLine">array of Tristimulus or Density referencevalues for the line.
        /// e.g. { R0, G0, B0, R1, G1, B1, ...}
        /// or {C0, M0, Y0, K0, C1, M1, ...}. Must not be null</param>
        /// <param name="lineSize">number of patches in line</param>
        /// <returns>e3NoError on success</returns>
        //I1PRO3_ResultType I1PRO3_SetReferenceChartLine(I1PRO3_DeviceHandle devHndl, const float *referenceChartLine, I1PRO3_Integer lineSize);
        [DllImport("i1Pro364.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern Result I1PRO3_SetReferenceChartLine(IntPtr handle, float[] referenceChartLine, int lineSize);


        #endregion

        #region Callback

        //typedef void (I1PRO3_CALLING_CONVENTION *FPtr_I1PRO3_DeviceEventHandler)(I1PRO3_DeviceHandle devHndl, I1PRO3_DeviceEvent event, void *context);
        public delegate void DeviceEventCallback(IntPtr device, uint eventId, IntPtr context);

        /// <summary>
        /// Registers a device event handler callback function
        /// In case of a device event, this handler is invoked in its own thread. Events will be queued.
        /// A second call to the SDK from a different thread is blocked.
        /// eg. eI1Pro3ScanReadyToMove is emitted by I1PRO3_TriggerMeasurement(). Which means,
        /// you can't make another SDK call inside your event handler until I1PRO3_TriggerMeasurement()
        /// has exit in the main thread.
        /// </summary>
        /// <param name="deviceEventFunction">a pointer to the event handler callback function. Or NULL if a registered handler should be removed</param>
        /// <param name="context">a context for the callbacks use. May be NULL</param>
        /// <returns>the previously installed event handler function or NULL if none has been installed yet</returns>
        //FPtr_I1_DeviceEventHandler I1_RegisterDeviceEventHandler (FPtr_I1_DeviceEventHandler handler, void* context);
        [DllImport("i1Pro364.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern DeviceEventCallback I1PRO3_RegisterDeviceEventHandler(DeviceEventCallback deviceEventFunction, IntPtr context);

        #endregion

        #endregion

        #region Properties

        public bool IsOpen { get; private set; }
        public bool IsWhiteCalibrated { get; private set; }
        public IntPtr Handle { get; set; }

        public string SerialNumber { get; private set; }
        public string DeviceType { get; private set; }

        public ConnectionStatus i1_Connected { get; private set; }

        #endregion

        #region Events

        /// <summary>
        /// Occurs when an instrument gets connected
        /// </summary>
        /// <param name="device">The connected device</param>
        public delegate void DeviceConnectedHandler(I1Pro3 device);
        public static event DeviceConnectedHandler DeviceConnected;

        /// <summary>
        /// Occurs when an instrument gets disconnected
        /// </summary>
        /// <param name="device">The disconnected device</param>
        public delegate void DeviceDisconnectedHandler(I1Pro3 device);
        public static event DeviceDisconnectedHandler DeviceDisconnected;

        /// <summary>
        /// Occurs when an instrument button is pressed.
        /// </summary>
        /// <param name="device">The device which's button was pressed</param>
        public delegate void ButtonPressedHandler(I1Pro3 device);
        public event ButtonPressedHandler ButtonPressed;

        /// <summary>
        /// Occurs when the instrument is ready for scanning
        /// </summary>
        /// <param name="device">The device which is ready for scanning</param>
        public delegate void ScanReadyHandler(I1Pro3 device);
        public event ScanReadyHandler ScanReady;

        /// <summary>
        /// Occurs when the device head was changed
        /// </summary>
        /// <param name="device">The device which's head was changed</param>
        public delegate void HeadChangedHandler(I1Pro3 device);
        public event HeadChangedHandler HeadChanged;

		#endregion

		#region Public

		public I1Pro3()
		{

		}
        /// <summary>
        /// Dispose function for freeing the device
        /// </summary>
        public void Dispose()
        {
            Close();
        }

        /// <summary>
        /// Close the device. The device stays valid and can be reopened.
        /// </summary>
        public void Close()
        {
            if (this.IsOpen)
            {
                Result result = I1PRO3_CloseDevice(Handle);
                HandleResult(result);
                IsOpen = false;
                IsWhiteCalibrated = false;
            }
        }

        /// <summary>
        /// Open the device for further access
        /// </summary>
        public void Open()
        {
            Result result = I1PRO3_OpenDevice(Handle);
            HandleResult(result);
            if (result == Result.e3NoError)
            {
                IsOpen = true;
                DeviceType = GetOption("DeviceTypeKey");
                SerialNumber = GetOption("SerialNumber");

                i1_Connected = I1PRO3_GetConnectionStatus(Handle);
            }
            else
            {
                i1_Connected = ConnectionStatus.unknown;
            }
        }

        /// <summary>
        /// SDK initialization function. 
        /// Gets called automatically from ListDevices.
        /// Multiple calls to SetupSDK are safe.
        /// </summary>
        public static void SetupSDK()
        {
            if (!sdkInitialized)
            {
                callbackFunction = new DeviceEventCallback(DeviceEventHandler);
                IntPtr context = new IntPtr();
                try
				{
                    DeviceEventCallback oldCallback = I1PRO3_RegisterDeviceEventHandler(callbackFunction, context);
				}
                catch (Exception ex)
				{
                    Console.WriteLine("Excwption: {0}", ex.Message);
				}

                sdkInitialized = true;

                HandleResult(I1PRO3_SetGlobalOption("OnMeasurementSuccessNoLedIndication", "1"));
            }
        }

        /// <summary>
        /// Resets all devices and terminates the SDK
        /// </summary>
        public static void CloseSDK()
        {
            HandleResult(I1PRO3_SetGlobalOption("Reset", "All"));
            sdkInitialized = false;
        }

        /// <summary>
        /// Setup for standard L*a*b* / Density measure
        /// </summary>
        /// <returns>true if successful</returns>
        public bool PrepareMeasure()
        {
            try
            {
                HandleResult(I1PRO3_SetOption(Handle, "MeasurementMode", "ReflectanceSpot"));
                HandleResult(I1PRO3_SetOption(Handle, "ResultIndexKey", "M0"));
                HandleResult(I1PRO3_SetOption(Handle, "ColorSpaceDescription.Type", "CIELab"));
                HandleResult(I1PRO3_SetOption(Handle, "Colorimetric.Illumination", "D65"));
                HandleResult(I1PRO3_SetOption(Handle, "Colorimetric.Observer", "TenDegree"));
                HandleResult(I1PRO3_SetOption(Handle, "Colorimetric.DensityStandard", "ANSII"));
            }
            catch (Exception excep)
            {
                MessageBox.Show("Error: " + excep.Message, "Measure Error", MessageBoxButtons.OK, MessageBoxIcon.Information);
                return false;
            }
            return true;
        }

        /// <summary>
        /// Performs a spot measurement at the current position
        /// </summary>
        /// <param name="ResultSpectrum">float[36] or null</param>
        /// <param name="ResultTri">float[3] or null</param>
        /// <param name="ResultDensities">float[4] or null</param>
        /// <returns>true if successful</returns>
        public bool MeasureSpot(ref float[] ResultSpectrum, ref float[] ResultTri, ref float[] ResultDensities)
        {
            try
            {
                HandleResult(I1PRO3_TriggerMeasurement(Handle));
                if (I1PRO3_GetNumberOfAvailableSamples(Handle) > 0)
                {
                    if (ResultSpectrum != null)
                    {
                        ResultSpectrum = new float[36];
                        HandleResult(I1PRO3_GetSpectrum(Handle, ResultSpectrum, 0));
                    }

                    if (ResultTri != null)
                    {
                        ResultTri = new float[3];
                        HandleResult(I1PRO3_GetTriStimulus(Handle, ResultTri, 0));
                    }

                    if (ResultDensities != null)
                    {
                        ResultDensities = new float[4];
                        int AutoDensIndex = 0;
                        HandleResult(I1PRO3_GetDensities(Handle, ResultDensities, ref AutoDensIndex, 0));
                    }
                }
                else
                {
                    MessageBox.Show("No Results available", "Measure", MessageBoxButtons.OK, MessageBoxIcon.Information);
                    return false;
                }
            }
            catch (Exception excep)
            {
                MessageBox.Show("Error: " + excep.Message, "Measure Error", MessageBoxButtons.OK, MessageBoxIcon.Information);
                return false;
            }
            return true;
        }

        /// <summary>
        /// Activates I1Pro3 LED
        /// </summary>        /// <param name="LedMode">LedActionEnum</param>
        public void SetLed(LedActionEnum LedMode)
        {
            string ModeString = Enum.GetName(typeof(LedActionEnum), LedMode);
            if (ModeString == "") ModeString = "I1PRO3_INDICATOR_LED_OFF";
            try
            {
                HandleResult(I1PRO3_SetOption(Handle, "IndicatorLedKey", ModeString));
            }
            catch (Exception excep)
            {
                MessageBox.Show("Error: " + excep.Message, "LED Error", MessageBoxButtons.OK, MessageBoxIcon.Information);
            }
        }

        /// <summary>
        /// Read all possible version info
        /// </summary>
        /// <returns>string[key, value] 0: I1Pro3_SDK_VERSION, 1: I1Pro3_HwRev, 2: I1Pro3_SupName,
        ///  3: I1PRO3_DEVICE_TYPE, 4: I1PRO3_SERIAL_NUMBER</returns>
        public string[,] Get_I1Pro3_Version()
        {
            string[,] ReplyString = new string[2, 5];
            string I1Pro3_SDK_VERSION = GetOption("SDKVersion");
            ReplyString[0, 0] = "I1Pro3_SDK_Version";
            ReplyString[1, 0] = I1Pro3_SDK_VERSION;
            string I1Pro3_HwRev = GetOption("HWRevision");
            ReplyString[0, 1] = "I1Pro3_HW_Revision";
            ReplyString[1, 1] = I1Pro3_HwRev;
            string I1Pro3_SupName = GetOption("SupplierName");
            ReplyString[0, 2] = "I1Pro3_SupName";
            ReplyString[1, 2] = I1Pro3_SupName;
            string I1PRO3_DEVICE_TYPE = GetOption("DeviceTypeKey");
            ReplyString[0, 3] = "I1Pro3_Device_Type";
            ReplyString[1, 3] = I1PRO3_DEVICE_TYPE;
            string I1PRO3_SERIAL_NUMBER = GetOption("SerialNumber");
            ReplyString[0, 4] = "I1Pro3_Serial_Number";
            ReplyString[1, 4] = I1PRO3_SERIAL_NUMBER;

            return ReplyString;
        }

        /// <summary>
        /// Does a white calibration
        /// </summary>
        /// <returns>true if successful</returns>
        public bool WhiteCalibrate()
        {
            try
            {
                PrepareMeasure();
                HandleResult(I1PRO3_Calibrate(Handle));
                IsWhiteCalibrated = true;
                SetLed(I1Pro3.LedActionEnum.IndicatorLedSucceeded);
            }
            catch (Exception excep)
            {
                SetLed(I1Pro3.LedActionEnum.IndicatorLedFailed);
                IsWhiteCalibrated = false;
                MessageBox.Show("Error: " + excep.Message, "Calibrate Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return false;
            }
            return true;
        }

        //--- Connect -------------------------------------
        public bool Connect()
		{
            SetupSDK();

            UInt32 count = 0;
            IntPtr pDevs = IntPtr.Zero;
            Result result = I1Pro3.I1PRO3_GetDevices(ref pDevs, ref count);
            if (count==0) return false;

            IntPtr deviceHdl;
            deviceHdl = (IntPtr)Marshal.PtrToStructure(pDevs, typeof(System.IntPtr));
            CreateDevice(deviceHdl);

            Open();
            if (IsOpen)
            {
                switch (i1_Connected)
                {
                    case I1Pro3.ConnectionStatus.eI1Pro3Open:
                        //Spectrometer ready
                        // Spectrometer.ButtonPressed += I1Pro3_ButtonPressed;
                        SetLed(I1Pro3.LedActionEnum.IndicatorLedIOPositionAccept);
                        string[,] Versions = Get_I1Pro3_Version();
                        //MessageBox.Show("i1Pro3 Spectrometer ready\r\n\r\n" +
                        //                Versions[0, 0] + " : " + Versions[1, 0] + "\r\n" +
                        //                Versions[0, 1] + " : " + Versions[1, 1] + "\r\n" +
                        //                Versions[0, 2] + " : " + Versions[1, 2] + "\r\n" +
                        //                Versions[0, 3] + " : " + Versions[1, 3] + "\r\n" +
                        //                Versions[0, 4] + " : " + Versions[1, 4], 
                        //                "i1Pro3", MessageBoxButtons.OK, MessageBoxIcon.Information);
                        return true;

                    default:
                        //No spectrometer found
                        MessageBox.Show("No i1Pro3 Spectrometer found", "i1Pro3", MessageBoxButtons.OK, MessageBoxIcon.Error);
                        break;
                }
            }

            return false;
		}

                /// Parses Cie and Density from spectrometer result
        /// </summary>
        /// <param name="ResultCie">(in) float[3] Cie L*a*b* values</param>
        /// <param name="ResultDensities">(in) float[3] CMYK density values</param>
        /// <param name="Result">(out) SpectroResultStruct containing Cie L*a*b* and CMYK density values</param>
        /// <returns>true if successful</returns>
        private bool Parse_i1Result(float[] ResultCie, float[] ResultDensities, ref ColorConversion.SpectroResultStruct Result)
        {
            try
            {
                Result = new ColorConversion.SpectroResultStruct();
                Result.Density = new ColorConversion.CmykStruct();
                Result.Density.C = ResultDensities[0];
                Result.Density.M = ResultDensities[1];
                Result.Density.Y = ResultDensities[2];
                Result.Density.K = ResultDensities[3];

                Result.CieLab.L = ResultCie[0];
                Result.CieLab.a = ResultCie[1];
                Result.CieLab.b = ResultCie[2];
            }
            catch(Exception excep)
            {
                return false;
            }
            return true;
        }


        public bool MeasurePoint(ref ColorConversion.SpectroResultStruct Result)
        {
            if (!IsWhiteCalibrated)
			{
                if (!WhiteCalibrate()) return false;
			}

            PrepareMeasure();

            float[] ResultSpectrum = null;
            float[] ResultCie = new float[3];
            float[] ResultDensities = new float[4];

            if (!MeasureSpot(ref ResultSpectrum, ref ResultCie, ref ResultDensities))
            {
                SetLed(I1Pro3.LedActionEnum.IndicatorLedWrongRow);
				System.Windows.Forms.MessageBox.Show("Could not perform Measurement!\n",
                                "i1Pro3", System.Windows.Forms.MessageBoxButtons.OK, System.Windows.Forms.MessageBoxIcon.Error);
                return false;
            }

            //Get Values out of Result
            if (!Parse_i1Result(ResultCie, ResultDensities, ref Result))
            {
                SetLed(I1Pro3.LedActionEnum.IndicatorLedWrongRow);
				System.Windows.Forms.MessageBox.Show("Could not parse Measurement Values!\n",
                                "i1Pro3", System.Windows.Forms.MessageBoxButtons.OK, System.Windows.Forms.MessageBoxIcon.Error);
                return false;
            }
            SetLed(I1Pro3.LedActionEnum.IndicatorLedSucceeded);
            return true;
        }

        #endregion

        #region Internal

        /// <summary>
        /// Internal handler of I1Pro events. Events from the C-SDK arrive here and are forwarded to the respective event handlers.
        /// </summary>
        /// <param name="handle">Device handle</param>
        /// <param name="eventId">Event identification (see I1Event)</param>
        /// <param name="context">Context associated with the event</param>
        private static void DeviceEventHandler(IntPtr handle, uint eventId, IntPtr context)
        {
            switch ((I1Event)eventId)
            {
                case I1Event.eI1Pro3Arrival:
                    try
                    {
                        I1Pro3 newDevice = CreateDevice(handle);
                        {
                            if (DeviceConnected != null)
                            {
                                DeviceConnected(newDevice);
                            }
                        }
                    }
                    catch (Exception)
                    {
                    }
                    break;
                default:
                    if (deviceMap.ContainsKey(handle))
                    {
                        I1Pro3 device = deviceMap[handle];
                        switch ((I1Event)eventId)
                        {
                            case I1Event.eI1Pro3Departure:
                                deviceMap.Remove(handle);
                                if (DeviceDisconnected != null)
                                {
                                    device.i1_Connected = ConnectionStatus.unknown;
                                    DeviceDisconnected(device);
                                }
                                break;
                            case I1Event.eI1Pro3ButtonPressed:
                                if (device.ButtonPressed != null)
                                {
                                    device.ButtonPressed(device);
                                }
                                break;
                            case I1Event.eI1Pro3ScanReadyToMove:
                                if (device.ScanReady != null)
                                {
                                    device.ScanReady(device);
                                }
                                break;
                            case I1Event.eI1Pro3HeadChanged:
                                if (device.HeadChanged != null)
                                {
                                    device.HeadChanged(device);
                                }
                                break;
                        }
                    }
                    break;
            }
        }

        /// <summary>
        /// Checks the result of an i1 operation and throws an I1Pro3Exception in case of an error
        /// </summary>
        /// <param name="result">The result from the C-SDK</param>
        private static void HandleResult(Result result)
        {
            if (result != Result.e3NoError)
            {
                uint BuffSize = 0;
                I1PRO3_GetGlobalOption("LastErrorText", null, ref BuffSize);
                StringBuilder ErrorText = new StringBuilder((int)BuffSize);
                I1PRO3_GetGlobalOption("LastErrorText", ErrorText, ref BuffSize);

                I1PRO3_GetGlobalOption("LastError", null, ref BuffSize);
                StringBuilder Error = new StringBuilder((int)BuffSize);
                I1PRO3_GetGlobalOption("LastError", Error, ref BuffSize);

                I1PRO3_GetGlobalOption("LastErrorNumber", null, ref BuffSize);
                StringBuilder ErrorNumber = new StringBuilder((int)BuffSize);
                I1PRO3_GetGlobalOption("LastErrorNumber", ErrorNumber, ref BuffSize);
            }

            switch (result)
            {
                case Result.e3NoError: return;
                case Result.e3Exception: throw new Exception("Internal exception");
                case Result.e3BadBuffer: throw new Exception("Size of the buffer is not large enough for the data");
                case Result.e3InvalidHandle: throw new Exception("I1PRO3_DeviceHandle is no longer valid. No device associated to this handle (device unplugged)");
                case Result.e3InvalidArgument: throw new Exception("A passed method argument is invalid (e.g. NULL)");
                case Result.e3DeviceNotOpen: throw new Exception("The device is not open. Open device first");
                case Result.e3DeviceNotConnected: throw new Exception("The device is not physically attached to the computer");
                case Result.e3DeviceNotCalibrated: throw new Exception("The device has not been calibrated or the calibration has expired");
                case Result.e3NoDataAvailable: throw new Exception("Measurement not triggered, index out of range (in scan mode)");
                case Result.e3NoMeasureModeSet: throw new Exception("No measure mode has been set");
                case Result.e3NoReferenceChartLine: throw new Exception("No reference chart line for correlation set");
                case Result.e3NoSubstrateWhite: throw new Exception("No substrate white reference set");
                case Result.e3NotLicensed: throw new Exception("Function not licensed (available) for this device");
                case Result.e3DeviceAlreadyOpen: throw new Exception("The device has been opened already");

                case Result.e3DeviceAlreadyInUse: throw new Exception("The device is already in use by another application");
                case Result.e3DeviceCommunicationError: throw new Exception("A USB communication error occurred, try to disconnect and reconnect the device");
                case Result.e3USBPowerProblem: throw new Exception("A USB power problem was detected. If you run the instrument on a self-powered USB hub, check the hub's power supply. If you run the instrument on a bus-powered USB hub, reduce the number of devices on the hub or switch to a self-powered USB hub");
                case Result.e3NotOnWhiteTile: throw new Exception("Calibration failed because the device might not be on its white tile or the protective white tile slider is closed");

                case Result.e3StripRecognitionFailed: throw new Exception("Recognition is enabled and failed. Scan again");
                case Result.e3ChartCorrelationFailed: throw new Exception("Could not map scanned data to the reference chart. Scan again");
                case Result.e3InsufficientMovement: throw new Exception("Distance of movement too short on the I1Pro3 ruler during scan. The device didn't move. Scan again");
                case Result.e3ExcessiveMovement: throw new Exception("Distance of movement exceeds licensed I1Pro3 ruler length. Print shorter patch lines");
                case Result.e3EarlyScanStart: throw new Exception("Missed patches at the beginning of a scan. The user must wait at least 500 milliseconds between pressing the button and starting to move the device");
                case Result.e3UserTimeout: throw new Exception("The user action took too long, try again quicker");
                case Result.e3IncompleteScan: throw new Exception("The user did not scan over all patches");
                case Result.e3DeviceNotMoved: throw new Exception("The user did not move during scan measurement (no Zebra Ruler data received). May be the user lifted the device");

                case Result.e3DeviceCorrupt: throw new Exception("An internal diagnostic detected a problem with the instruments data. Please check using i1Diagnostics to obtain more information");
                case Result.e3WavelengthShift: throw new Exception("An internal diagnostic of wavelength shift detected a problem with spectral sensor. Please check with i1Diagnostics to obtain more information");

                case Result.e3RawValuesTooLow: throw new Exception("Raw values too low for white calibration");


                default: throw new Exception("Unknown error");
            }
        }

        /// <summary>
        /// Access to device options
        /// </summary>
        /// <param name="key">The device option key</param>
        /// <returns>The option value</returns>
        private string GetOption(string key)
        {
            StringBuilder buffer = new StringBuilder(100);
            UInt32 bufferSize = (UInt32)buffer.Capacity;
            Result result = I1PRO3_GetOption(Handle, key, buffer, ref bufferSize);
            if (result == Result.e3BadBuffer)
            {
                buffer.Capacity = (int)bufferSize;
                result = I1PRO3_GetOption(Handle, key, buffer, ref bufferSize);
            }
            HandleResult(result);
            return buffer.ToString();
        }

        /// <summary>
        /// Setup an I1Pro364 device from the given handle.
        /// </summary>
        /// <param name="deviceHandle">The device handle</param>
        /// <returns>The new I1Pro364 device</returns>
        private static I1Pro3 CreateDevice(IntPtr deviceHandle)
        {
//            using (I1Pro3 device = new I1Pro3(deviceHandle))
            {
                I1Pro3 device = RxGlobals.I1Pro3;
                if (deviceHandle.ToInt64() == 0)
                {
                    throw new Exception("Invalid handle");
                }
				{
                    device.Handle = deviceHandle;
                    device.DeviceType = "";
                    device.SerialNumber = "";
                    device.IsOpen = false;
                    device.IsWhiteCalibrated = false;
                    device.i1_Connected = ConnectionStatus.unknown;
				}

                deviceMap.Add(deviceHandle, device);
                return device;
            }
        }


        #endregion


    }
}