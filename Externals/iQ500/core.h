/**
*    \file     core.h
*    \copyright  Bobst 2015 (c)
*    \brief      IQ Core API
*/

#ifndef IQCORE_H
#define IQCORE_H
#pragma once

#if defined(IQCORE_LIBRARY_DLL)
#if defined(IQCORELIBRARY_EXPORT) // inside DLL
#   define IQDLL  __declspec(dllexport)
#else // outside DLL
#   define IQDLL  __declspec(dllimport)
#endif
#else
#define IQDLL
#endif

#ifdef __cplusplus
  #include <cstdint>
extern "C"
{
  #define as_uint32_t :uint32_t
  #define as_int32_t  :int32_t
#else
  #include <wchar.h>
  #include <stdint.h>
  #include <stdbool.h>
  #define as_uint32_t
  #define as_int32_t
#endif

   typedef enum TIqError as_uint32_t
   {
      IQ_OK                                 = 0x00000000,
      IQ_ERROR_FILE_NOT_FOUND               = 0x00000001,
      IQ_ERROR_ACQ                          = 0x00000002,
      IQ_ERROR_VISION                       = 0x00000003,
      IQ_ERROR_DB                           = 0x00000004,
      IQ_ERROR_RPC                          = 0x00000005,
      IQ_ERROR_STATE                        = 0x00000006,
      IQ_ERROR_FORMAT_NOT_FOUND             = 0x00000007,
      IQ_ERROR_ALREADY_EXISTS               = 0x00000008,
      IQ_ERROR_ALIGN_MODEL                  = 0x00000009,
      IQ_ERROR_MACHINE_RESET_PRESSED        = 0x0000000A,
      IQ_ERROR_GI                           = 0x0000000C,
      IQ_ERROR_PDF_MATCHING_FAILED          = 0x0000000D,
      IQ_ERROR                              = 0x0000000B
   } IqError;

   typedef enum TIqErrorToggle as_uint32_t
   {
      IQ_ERROR_CONNECTION                       = 0x10000000,
      IQ_ERROR_OVERSPEED_PROCESSING             = 0x10000001,
      IQ_ERROR_OVERSPEED_MACHINE                = 0x10000002,
      IQ_ERROR_TEMP_INLET_TOO_HOT               = 0x11000001,
      IQ_ERROR_TEMP_INLET_TOO_COLD              = 0x11100001,
      IQ_ERROR_TEMP_CPU1_TOO_HOT                = 0x11000002,
      IQ_ERROR_TEMP_CPU1_TOO_COLD               = 0x11100002,
      IQ_ERROR_TEMP_CPU2_TOO_HOT                = 0x11000003,
      IQ_ERROR_TEMP_CPU2_TOO_COLD               = 0x11100003,
      IQ_ERROR_TEMP_HD_TOO_HOT                  = 0x11000004,
      IQ_ERROR_TEMP_HD_TOO_COLD                 = 0x11100004,
      IQ_ERROR_TEMP_PCI1_TOO_HOT                = 0x11000005,
      IQ_ERROR_TEMP_PCI1_TOO_COLD               = 0x11100005,
      IQ_ERROR_TEMP_PCI2_TOO_HOT                = 0x11000006,
      IQ_ERROR_TEMP_PCI2_TOO_COLD               = 0x11100006,
      IQ_ERROR_TEMP_PCI3_TOO_HOT                = 0x11000007,
      IQ_ERROR_TEMP_PCI3_TOO_COLD               = 0x11100007,
      IQ_ERROR_TEMP_PCI4_TOO_HOT                = 0x11000008,
      IQ_ERROR_TEMP_PCI4_TOO_COLD               = 0x11100008,
      IQ_ERROR_TEMP_PCI5_TOO_HOT                = 0x11000009,
      IQ_ERROR_TEMP_PCI5_TOO_COLD               = 0x11100009,
      IQ_ERROR_TEMP_PCI6_TOO_HOT                = 0x1100000A,
      IQ_ERROR_TEMP_PCI6_TOO_COLD               = 0x1110000A,
      IQ_ERROR_TEMP_PCI7_TOO_HOT                = 0x1100000B,
      IQ_ERROR_TEMP_PCI7_TOO_COLD               = 0x1110000B,
      IQ_ERROR_TEMP_PCI8_TOO_HOT                = 0x1100000C,
      IQ_ERROR_TEMP_PCI8_TOO_COLD               = 0x1110000C,
      IQ_ERROR_TEMP_PCI9_TOO_HOT                = 0x1100000D,
      IQ_ERROR_TEMP_PCI9_TOO_COLD               = 0x1110000D,
      IQ_ERROR_TEMP_GPU1_TOO_HOT                = 0x11100010,
      IQ_ERROR_TEMP_GPU2_TOO_HOT                = 0x11100011,
      IQ_ERROR_TEMP_GPU3_TOO_HOT                = 0x11100012,
      IQ_ERROR_TEMP_GPU4_TOO_HOT                = 0x11100013,
      IQ_ERROR_CAMERA_DISCONNECTED              = 0x12000000,
      IQ_ERROR_DISK_C_FULL                      = 0x12000001,
      IQ_ERROR_DISK_D_FULL                      = 0x12000002,
      IQ_ERROR_DISK_FAILURE                     = 0x12000003,
      IQ_ERROR_RAID_FAILURE                     = 0x12000004,
	   IQ_ERROR_PWR_SPLY1_FAILURE                = 0x12000005,
	   IQ_ERROR_PWR_SPLY2_FAILURE                = 0x12000006,
      IQ_ERROR_ML_IMAX                          = 0x13000005,
    //IQ_ERROR_GL_IMAX                          = 0x13000006, // Not used anymore
    //IQ_ERROR_ML_TEMP_TOO_HOT                  = 0x13000007, // Not used anymore
    //IQ_ERROR_GL_TEMP_TOO_HOT                  = 0x13000008, // Not used anymore
      IQ_ERROR_ML_AUTO_TEST                     = 0x13000009,
      IQ_ERROR_GL_AUTO_TEST                     = 0x13000010,
      IQ_ERROR_WATCHDOG_LIGHTNING               = 0x13000011,
      IQ_ERROR_48V_DEFECT                       = 0x14000000,
      IQ_ERROR_CLIMATISEUR_DEFECT               = 0x14000001,
      IQ_ERROR_CLIMATISEUR_WARNING              = 0x14000008,
      IQ_ERROR_24V_DEFECT                       = 0x14000002,
      IQ_ERROR_115V_DEFECT                      = 0x14000003,
      IQ_ERROR_ETHERCAT_DEFECT                  = 0x14000004,
      IQ_ERROR_CHILLER_ALARM                    = 0x14000005,
      IQ_ERROR_MACHINE_NOT_READY_FOR_QC         = 0x14000006,
      IQ_ERROR_CHILLER_WARNING                  = 0x14000007, //WARNING
      IQ_ERROR_EMERGENCY_STOP_BUTTON_PRESSED    = 0x14000009,
      IQ_ERROR_MULTILIGHT_BOARDS_TEMP_MAX       = 0x14000010,
      IQ_ERROR_CAMERA_TEMPERATURE               = 0x14000011,
      IQ_ERROR_MASTERLIGHT_TOO_COLD             = 0x14000012, //WARNING
      IQ_ERROR_MASTERLIGHT_TOO_HOT              = 0x14000013, //WARNING
      IQ_ERROR_GOLDENLIGHT_TOO_COLD             = 0x14000014, //WARNING
      IQ_ERROR_GOLDENLIGHT_TOO_HOT              = 0x14000015, //WARNING
      IQ_ERROR_ML_AMBIENT_TEMPERATURE_TOO_HOT   = 0x14000016, //WARNING
      IQ_ERROR_CALIBRATION_CONVEYOR_NOTINPLACE  = 0x14000017,
      IQ_ERROR_COMMUNICATION_HTTP_MACHINE       = 0x14000018, //WARNING
      IQ_ERROR_CONVEYOR_REGULATION              = 0x14000019,
      IQ_ERROR_CONVEYOR_LEARNING                = 0x14000020,
      IQ_ERROR_CONVEYOR_SENSOR_NOPULSE          = 0x14000021,
      IQ_ERROR_CONVEYOR_GI_NOPULSE              = 0x14000022,
      IQ_ERROR_CONVEYOR_LEARNING_IN_PROGRESS    = 0x14000023,
      IQ_ERROR_MACHINE_EJECTION_STOPPED         = 0x14000024, //WARNING
      IQ_ERROR_UPS_EXTERNAL_POWER_VARIATIONS    = 0x15000000, //WARNING
      IQ_ERROR_UPS_CHANGE_BATTERY               = 0x15000001, //WARNING
      IQ_ERROR_UPS_OVERLOAD                     = 0x15000002,
      IQ_ERROR_UPS_INSUFFICIENT_RUNTIME         = 0x15000003,
      IQ_ERROR_UPS_TEMPERATURE_BATTERY_HIGH     = 0x15000004,
      IQ_ERROR_UPS_TEMPERATURE_BATTERY_WARNING  = 0x15000005, //WARNING
      IQ_ERROR_UPS_TEMPERATURE_BATTERY_CRITICAL = 0x15000006, //UNUSED
      IQ_ERROR_UPS_POWER_FAILED_WARNING         = 0x15000007, //WARNING
      IQ_ERROR_UPS_CRITICAL_HARDWARE_FAULT      = 0x15000008,
      IQ_ERROR_UPS_BATTERY_DISCHARGED           = 0x15000009, //WARNING
      IQ_ERROR_UPS_NO_BATTERY                   = 0x15000010,
      IQ_ERROR_UPS_LOW_BATTERY_WARNING          = 0x15000011, //WARNING
      IQ_ERROR_UPS_LOAD_ALARM_VIOLATION         = 0x15000012, //WARNING
      IQ_ERROR_UPS_COMMUNICATION                = 0x15000013,
      IQ_ERROR_PDF_CONTOUR_FAILED               = 0x16000000,
      IQ_INTERNAL_48V_IS_OFF                    = 0x20000000, // DO NOT NOTIFY IN HMI
      IQ_INTERNAL_LLCB_ERROR                    = 0x20000001 // DO NOT NOTIFY IN HMI
   } IqErrorToggle;

   typedef enum TStatesEnum as_uint32_t
   {
      STATE_OFF,
      STATE_INIT,
      STATE_IDLE,
      STATE_RM,
      STATE_PROD_LEARNING,
      STATE_PROD,
      STATE_DIG_CALIB
   } StatesEnum;

   // Should have the same values in database schema
   typedef enum TMachineType as_uint32_t
   {
      PCR                                 = 0,
      CHAMPLAIN                           = 1,
      FLEXO                               = 2,
      HFS                                 = 3,
      DIGITAL                             = 4,
      IQ50                                = 5,
      UNKNOWN_MACHINETYPE                 = 1000
   } MachineType;

   typedef enum TProdLearningType as_uint32_t
   {
      PRODLRNTYPE_NONE                    = 0,
      PRODLRNTYPE_REFCHECK                = 1,
      PRODLRNTYPE_CONFCHECK               = 2
   } ProdLearningType;

   typedef enum TCameraType as_uint32_t
   {
      CAMERALINK_MATROX                   = 0,
      IDB_READER                          = 1,
      GENICAM_SYNCHRO                     = 2,
      GENICAM_NO_SYNCHRO                  = 3,
      CAMERALINK_EURESYS                  = 4
   } CameraType;

   typedef enum TCamImageType as_uint32_t
   {
      CAMIMAGTYPE_MAIN_MULTILIGHT,
      CAMIMAGTYPE_MAIN_MULTILIGHT_CAM1,
      CAMIMAGTYPE_MAIN_MULTILIGHT_CAM2,
      CAMIMAGTYPE_MAIN_MULTILIGHT_CAM3,
      CAMIMAGTYPE_SCRATCHLIGHT,
      CAMIMAGTYPE_WEBCAM
   } CamImageType;

   // Should have the same values in database schema 
   typedef enum as_uint32_t // Values are same in DB
   {
      MASTER_IMAGE = 0,
      MEAN_IMAGE = 1,
      STD_IMAGE = 2, // NOT USABLE YET
      MAXDIFF_IMAGE = 3,
      MAXDIFFWITHACCEPTEDDEFECTS_IMAGE = 4,
      MAXDIFFBLURRED_IMAGE = 18,
      MAXDIFFBLURREDWITHACCEPTEDDEFECTS_IMAGE = 19,
      TXTTHRESHOLDWITHACCEPTEDDEFECTS_IMAGE = 5,
      THRESHOLD_IMAGE = 6,
      MASTERCROPPED_IMAGE = 9,
      PDFMATCHED_IMAGE = 17,
      VIRTUALMASTER_IMAGE = 20
   } JobImageType;

   typedef enum TDigCalibrationMode as_uint32_t
   {
      DIGCALIBRATIONMODE_STITCHING
   } DigCalibrationMode;

   typedef enum TDigCalibrationColorType as_uint32_t
   {
      DIGCALIBRATIONCOLORTYPE_C,
      DIGCALIBRATIONCOLORTYPE_M,
      DIGCALIBRATIONCOLORTYPE_Y,
      DIGCALIBRATIONCOLORTYPE_K,
      DIGCALIBRATIONCOLORTYPE_O, //Orange
      DIGCALIBRATIONCOLORTYPE_B, //BLue
      DIGCALIBRATIONCOLORTYPE_V, //Violet
      DIGCALIBRATIONCOLORTYPE_G, //Green
      DIGCALIBRATIONCOLORTYPE_W, //White
      DIGCALIBRATIONCOLORTYPE_L  //Lacquer
   } DigCalibrationColorType;

   typedef enum as_uint32_t
   {
      FLAT_LAYER = 0,
      HILLY_LAYER = 1,
      VARNISH_LAYER = 2,
      EMBOSS_LAYER = 3
   } LayerImageType;

   typedef enum as_uint32_t
   {
      BUFFERTYPE_RGB
   } BufferType;

   typedef enum as_int32_t
   {
      UNKNOWN = -1,
      DISABLED = 0,
      ACTIVE = 1
   } ComMachineResultStatus;

#pragma pack(push)  /* push current alignment to stack */
#pragma pack(1)     /* set alignment to 1 byte boundary */

   typedef struct TCamImage
   {
      CamImageType   camType;
      uint32_t       modifiedRoiCntr;
      BufferType     bufferType;
      uint32_t       bufferRealWidth;
      uint32_t       bufferRealHeight;
      uint32_t       bufferWidth;
      uint32_t       bufferWidthStep;
      uint32_t       bufferHeight;
      uint32_t       bufferSize;
      uint8_t *      buffer;
      float          roiTop;
      float          roiLeft;
      float          roiZoom;
      uint32_t       roiBufferRealWidth;
      uint32_t       roiBufferRealHeight;
      uint32_t       roiBufferWidth;
      uint32_t       roiBufferWidthStep;
      uint32_t       roiBufferHeight;
      uint32_t       roiBufferSize;
      uint8_t *      roiBuffer;
      float          resizeFactorX;
      float          resizeFactorY;
      float          resizeFactorRoiX;
      float          resizeFactorRoiY;
      uint32_t       bufferOriginalWidthStep;
      uint32_t       roiBufferOriginalWidthStep;
   } CamImage;

   typedef enum TDataResultTypeEnum as_uint32_t
   {
      DATARESULT_RM                         = 0x00000000,
      DATARESULT_BASIC_LEARNING             = 0x00000001,
      DATARESULT_PROD_LEARNING              = 0x00000002,
      DATARESULT_PROD                       = 0x00000003,
      DATARESULT_DIG_CALIB                  = 0x00010000
   } DataResultType;

   typedef struct TRMResult
   {
      float       angle;
      uint32_t    areModelsFound;
      uint32_t    modelFound;
      uint32_t    nbOfCamType;
      uint32_t    softwareCounter;
      uint32_t *  nbOfScene;
      CamImage ** img;
   } RMResult;

   typedef struct TBasicLearningResult
   {
      uint32_t    nbOfCamType;
      uint32_t    softwareCounter;
      uint32_t *  nbOfScene;
      CamImage ** img;
   } BasicLearningResult;

   typedef struct TRect
   {
      float middleY;
      float middleX;
      float zoom;
   } Rect;

   typedef enum TTrtResultType as_uint32_t
   {
      RESULTTYPE_ALIGN                      = 0x00000000,
      RESULTTYPE_GTC                        = 0x00000001,
      RESULTTYPE_TXT                        = 0x00000002,
      RESULTTYPE_DE                         = 0x00000003,
      RESULTTYPE_REGISTER                   = 0x00000004,
      RESULTTYPE_VARNISH                    = 0x00000005,
      RESULTTYPE_BARCODE                    = 0x00000006,
      RESULTTYPE_STREAK                     = 0x00000007,
      RESULTTYPE_REFCTRL                    = 0x00000008
   } TrtResultType;

   typedef enum TTrtGtcResultType as_uint32_t
   {
      GTCRESULTTYPE_SPOT                    = 0x00000000,
      GTCRESULTTYPE_PREVENT                 = 0x00000001,
      GTCRESULTTYPE_PERSISTENT              = 0x00000002,
      GTCRESULTTYPE_HIGH_CONTRASTED         = 0x00000003,
      GTCRESULTTYPE_HALO                    = 0x00000004,
      GTCRESULTTYPE_DENSITY                 = 0x00000005
   } TrtGtcResultType;

   // If we have a GTC result, then it means there is a defect
   // A circle should be drawn arround the ROI FOR EVERY RESULT
   typedef struct TTrtResultGTC
   {
      TrtGtcResultType blobType; 
      uint32_t defectId;
      uint32_t ejectable;
      Rect roi;
      int64_t area;  // In pixels
      int32_t width; // In pixels
      int32_t height;// In pixels
      double constrast;
      int32_t mostSignifiantLayer; // -1 UNDEFINED, 0 FLAT, 1 HILLY, 2 VARNISH, 3 FOIL
      uint32_t isAccepted;
   } TrtResultGTC;

   // If we have a GTC result, then it means there is a defect
   // A circle should be drawn arround the ROI FOR EVERY RESULT
   typedef struct TTrtResultTXT
   {
      uint32_t errorStatus; // 0 is ok
      uint32_t defectId;
      Rect roi;
      int64_t area;  // In pixels
      int32_t width; // In pixels
      int32_t height;// In pixels
      double intensity;
      uint32_t ejectable;
      uint32_t isAccepted;
      uint32_t shapePrimaryKey;
   } TrtResultTXT;

   typedef struct TColor_Lab
   {
      double L;
      double a;
      double b;
   } Color_Lab;

   typedef struct TColor_RGB
   {
      double r;
      double g;
      double b;
   } Color_RGB;

   typedef struct TColor_XYZ
   {
      double x;
      double y;
      double z;
   } Color_XYZ;

   typedef enum TMeasurement_Decision as_uint32_t
   {
      MEAS_DEC_OK = 1,
      MEAS_DEC_OUT_OF_WARNING_THRESHOLD, // > warning threshold
      MEAS_DEC_OUT_OF_ERROR_THRESHOLD, // > error threshold
      MEAS_DEC_ERROR // > Error during computation
   } Measurement_Decision;

   typedef struct TTDE_Threshold
   {
      double warning;
      double error;
   } dE_Threshold;

   typedef struct TTrtResultDE
   {
      int group;
      Color_Lab lab;
      Color_RGB rgb;
      double dE;
      Measurement_Decision decision;
      dE_Threshold threshold;
   } TrtResultDE;

   typedef struct TTrtRegisterShape
   {
      uint32_t isShapeOk;
      uint32_t shapePrimaryKey;
   } TrtRegisterShape;

   typedef struct TTrtResultRegister
   {
      uint32_t regControlType1;
      uint32_t groupId1;

      uint32_t regControlType2;
      uint32_t groupId2;
      
      Measurement_Decision x_status;
      double x_offset;

      Measurement_Decision y_status;
      double y_offset;

      uint32_t nbShapes1;
      TrtRegisterShape * shapeList1;
      uint32_t nbShapes2;
      TrtRegisterShape * shapeList2;
   } TrtResultRegister;

   typedef struct TTrtResultStreak
   {
      Rect roi;
      uint32_t heightPx;
   } TrtResultStreak;

   typedef struct TTrtResultVarnish
   {
      Measurement_Decision status; // It is not possible to have MEAS_DEC_OUT_OF_WARNING_THRESHOLD value
      Rect roi;
      double value1_tobedefined;
      double value2_tobedefined;
      double value3_tobedefined;
   } TrtResultVarnish;

   typedef struct TTrtResultRefCtrl
   {
      Rect roi;
   } TrtResultRefCtrl;

   typedef enum TTrtBarcodeType as_uint32_t // Values are same in DB
   {
      BARCODETYPE_UNKNOWN = 1, // Never set for a barcodezone
      BARCODETYPE_AUTO = 2, // Never set for a result
      // 1D code
      BARCODETYPE_4_STATE = 3,
      BARCODETYPE_BC412 = 4,
      BARCODETYPE_CODABAR = 5,
      BARCODETYPE_CODE39 = 6,
      BARCODETYPE_CODE93 = 7,
      BARCODETYPE_CODE128 = 8,
      BARCODETYPE_EAN8 = 9,
      BARCODETYPE_EAN13 = 10,
      BARCODETYPE_EAN14 = 11,
      BARCODETYPE_GS1_128 = 12,
      BARCODETYPE_GS1_DATABAR = 13,
      BARCODETYPE_INDUSTRIAL25 = 14,
      BARCODETYPE_INTERLEAVED25 = 15,
      BARCODETYPE_PHARMACODE = 16,
      BARCODETYPE_PLANET = 17,
      BARCODETYPE_POSTNET = 18,
      BARCODETYPE_UPC_A = 19,
      BARCODETYPE_UPC_E = 20,
      // 2D code
      BARCODETYPE_AZTEC = 21,
      BARCODETYPE_DATAMATRIX = 22,
      BARCODETYPE_MAXICODE = 23,
      BARCODETYPE_MICROPDF417 = 24,
      BARCODETYPE_MICROQRCODE = 25,
      BARCODETYPE_PDF417 = 26,
      BARCODETYPE_QRCODE = 27,
      BARCODETYPE_TRUNCATED_PDF417 = 28
   } TrtBarcodeType;

   typedef enum TTrtBarcodeGrade as_uint32_t // Values are same in DB
   {
      BARCODEGRADE_UNKNOWN = 1,
      BARCODEGRADE_F = 2,
      BARCODEGRADE_D = 3,
      BARCODEGRADE_C = 4,
      BARCODEGRADE_B = 5,
      BARCODEGRADE_A = 6
   } TrtBarcodeGrade;

   typedef enum TTrtResultBarcodeDecision as_uint32_t // Values are same in DB
   {
      BARCODEDECISION_OK = 1, // Should not happen for the moment, only defects are sent
      BARCODEDECISION_GRADE_TOO_LOW = 2,
      BARCODEDECISION_UNKNOWN_DATA = 3, // Happens when a list of code is provided and not in the list
      BARCODEDECISION_INSPECTION_ERROR = 4, // Happens if the bar code cannot be read
   } TrtResultBarcodeDecision;

   typedef struct TTrtResultBarCode
   {
      TrtBarcodeGrade grade;
      TrtResultBarcodeDecision decision; // Ejected if value is different from BARCODEDECISION_OK !
      Rect roi;
      uint32_t crc; // NOT USED FOR HMI, JUST FOR TESTING
   } TrtResultBarcode;

   typedef struct TTrtResultAlign
   {
      uint32_t isModelOk;
      uint32_t shapePrimaryKey;
   } TrtResultAlign;

   typedef struct TTrtResult
   {
      TrtResultType type;
      uint32_t errorStatus; // 0 is ok, it means that the algorithm occured corretly
      uint32_t nbOfValue; // Number of values in values array
      void ** values;
   } TrtResult;

   typedef struct TProdLearningResult
   {
      uint32_t nbOfTrtResult;
      TrtResult * trtResult;
      uint32_t prdLearningCounter;
      uint32_t hasDefect; // Added for conformity check
      uint32_t nbOfCamType;
      uint32_t softwareCounter;
      uint32_t * nbOfScene;
      CamImage ** img;
      CamImage * overlay; // Added for conformity check
   } ProdLearningResult;

   typedef struct TProdResult
   {
      float       angle;
      uint32_t nbOfTrtResult;
      TrtResult * trtResult;
      uint32_t isEjected;
      uint32_t hasDefect;
      uint64_t timeOfAcqMsSinceEpoch;
      uint32_t nbOfCamType;
      uint32_t softwareCounter;
      uint32_t * nbOfScene;
      CamImage ** img;
      CamImage * overlay;
   } ProdResult;

   typedef enum TDigCalibResultStatus as_uint32_t
   {
      IQDIGCALIB_OK,
      IQDIGCALIB_ERROR = 0xFFFFFFFF //TODO Implement the different known errors
   } DigCalibResultStatus;

   typedef struct TDigCalibStitchingHeadResult
   {
      double positionXMm; // Millimeters
      double positionYMm; // Millimeters
      double c2cPositionXMm; // Millimeters
      double c2cPositionYMm; // Millimeters
      double rotationDeg;
      double skewX;
      double skewY;
      double scaleX;
      double scaleY;
      uint32_t * nozzlesNotMatchedArray; // ColorBarReference (1-16256 with 8 heads) ? PrintHeadReference (1-2176) ? NozzleIndex (1-2048) ?
      uint32_t nbOfNozzlesNotMatched;
      uint32_t nbOfMatchedNozzle;
      uint32_t nbOfDetectedNozzle;
   } DigCalibStitchingHeadResult;

   typedef struct TDigCalibStitchingBarResult
   {
      DigCalibResultStatus status;
      int32_t colorBarId;
      DigCalibStitchingHeadResult * resultsArray;
      uint32_t nbOfResults; // Equal to number of printing head
      uint32_t softwareCounter;
   } DigCalibStitchingBarResult;

   typedef struct TPairInt
   {
      int x;
      int y;
   } PairInt;

   typedef struct TDefectGenDescription
   {
      // What
      int width_min;
      int width_max;
      int height_min;
      int height_max;
      int fixed_size_pattern_elem_nbpixels;
      int fixed_size_pattern_nb_repetitions_x;
      int fixed_size_pattern_nb_repetitions_y;
      int fixed_size_pattern_nb_repetitions;
      int fixed_size_pattern_internal_margin_x;
      int fixed_size_pattern_internal_margin_y;

      // Where
      int fixed_top_when_cam_fixed;
      int fixed_left_when_cam_fixed;
      int margin_x; //used to localize random defects
      int margin_y; //used to localize random defects
      int cam; // fixed if 0 <= cam < NB_CAMS, random otherwise

      // When
      int randomtime; //0
      int delay;//0
      int duration;//1
      int replay; // 1 = yes, else = no

      // Action
      int action_strategy; // 0=blobs, 1=abs, 2=add, 3=sub, 4=abs (before warp), 5=add (before warp), 6=sub (before warp)
      int graylevel; // 1-255 (limited to this range)
      int scene; // scene id to consider (0 <= scene < NB_SCENES; randomized otherwise)
      int band; // band id to consider (0 <= band < 3; randomized otherwise)
   } DefectGenDescription;

   typedef struct TJobStat
   {
      uint32_t nbOfFormatInspectedDuringOp;
      uint32_t nbOfFormatSeenDuringOp;
      uint32_t nbOfFormatWarningDuringOp;
      uint32_t nbOfFormatEjectedDuringOp;
      uint64_t timeElapsedInInspection; // Time elapsed in inspection/production for the current job operation. (type is time_t, seconds since EPOCH)
   } JobStat;

   typedef struct TIqBuffer
   {
      uint8_t * data;
      uint64_t size;
   } IqBuffer;
   
   typedef struct TPEJob
   {
      int32_t jobId;
      uint32_t nbOfCopies;
      int32_t marginXMm;
      int32_t marginYMm;
      int32_t shiftOffsetXMm;
      int32_t shiftOffsetYMm;
      int32_t nbOfInspectedCopies; // Not used in IqPELoadJobBatch
   } PEJob;

#pragma pack(pop)   /* restore original alignment from stack */

   /**
   * Initialize the IQ server
   * @param confFilePath The filepath to the configuration file (Typically config.xml)
   * @return IQ_OK When successful
   */
   IQDLL IqError IqInit( const wchar_t * confFilePath );

   /**
   * Initialize the IQ server
   * @param confFilePath The filepath to the configuration file (Typically config.xml)
   * @return IQ_OK When successful
   */
   IQDLL IqError IqInit2( const wchar_t * confFilePath, const wchar_t * overrideDataFolder, bool forceSimuMode );

   /**
   * Return the current IQServer state
   * @param state The current IQServer state
   * @return IQ_OK when successful
   */
   IQDLL IqError IqGetInternalState( StatesEnum * state );

   /**
   * Quit the application and release associated memory
   */
   IQDLL IqError IqQuit( );

#ifdef __cplusplus
}
#endif

#endif
