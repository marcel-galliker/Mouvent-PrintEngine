/**
*    \file     coreremote.h
*    \copyright  Bobst 2015 (c)
*    \brief      IQ Core Remote TCP API
*/
#ifndef IQCOREREMOTE_H
#define IQCOREREMOTE_H
#pragma once
#include "core.h"


#ifdef __cplusplus
extern "C"
{
#endif 

   typedef void ( *DataResultCB ) (DataResultType type, void * data, void * cbArg);
   typedef void (*IqErrorCB) (IqError error, void * cbArg);
   typedef void (*IqErrorToggleCB) (IqErrorToggle error, bool status, int32_t value, void * cbArg);
   typedef void * IQREMOTEHANDLE;

   /**
   * Connect to an IQ Server given its hostname or IP address
   * @param[out] handle, handle returned to the established connection
   * @param hostname to the IQ server
   * @param timeout The application timeout for requests
   * @return IQ_OK when successful
   */
   IQDLL IqError IqConnect( IQREMOTEHANDLE * handle, const char * hostname, uint32_t timeout );

   /**
   * Disconnect from an IQ Server given its connection handle
   * @param handle, handle to the established connection
   * @return IQ_OK when successful
   */
   IQDLL IqError IqDisconnect( IQREMOTEHANDLE * handle );

   /**
   * Sets the log verbosity
   * @param verbose, set log verbosity to DEBUG level, INFO otherwise
   */
   IQDLL void IqSetLogVerbose(bool verbose);

   /**
   * Register a new display and returns a unique displayId
   * @param displayId A unique generated displayId by the server
   * @return IQ_OK
   */
   IQDLL IqError IqRegisterDisplayId( IQREMOTEHANDLE handle, uint32_t * displayId );

   /**
   * Unregister a display from the server
   * @param displayId A unique generated displayId by the server
   * @return IQ_OK
   */
   IQDLL IqError IqUnregisterDisplayId( IQREMOTEHANDLE handle, uint32_t displayId );

   /**
   * Set a callback for format results.
   * This callback should be short enough ... or it can miss some data
   * @param displayId A choosen identifier for your display. When other clients has same id, same rois will be streamed (and modified synchronously)
   * @param maxPxSizeX The maximum size X for the formats images
   * @param maxPxSizeY The maximum size Y for the formats images
   * @param cb The callback function to use in your source code
   * @param cbArg A pointer that you want to retrieved in the callback (with cbArg)
   * @return IQ_OK 
   */
   IQDLL IqError IqSetStreamedDataResultCallBack(
      IQREMOTEHANDLE handle,
      uint32_t displayId,
      uint32_t maxPxSizeX,
      uint32_t maxPxSizeY,
      DataResultCB cb, 
      void * cbArg );

   /**
   * Set a new error callback
   * @param cb The callback function to use in your source code
   * @return IQ_OK
   */
   IQDLL IqError IqSetErrorCallBack(
      IQREMOTEHANDLE handle,
      IqErrorCB cb,
     void * cbArg);

   /**
   * Set a new error callback
   * @param cb The callback function to use in your source code
   * @return IQ_OK
   */
   IQDLL IqError IqSetErrorToggleCallBack(
      IQREMOTEHANDLE handle,
     IqErrorToggleCB cb,
     void * cbArg );

   /**
   * Set a ROI size or location
   * @param softwareCounter The softwareCounter for the format you want Roiing (Last received format in the callback is good...)
   * @param camType The Camera type to use, just use MAIN_MULTILIGHT for the moment
   * @param scene The scene number to use for the ROI
   * @param middleY The middle Y position for the ROI, 0 is top - 1 is bottom
   * @param middleX The middle X position for the ROI, 0 is left - 1 is right
   * @param maxRoiPxSizeX The maximum size X for the ROI images
   * @param maxRoiPxSizeY The maximum size Y for the ROI images
   */
   IQDLL IqError IqSetRoi(
      IQREMOTEHANDLE handle,
      uint32_t softwareCounter,
      CamImageType camType,
      uint32_t scene,
      float middleY,
      float middleX,
      float zoom,
      uint32_t maxRoiPxSizeX,
      uint32_t maxRoiPxSizeY );

   /**
   * Get a ROI corresponding to the Bounding Box of the format (ONLY AVAILABLE WHEN IqSetMasterImage has been successful)
   * @param middleY The middle Y position for the ROI, 0 is top - 1 is bottom
   * @param middleX The middle X position for the ROI, 0 is left - 1 is right
   * @param zoom The applied zoom
   * @return IQ_OK
   */
   IQDLL IqError IqGetBoundingBoxRoi( IQREMOTEHANDLE handle,
                                      float * middleY,
                                      float * middleX,
                                      float * zoom );

   /**
   * Create a job in the IQ without saving it in the database, start the acquisition and go in RM_STATE
   * @param name The job name
   * @param number The job number
   * @param widthMm The job width in mm (Not so useful)
   * @param heightMm The job height in mm
   * @param jobProfileId The number of scene in the job
   * @param acqResYMmPerPx The resolution in mm per px in Y
   * @return IQ_OK when successful
   */
   IQDLL IqError IqCreateJob( IQREMOTEHANDLE handle, const wchar_t * name, const wchar_t * number, uint32_t widthMm, uint32_t heightMm, uint32_t thicknessMm, uint32_t jobProfileId, float acqResYMmPerPx );

   /**
   * Load a job in the IQ from the database, start the acquisition and go in RM_STATE
   * @param jobId The corresponding JobId in the database
   * @param operationId The corresponding operationID in the database
   * @param filepath Set the idbFilePath. Set to 'L""' if not used.
   * @return IQ_OK when successful
   */

   IQDLL IqError IqLoadJob( IQREMOTEHANDLE handle, int32_t jobId, int32_t operationId, const wchar_t * idbFilepath);

   /**
   * Copy a job PDF in the current job it locally in database.
   * @param jobId The corresponding JobId in the database
   * @param filepath Filepath to the PDF from the PDF directory defined in configuration
   * @return IQ_OK when successful
   */
   IQDLL IqError IqLoadJobPDF( IQREMOTEHANDLE handle, int32_t jobId, const wchar_t * filepath );

   /**
   * Delete a job PDF in the current job it locally in database.
   * @param jobId The corresponding JobId in the database
   * @return IQ_OK when successful
   */
   IQDLL IqError IqDeleteJobPDF( IQREMOTEHANDLE handle, int32_t jobId );

   /**
   * Get a job PDF from the jobId or from filepath in the PDF repository
   * @param jobId The corresponding JobId in the database. If -1, search in PDF repo instead
   * @param filepath Filepath to the PDF from the PDF directory defined in configuration. Required only if jobid < 0
   * @param buffer The buffer pointer corresponding the to resulting file read. Please use IqFreeBuffer to free it !!!
   * @param size The buffer size
   * @return IQ_OK when successful
   */
   IQDLL IqError IqGetPDF( IQREMOTEHANDLE handle, int32_t jobId, const wchar_t * filepath, uint8_t ** buffer, int64_t * size );

   /**
   * Reload current job data from the database (If you have loaded or saved previously the current job in IQ)
   * @return IQ_OK when successful
   */
   IQDLL IqError IqReloadCurrentJobData( IQREMOTEHANDLE handle );

   /**
   * Reload current job data and analyse the barcode zones. Once analysed, barcode zones data of the current job are
   * saved in database with updated values.
   * @return IQ_OK when successful
   */
   IQDLL IqError IqAnalyseBarcodeZones( IQREMOTEHANDLE handle );

   /**
   * Save the current job in the database (with a auto generated ID)
   * @return IQ_OK when successful
   */
   IQDLL IqError IqSaveCurrentJob( IQREMOTEHANDLE handle );

   /**
   * Unload the current job from the IQ, stop the acquisition and go in IDLE_STATE
   * @return IQ_OK when successful
   */
   IQDLL IqError IqUnloadJob( IQREMOTEHANDLE handle );

   /**
   * Start a basic learning with the next formatCount formats.
   * The basic learning is only triggeable in RM_STATE.
   * @param formatCount The number of next formats to put in basic learning mode
   * @return IQ_OK when successful
   */
   IQDLL IqError IqStartBasicLearning( IQREMOTEHANDLE handle, uint32_t formatCount );

   /**
   * Start PRODLEARNING_STATE. This state is mandatory to go in production,
   * and the IQ must be in RM_STATE or PRODUCTION_STATE.
   * A master image MUST have been set before going to this mode.
   * When going out from this mode to production, MEAN image is created.
   * When nbOfRefFormat has been reached in this state, then it goes automatically in production
   * @parameter nbOfRefFormat Number of format to run in prod learning (nb of references) before going to production
   * @return IQ_OK when successful
   */
   IQDLL IqError IqStartProdLearning(IQREMOTEHANDLE handle, uint32_t nbOfRefFormat);

   /**
   * Start PROD_STATE. This state is the PRODUCTION mode.
   * The IQ must in PROD_LEARNING to go to this state.
   * @return IQ_OK when successful
   */
   IQDLL IqError IqStartProd( IQREMOTEHANDLE handle );

   /**
   * Start RM_STATE. This state is the RM mode.
   * @return IQ_OK when successful
   */
   IQDLL IqError IqStartRM( IQREMOTEHANDLE handle );

   /**
   * Get the nb of production learning format successfull and faulty
   * @param formatCountSuccessful the number of format successfully processed in prodlearning
   * @param formatCountErrors The number of formats in errors while processing in prodlearning
   * @return IQ_OK when successful
   */
   IQDLL IqError IqGetNbOfProdLearning(IQREMOTEHANDLE handle, uint32_t * formatCountSuccessful, uint32_t * formatCountErrors);

   /**
   * Compute an angle from the received basic learning formats.
   * @param softwareCounter The format software counter to proceed angle computation
   * @param angle The returned angle estimation for the specified format
   * @return IQ_OK when successful
   */
   IQDLL IqError IqComputeAngleBasicLearning( IQREMOTEHANDLE handle, uint32_t softwareCounter, float * angle );

   /**
   * Lock a RM format to be used as a master image
   * @param softwareCounter The format software counter to proceed angle computation   
   * @return IQ_OK when successful
   */
   IQDLL IqError IqLockRMFormatForMaster( IQREMOTEHANDLE handle, uint32_t softwareCounter );

   /**
   * Set the master image to the job and compute countour and models from this image.
   * This format MUST come from the basic learning
   * @DEPRECATED Use SetMasterImageCountour and ComputeMasterModels in 2 steps instead
   * @param softwareCounter The format software counter to proceed master image
   * @param angle The rotation angle for the format
   * @param centerX The rotation center in X for the format (default is 0.5f)
   * @param centerY The rotation center in Y for the format (default is 0.5f)
   * @return IQ_OK
   */
   IQDLL IqError IqSetMasterImage( IQREMOTEHANDLE handle, uint32_t softwareCounter, float angle, float centerX, float centerY );

   /**
   * Set the master image to the job and compute countour from this image.
   * This format MUST come from the basic learning
   * @param softwareCounter The format software counter to proceed master image
   * @param angle The rotation angle for the format
   * @param centerX The rotation center in X for the format (default is 0.5f)
   * @param centerY The rotation center in Y for the format (default is 0.5f)
   * @return IQ_OK
   */
   IQDLL IqError IqSetMasterImageCountour( IQREMOTEHANDLE handle, uint32_t softwareCounter, float angle, float centerX, float centerY );

   /**
   * Compute some Models from the selected master and validated box countour
   * @return IQ_OK
   */
   IQDLL IqError IqComputeMasterModels( IQREMOTEHANDLE handle );

   /**
   * Retrieve a job image, given the JobId in the database or current job.
   * @param jobId The jobId in the IQ database OR -1 for current job loaded in server
   * @param type The image type to retrieve
   * @param numScene The scene number to retrieve
   * @param image A pointer to a CamImage structure. If successful, DO NO FORGET TO FREE "buffer" !!!
   * @param acqResXMmPerPx The resolution in width in mm per pixel. Useful to compare with acqResYMmPerPx to transform this image in squared pixel !! 
   * @param acqResYMmPerPx The resolution in height in mm per pixel. Useful to compare with acqResXMmPerPx to transform this image in squared pixel !!
   * @return IQ_OK When successful
   */
   IQDLL IqError IqGetJobImage( IQREMOTEHANDLE handle,
                                int32_t jobId,
                                JobImageType type,
                                uint32_t numScene,
                                CamImage * image,
                                float * acqResXMmPerPx,
                                float * acqResYMmPerPx
                                );

   /**
   * Delete the job references image, given the JobId in the database.
   * @param jobId The jobId in the IQ database OR -1 for current job loaded in server
   * @return IQ_OK When successful
   */
   IQDLL IqError IqDeleteJobReferences(
      IQREMOTEHANDLE handle,
      int32_t jobId
      );

   IQDLL IqError IqGetCurrentJobBoundingBox( 
      IQREMOTEHANDLE handle,
      uint32_t * top,
      uint32_t * left,
      uint32_t * bottom,
      uint32_t * right,
      float * middleX,
      float * middleY,
      float * zoom
      );

   /**
   * Retrieve a job image, given the JobId in the database or current job.
   * @param jobId The jobId in the IQ database OR -1 for current job loaded in server
   * @param type The image type to retrieve
   * @param numScene The scene number to retrieve
   * @param width The image width requested, will be modified with correct ratio if needed
   * @param heigh The image height requested, will be modified with correct ratio if needed
   * @param image A pointer to a CamImage structure. If successful, DO NO FORGET TO FREE "buffer" !!!
   * @param acqResXMmPerPx The resolution in width in mm per pixel.
   * @param acqResYMmPerPx The resolution in height in mm per pixel.
   * @return IQ_OK When successful
   */
   IQDLL IqError IqGetJobImageResized( IQREMOTEHANDLE handle,
                                int32_t jobId,
                                JobImageType type,
                                uint32_t numScene,
                                uint32_t width,
                                uint32_t height,
                                CamImage * image,
                                float * acqResXMmPerPx,
                                float * acqResYMmPerPx
                                );


   /**
   * Retrieve a job image, given the JobId in the database or current job.
   * @param jobId The jobId in the IQ database OR -1 for current job loaded in server
   * @param type The image type to retrieve
   * @param numScene The scene number to retrieve
   * @param width The image width requested, will be modified with correct ratio if needed
   * @param heigh The image height requested, will be modified with correct ratio if needed
   * @param image A pointer to a CamImage structure. If successful, DO NO FORGET TO FREE "buffer" !!!
   * @param acqResXMmPerPx The resolution in width in mm per pixel.
   * @param acqResYMmPerPx The resolution in height in mm per pixel.
   * @return IQ_OK When successful
   */
   IQDLL IqError IqGetJobImageResizedROI( IQREMOTEHANDLE handle,
                                       int32_t jobId,
                                       JobImageType type,
                                       uint32_t numScene,
                                       uint32_t width,
                                       uint32_t height,
                                       CamImage * image,
                                       float * acqResXMmPerPx,
                                       float * acqResYMmPerPx,
                                       float * middleX,
                                       float * middleY,
                                       float * zoom
                                       );

   /**
   * Retrieve a current job image on the displayROI
   * @param type The image type to retrieve
   * @param numScene The scene number to retrieve
   * @param image A pointer to a CamImage structure. If successful, DO NO FORGET TO FREE "buffer" !!!
   * @return IQ_OK When successful
   */
   IQDLL IqError IqGetJobImageRoi( IQREMOTEHANDLE handle,
                                   JobImageType type, 
                                   uint32_t numScene, 
                                   CamImage * image);

   /**
   * Return the current IQServer state
   * @param state The current IQServer state
   * @return IQ_OK when successful
   */
   IQDLL IqError IqGetState( IQREMOTEHANDLE handle, StatesEnum * state );

   /**
   * Accept some defect from a ProdResult
   */
   IQDLL IqError IqAcceptDefect( IQREMOTEHANDLE handle, TrtResultType defectType, uint32_t softwareCntr, uint32_t defectId );

   /**
   * Accept all defects from a ProdResult
   */
   IQDLL IqError IqAcceptFormatDefects(IQREMOTEHANDLE handle, uint32_t softwareCntr);

   /**
   * Reset the previous accepted defects from current job
   */
   IQDLL IqError IqResetAcceptedDefects( IQREMOTEHANDLE handle );

   /**
   * Return the last error occured in the state machine (ASYNCHRONOUS errors)
   * @return IQ_OK when it's OK :P
   */
   IQDLL IqError IqGetSMLastError( IQREMOTEHANDLE handle );

   /**
   * Start to record an IDB on the current job
   */
   IQDLL IqError IqStartrecordIdb( IQREMOTEHANDLE handle );

   /**
   * Start to record an IDB on the current job
   * @param nbOfFormat Number of format to record, -1 unlimited
   */
   IQDLL IqError IqStartrecordIdbSpecific( IQREMOTEHANDLE handle, int32_t nbOfFormat );

   /**
   * Start to record an IDB on the current job recording only defects in inspection
   */
   IQDLL IqError IqStartrecordIdbProdDefectsOnly( IQREMOTEHANDLE handle );

   /**
   * Stop record
   */
   IQDLL IqError IqStoprecordIdb( IQREMOTEHANDLE handle );

   /**
   * Get the number of camera configured for the system
   */
   IQDLL IqError IqGetNbOfCamera( IQREMOTEHANDLE handle, uint32_t * nbOfCamera );

   /**
   * Shift the Top acquisition from an amount of pixels
   * @param shiftInPixels Amount of pixels to shift
   * @param bufferHeight Height of the image buffer corresponding to the shiftInPixels argument
   * @param effectiveFormatSoftwareCntr The effective format software counter
   */
   IQDLL IqError IqShiftTopAcq( IQREMOTEHANDLE handle, int32_t shiftInPixels, int32_t bufferHeight, float zoom, uint32_t * effectiveFormatSoftwareCntr );

   /**
   * Get the top acquisition shift from an amount of pixels to millimeters
   * @param shiftInPixels Amount of pixels to shift
   * @param bufferHeight Height of the image buffer corresponding to the shiftInPixels argument
   * @param shiftInMm The resulting shift in Mm from the top cellule
   */
   IQDLL IqError IqGetShiftToAcqEstimation( IQREMOTEHANDLE handle, int32_t shiftInPixels, int32_t bufferHeight, float zoom, int32_t * shiftInMm );

   /**
   * Retrieve the align range size. All coordinates values taken from IQDatabase or given in the API should be substracted with these values
   * when using the master image
   * @param alignRangeXPx The alignment range in pixels in X
   * @param alignRangeYPx The alignment range in pixels in Y
   */
   IQDLL IqError IqGetAlignmentRange( IQREMOTEHANDLE handle, int32_t * alignRangeXPx, int32_t * alignRangeYPx );

   /**
   * Retrieve the HMI margin size. All coordinates values taken from IQDatabase or given in the API should be substracted with these values
   * when using master cropped, mean, maxdiff ...
   * @param hmiMarginXPx The HMI margin range in pixels in X
   * @param hmiMarginYPx The HMI margin range in pixels in Y
   */
   IQDLL IqError IqGetHMIMargin( IQREMOTEHANDLE handle, int32_t * hmiMarginXPx, int32_t * hmiMarginYPx );

   /**
   * Retrieve the crop Top Left (MASTER_CROPPED_IMAGE) in the MASTER_IMAGE referential without margin removal. 
   * Be careful to remove the HMI margin size before writing the computed coordinates in DB
   * @param cropXPx The crop left in pixels in X
   * @param cropYPx The crop top in pixels in Y
   */
   IQDLL IqError IqGetCropTopLeft( IQREMOTEHANDLE handle, int32_t * cropXPx, int32_t * cropYPx );

   /**
   * Retrieve the last box shift in RM on the iQ500 belt
   * @param shiftXPx The shift X on the iQ500 belt
   * @param shiftYPx The shift Y on the iQ500 belt
   */
   IQDLL IqError IqGetRMBoxShift( IQREMOTEHANDLE handle, int32_t * shiftXPx, int32_t * shiftYPx );

   /**
   * Retrieve the current machine speed.
   * @param machineSpeedMetersPerMinute The machine speed in meters per minute
   * @param alignRangeYPx The alignment range in pixels in Y
   */
   IQDLL IqError IqGetMachineSpeed( IQREMOTEHANDLE handle, float * machineSpeedMetersPerMinute );

   /**
   * Retrieve the lines that cannot be twice overlapped by alignment models
   * @param linesY Array of lines not to be overlapped (WARNING !! Use IqFreeBuffer to release memory !!)
   * @param nbOfLinesY Number of lines in the linesY array
   */
   IQDLL IqError IqGetModelOverlapLines( IQREMOTEHANDLE handle, uint32_t ** linesY, uint32_t * nbOfLinesY );

   /**
   * Retrieve the acquisition resolution in millimeter per pixel. All coordinates values taken from IQDatabase or given in the API should be multiplied by the ratio 
   * (Y'=acqResYMmPerPx/acqResXMmPerPx*Y)!!!
   * 
   * @param acqResXMmPerPx Acquisition resolution retrieved in mm per pixel in width
   * @param acqResYMmPerPx Acquisition resolution retrieved in mm per pixel in height
   */
   IQDLL IqError IqGetAcquisitionResolutionMmPerPixel( IQREMOTEHANDLE handle, float * acqResXMmPerPx, float * acqResYMmPerPx );

   /**
   * Return the maximum IQServer state the current job can reach depending on the job data
   * @param state The current IQServer state
   * @return IQ_OK when successful
   */
   IQDLL IqError IqGetJobMaximumState( IQREMOTEHANDLE handle, StatesEnum * state );

   /**
   * Retrieve the machine type used for the iQ500 
   * @param type The machine type used in iQ500 
   * @return IQ_OK when successful
   */
   IQDLL IqError IqGetMachineType( IQREMOTEHANDLE handle, MachineType * type );

   /**
    * Get the job data path
    * @param filepath Get the job datapath. Use IqFreeBuffer to release the buffer
    * @return IQ_OK when successful
    */
   IQDLL IqError IqGetJobDataPath( IQREMOTEHANDLE handle, wchar_t ** filepath);

   /**
   * Add some defect scenario description for the defects generation.
   * This call can be called multiple times to add new scenarii
   * @param defectDesc The description for some defect generation to add
   * @return IQ_OK when successful
   */
   IQDLL IqError IqAddGenDefect( IQREMOTEHANDLE handle, DefectGenDescription * defectDesc );

   /**
   * Clear/Reset the scenarii description for defects generation
   * @return IQ_OK when successful
   */
   IQDLL IqError IqResetGenDefect( IQREMOTEHANDLE handle );

   /**
    * Get the current job statistics 
    * @param jobStat A structure containing the job statistics
    * @return IQ_OK when successful
    */
   IQDLL IqError IqGetJobStat( IQREMOTEHANDLE handle, JobStat * jobStat);

   /**
   * Get the current job ID running
   * @param jobId The job ID running in the process
   * @return IQ_OK when successful
   */
   IQDLL IqError IqGetJobId( IQREMOTEHANDLE handle, int32_t * jobId );

   /**
   * Update the current operation ID running. Will reset format counters !
   * @param ipId The operation ID to change into vision
   * @return IQ_OK when successful
   */
   IQDLL IqError IqUpdateOperationId( IQREMOTEHANDLE handle, int32_t opId );

   /**
   * Autogenerate cut zones in PCR and FLEXO.
   * These zones can be generated after having defined the master image with the correct contour.
   * They are stored in the job database.
   * @return IQ_OK when successful
   */
   IQDLL IqError IqRefreshAutoCutZones( IQREMOTEHANDLE handle );

   /**
   * Autogenerate txt zones 
   * These zones can be generated after having defined the master image with the correct contour.
   * They are stored in the job database.
   * @return IQ_OK when successful
   */
   IQDLL IqError IqRefreshAutoTXTZones( IQREMOTEHANDLE handle );

   /**
   * Get the application server version
   * @param major The major application version
   * @param minor The minor application version
   * @param branch The branch application version
   * @param commit The commit application version
   * @return IQ_OK when successful
   */
   IQDLL IqError IqGetIqServerVersion( IQREMOTEHANDLE handle, uint32_t * major, uint32_t * minor, uint32_t * branch, uint32_t * commit );

   /**
   * Get the coreremote DLL version
   * @param major The major DLL version
   * @param minor The minor DLL version
   * @param branch The branch DLL version
   * @param commit The commit DLL version
   * @return IQ_OK when successful
   */
   IQDLL IqError IqGetIqRemoteVersion( uint32_t * major, uint32_t * minor, uint32_t * branch, uint32_t * commit );

   /**
   * Get a unique identifier for the application server. It is generated from the MIL dongle.
   * Some challenge response is generated to avoid possible coreremote DLL reverse engineering.
   * It can be automatically check with IqCheckChallenge which should return IQ_OK when successful.
   * Do not forget to free data (**)
   * @return IQ_OK when successful
   */
   IQDLL IqError IqGetGUID( IQREMOTEHANDLE handle, char ** GUID, uint32_t * lengthOfGUID, uint32_t challenge, uint32_t * challengeServer, char ** challengeAnswer, uint32_t * lengthOfChallenge );

   /**
   * Get a unique identifier for the application server. It is generated from the MIL dongle.
   * Some challenge response is generated to avoid possible coreremote DLL reverse engineering.
   * It can be automatically check with IqCheckChallenge which should return IQ_OK when successful.
   * @return IQ_OK when successful
   */
   IQDLL IqError IqCheckChallenge( uint32_t challenge, uint32_t challengeServer, char * challengeAnswer, uint32_t lengthOfChallenge );

   
   IQDLL IqError IqGetBoxesPerHour(IQREMOTEHANDLE handle, uint32_t * boxesPerHour);

   IQDLL IqError IqReplayConformityRefences(IQREMOTEHANDLE handle);

   IQDLL IqError IqGetProdLearningType(IQREMOTEHANDLE handle, ProdLearningType * type);

   /**
   * Get the current system errors
   * Do not forget to free data (**)
   * @param nbOfErrors The number of errors in the errorCodes array
   * @param errorCodes An array with the different error codes
   * @param errorCodes An array with the different error statuses
   * @param errorCodes An array with the different error values
   * @return IQ_OK when successful
   */
   IQDLL IqError IqGetCurrentErrors( IQREMOTEHANDLE handle, uint32_t * nbOfErrors, int32_t ** errorCodes, bool ** statuses, int32_t ** values );
   
   IQDLL IqError IqGetModelsMaxSize( IQREMOTEHANDLE handle, int32_t * sizeX, int32_t * sizeY );

   IQDLL IqError IqGetGIWeb( IQREMOTEHANDLE handle, uint32_t * current, uint32_t * mean, uint32_t * min, uint32_t * max );

   IQDLL IqError IqComputeRealDisplayRoi(
      uint32_t imageWidth,
      uint32_t imageHeight,
      float displayRoiTop,
      float displayRoiLeft,
      uint32_t displayRoiWidth,
      uint32_t displayRoiHeight,
      float zoom,
      uint32_t * realX,
      uint32_t * realY,
      uint32_t * realWidth,
      uint32_t * realHeight
      );

   IQDLL IqError IqDropResultSettings(IQREMOTEHANDLE handle, uint32_t dropResultDelayAmount, float dropResultFps);

   IQDLL IqError IqTestSendError( IQREMOTEHANDLE handle, int32_t errorCode, int32_t status, int32_t value );

   IQDLL IqError IqGetIdbInfos(IQREMOTEHANDLE handle, uint32_t* idbSize, int32_t* index, wchar_t** currentFileName);

   IQDLL IqError IqSetFPSIDB( IQREMOTEHANDLE handle, float fps );

   IQDLL IqError IqSetIndexIDB(IQREMOTEHANDLE handle, int32_t index);

   IQDLL IqError IqSetIdbReadingPath(IQREMOTEHANDLE handle, const wchar_t * idbPath);

   IQDLL IqError IqStepIDB( IQREMOTEHANDLE handle);

   IQDLL IqError IqGetRegulationStatus(IQREMOTEHANDLE handle, int32_t* regulationStatus);


   /**
   * Free a format result received in the callback. It's a mandatory call
   * @param result The data to free
   * @param type The data result type
   */
   IQDLL void IqFreeResult( void * result, DataResultType type );

   /**
   * Free wrapper for C++ buffer allocated by this DLL ( for example IqGetJobImage )
   * @param buffer The buffer to free
   */
   IQDLL void IqFreeBuffer( void * buffer );

   /**
   * Return the box counters just after the input cell.
   *        The goal is to replace the RED light, which is present on the OPTO22 of the top box cell,
   *        by a software counter.
   *        This is used for debugging any acquisition problem onto the machine.
   * @param inputCellCounter Pointer to a buffer where to store the reading of the input cell's counter
   * @param inputCellCounterFiltered Pointer to a buffer where to store the reading of the input cell's counter (after FPGA filtering)
   */
   IQDLL IqError IqGetTopBoxCountersFromInputCell(IQREMOTEHANDLE handle, uint16_t* inputCellCounter, uint16_t* inputCellCounterFiltered);

   /**
    **************************************************************
    * IQ DIGITAL FUNCTIONS
    **************************************************************
    */
    /**
     * Returns stitching patterns to be printed for calibration.
     * They are returned as TIFF buffers. The TIFF buffers are independent from color configuration on color bars
     * @param handle Handle to the established connection
     * @param colorBarIdForCalib Color bar identifier to be calibrated (0-based indexing)
     * @param colorBarIdForC2CReference Color bar identifier to be the reference for the color to color registering (0-based indexing)
     * @param nbOfColorBar The number of color bars on the machine
     * @param nbOfPrintingHeadPerColorBar The number of printing head per color bar group
     * @param tifPatterns A preallocated array of buffers of size nbOfColorBar. If the error status is IQ_OK, the buffers will be loaded with TIF buffers for each color bar.
     * @return IQ_OK when successful
     *         IQ_ERROR_RPC when a RPC or network error happened
     *         IQ_ERROR when a generic error happened or parameters are invalid
     */
   IQDLL IqError IqDigGetStitchingPattern(IQREMOTEHANDLE handle, int32_t colorBarIdForCalib, int32_t colorBarIdForC2CReference, uint32_t nbOfColorBar, uint32_t nbOfPrintingHeadPerColorBar, IqBuffer * tifPatterns);

   /**
    * Put iQ in digital calibration state "STATE_DIG_CALIB", only reachable when the IQ state is "STATE_IDLE".
    * If the IQ state is "STATE_RM, STATE_PROD_LRN, STATE_PROD", launch IqUnloadJob to put IQ state to STATE_IDLE.
    * When digital calibration is completed, launch IqDigStopCalib to put IQ state back to STATE_IDLE.
    * Wait for the call to return IQ_OK before printing the calibration patterns.
    * By default when launching the calibration state, the IQ will wait to calibrate all color bars in the colorTypesArray sequence, from index 0 to colorTypesArrayLength - 1 (IqDigSetColorBarIdSequence can change this sequence).
    * By default each color bar should print ONE format with the pattern (IqDigSetNbOfPrintedFormats can increase this number).
    * @param handle Handle to the established connection
    * @param mode Calibration mode (Only DIGCALIBRATIONMODE_STITCHING available yet)
    * @param colorBarsArray An array of ink colors for each color bar. Data should have indexes matching the colorBarIds, e.g. if colorBarId 0 has C color, then C should have index 0 in colorTypesArray
    * @param colorBarsArrayLength The number of color bars on the machine and the colorTypesArray length
    * @return IQ_OK when successful
    *         IQ_ERROR_RPC when a RPC or network error happened
    *         IQ_ERROR_STATE when the IQ is not in STATE_IDLE state
    *         IQ_ERROR when a generic error happened or parameters are invalid
    */
   IQDLL IqError IqDigStartCalib(IQREMOTEHANDLE handle, DigCalibrationMode mode, const DigCalibrationColorType * colorBarsArray, uint32_t colorBarsArrayLength);

   /**
    * Stop calibration state and put IQ state to STATE_IDLE.
    * @param handle Handle to the established connection
    * @return IQ_OK when successful
    *         IQ_ERROR_RPC when a RPC or network error happened
    *         IQ_ERROR_STATE when the IQ is not in STATE_DIG_CALIB state
    *         IQ_ERROR when a generic error happened or parameters are invalid
    */
   IQDLL IqError IqDigStopCalib(IQREMOTEHANDLE handle);

   /**
    * Set the sequence of color bar IDs to calibrate.
    * This function permits to modify the calibration sequence of color bars (i.e. the order or a subset).
    * It should be called only when IQ is in STATE_DIG_CALIB
    * @param handle Handle to the established connection
    * @param colorBarIdSequenceArray A sequence of IDs referring to the offset of colorTypesArray of the IqDigStartCalib call
    * @param colorBarIdSequenceArrayLength The colorBarIdSequenceArray length
    * @return IQ_OK when successful
    *         IQ_ERROR_RPC when a RPC or network error happened
    *         IQ_ERROR_STATE when the IQ is not in STATE_DIG_CALIB state
    *         IQ_ERROR when a generic error happened or parameters are invalid
    */
   IQDLL IqError IqDigSetColorBarIdSequence(IQREMOTEHANDLE handle, const int32_t * colorBarIdSequenceArray, uint32_t colorBarIdSequenceArrayLength);

   /**
    * Set the number of printed formats per color bar during calibration.
    * It permit to increase the number of printed formats/pattern for each color bar in the calibration sequence.
    * Increasing this number mainly permits to average mechanical/optical... errors and will provide a more precise calibration result. 
    * It should be called only when IQ is in STATE_DIG_CALIB
    * @param handle Handle to the established connection
    * @param nbOfFormats The number of printed formats for each color bar in the calibration sequence.
    * @return IQ_OK when successful
    *         IQ_ERROR_RPC when a RPC or network error happened
    *         IQ_ERROR_STATE when the IQ is not in STATE_DIG_CALIB state
    *         IQ_ERROR when a generic error happened or parameters are invalid
    */
   IQDLL IqError IqDigSetNbOfPrintedFormats(IQREMOTEHANDLE handle, uint32_t nbOfFormats);

   /**
    * Get the number of recommended printed formats per color bar during calibration.
    * 
    * It should be called only when IQ is in STATE_DIG_CALIB
    * @param handle Handle to the established connection
    * @param nbOfFormats The returned number of printed formats for each color bar in the calibration sequence.
    * @return IQ_OK when successful
    *         IQ_ERROR_RPC when a RPC or network error happened
    *         IQ_ERROR_STATE when the IQ is not in STATE_DIG_CALIB state
    *         IQ_ERROR when a generic error happened or parameters are invalid
    */
   IQDLL IqError IqDigGetRecommendedNbOfPrintedFormats(IQREMOTEHANDLE handle, uint32_t * nbOfFormats);

   /**
   * Resize the height of the image without any allocation overhead
   *
   * @param handle Handle to the established connection
   * @param newSizeYPx The new image height in px
   * @param formatCounter depends ont atOrFor.
   * @param atOrIn: put value 0 for At, meaning "at some" Hardware counter, reshape will occur. Put value 1 for "in N format", process reshape
   * @return IQ_OK when successful
   *         IQ_ERROR_RPC when a RPC or network error happened
   *         IQ_ERROR when a generic error happened or parameters are invalid
   */
   IQDLL IqError IqReshape(IQREMOTEHANDLE handle, uint32_t newSizeYPx, uint64_t formatCounter, uint32_t atOrIn);

   /**
   * Preload a job in iQ. It will take the print engine TIF files for the job,
   * transforms them and create the job in the iQ. This is a blocking call
   * @param handle Handle to the established connection
   * @param jobFileIndexPath Filepath to the XML index file from the root shared folder
   * @param jobId The unique identifier for the preloaded job, created in iQ database
   * @return IQ_OK when successful
   *          IQ_ERROR_RPC when a RPC or network error happened
   *          IQ_ERROR when a generic error happened or parameters are invalid
   */
   IQDLL IqError IqDigPEPreloadJob(IQREMOTEHANDLE handle, wchar_t * jobFileIndexPath, int32_t * jobId);

   /**
   * Delete a job. This delete is partial and only remove data used for job processing.
   * Job statistics and defects for previous productions are still stored.
   * iQ is still able to generate reports. This is a blocking call
   * @param handle Handle to the established connection
   * @param jobId The unique identifier for the preloaded job, created in iQ database
   * @return IQ_OK when successful
   *          IQ_ERROR_RPC when a RPC or network error happened
   *          IQ_ERROR when a generic error happened or parameters are invalid
   */
   IQDLL IqError IqDigPEDeleteJob(IQREMOTEHANDLE handle, int32_t jobId);

   /**
    * \brief 
    * \param handle Handle to the established connection
    * \param jobs
    * \param nbOfJobs 
    * \return 
    */
   IqError IqDigPELoadJobBatch(IQREMOTEHANDLE handle, PEJob * jobs, uint32_t nbOfJobs);   
   /**
    * \brief It is REQUIRED to free the "jobs" paraters to avoid memory leaks
    * \param handle Handle to the established connection
    * \param jobs
    * \param nbOfJobs 
    * \return 
    */
   IqError IqDigPEGetBatchedJobs(IQREMOTEHANDLE handle, PEJob ** jobs, uint32_t * nbOfJobs);
   /**
    * \brief 
    * \param handle Handle to the established connection
    * \param jobId
    * \return 
    */
   IqError IqDigPECurrentRunningBatchedJob(IQREMOTEHANDLE handle, PEJob * jobId);
   /**
    * \brief 
    * \param handle Handle to the established connection
    * \param jobId
    * \return 
    */
   IqError IqDigPERemoveBatchedJob(IQREMOTEHANDLE handle, int32_t * jobId);
   /**
    * \brief 
    * \param handle Handle to the established connection
    * \return 
    */
   IqError IqDigPEResetAllBatchedJobs(IQREMOTEHANDLE handle);

#ifdef __cplusplus
}
#endif

#endif
