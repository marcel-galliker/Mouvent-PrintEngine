
#include "stdafx.h"
#include "rx_common.h"
#include <mlpiGlobal.h>
#include <mlpiApiLib.h>
#include <mlpiSystemLib.h>
#include <mlpiLogicLib.h>
#include <mlpiParameterLib.h>


#ifdef _WIN64_asdkj

MLPI_API MLPIRESULT mlpiApiSetDefaultTimeout(const ULONG timeout){return MLPI_S_OK;}
MLPI_API MLPIRESULT mlpiApiConnect(const WCHAR16* connectionIdentifier, MLPIHANDLE* connection){return MLPI_S_OK;}
MLPI_API MLPIRESULT mlpiApiDisconnect(MLPIHANDLE *connection){return MLPI_S_OK;}

MLPI_API MLPIRESULT mlpiLogicLoadBootApplication(const MLPIHANDLE connection, const WCHAR16 *file, const WCHAR16 *path, WCHAR16 *application, const ULONG numElements){return MLPI_S_OK;}
MLPI_API MLPIRESULT mlpiLogicGetSymbolsOfApplication(const MLPIHANDLE connection, const WCHAR16 *application, MLPIHANDLE* node, WCHAR16 *symbols, const ULONG numElements){return MLPI_S_OK;}
MLPI_API MLPIRESULT mlpiLogicGetTypeOfSymbol(const MLPIHANDLE connection, const WCHAR16 *symbol, MlpiLogicType *type, MlpiLogicType *subtype){return MLPI_S_OK;}
MLPI_API MLPIRESULT mlpiLogicReadVariableBySymbolString(const MLPIHANDLE connection, const WCHAR16 *symbol, WCHAR16 *data, const ULONG numElements){return MLPI_S_OK;}
MLPI_API MLPIRESULT mlpiLogicReadVariableBySymbolFloat(const MLPIHANDLE connection, const WCHAR16 *symbol, FLOAT *data){return MLPI_S_OK;}
MLPI_API MLPIRESULT mlpiLogicGetNumberOfApplications(const MLPIHANDLE connection, ULONG* number){return MLPI_S_OK;}
MLPI_API MLPIRESULT mlpiLogicGetNameOfApplication(const MLPIHANDLE connection, const ULONG index, WCHAR16* application, const ULONG numElements){return MLPI_S_OK;}
MLPI_API MLPIRESULT mlpiLogicWriteVariableBySymbolString(const MLPIHANDLE connection, const WCHAR16 *symbol, const WCHAR16 *data){return MLPI_S_OK;}
MLPI_API MLPIRESULT mlpiLogicStartApplication(const MLPIHANDLE connection, const WCHAR16 *application){return MLPI_S_OK;}
MLPI_API MLPIRESULT mlpiLogicStopApplication(const MLPIHANDLE connection, const WCHAR16 *application){return MLPI_S_OK;}
MLPI_API MLPIRESULT mlpiLogicResetApplication(const MLPIHANDLE connection, const MlpiApplicationResetMode resetMode, const WCHAR16 *application){return MLPI_S_OK;}

MLPI_API MLPIRESULT mlpiSystemGetSpecialPath(const MLPIHANDLE connection, const MlpiSpecialPath pathIdentifier, WCHAR16 *path, const ULONG numElements){return MLPI_S_OK;}
MLPI_API MLPIRESULT mlpiSystemGetVersionInfo(const MLPIHANDLE connection, const WCHAR16 *component, WCHAR16 *version, const ULONG numElements){return MLPI_S_OK;}
MLPI_API MLPIRESULT mlpiSystemGetCurrentMode(const MLPIHANDLE connection, MlpiSystemMode *mode){return MLPI_S_OK;}
MLPI_API MLPIRESULT mlpiSystemSetTargetMode(const MLPIHANDLE connection, const MlpiSystemMode mode){return MLPI_S_OK;}
MLPI_API MLPIRESULT mlpiSystemGetTemperature(const MLPIHANDLE connection, FLOAT *temperature){return MLPI_S_OK;}
MLPI_API MLPIRESULT mlpiSystemGetTemperatureMax(const MLPIHANDLE connection, FLOAT *temperature){return MLPI_S_OK;}
MLPI_API MLPIRESULT mlpiSystemGetDateAndTimeUtc(const MLPIHANDLE connection, MlpiDateAndTime *dateTime){return MLPI_S_OK;}
MLPI_API MLPIRESULT mlpiSystemSetDateAndTimeUtc(const MLPIHANDLE connection, MlpiDateAndTime dateTime){return MLPI_S_OK;}
MLPI_API MLPIRESULT mlpiSystemGetSerialNumber(const MLPIHANDLE connection, WCHAR16 *serialNumber, const ULONG numElements){return MLPI_S_OK;}
MLPI_API MLPIRESULT mlpiSystemGetHardwareDetails(const MLPIHANDLE connection, WCHAR16 *hardwareDetails, const ULONG numElements){return MLPI_S_OK;}
MLPI_API MLPIRESULT mlpiSystemGetLocalBusConfiguration(const MLPIHANDLE connection, WCHAR16 *configuration, const ULONG numElements){return MLPI_S_OK;}
MLPI_API MLPIRESULT mlpiSystemGetFunctionModulBusConfiguration(const MLPIHANDLE connection, WCHAR16 *configuration, const ULONG numElements){return MLPI_S_OK;}
MLPI_API MLPIRESULT mlpiSystemGetFunctionModuleHardwareDetails(const MLPIHANDLE connection, const ULONG functionModuleNum, WCHAR16 *hardwareDetails, const ULONG numElements){return MLPI_S_OK;}
MLPI_API MLPIRESULT mlpiSystemClearError(const MLPIHANDLE connection){return MLPI_S_OK;}
MLPI_API MLPIRESULT mlpiSystemReboot(const MLPIHANDLE connection){return MLPI_S_OK;}
MLPI_API MLPIRESULT mlpiSystemCleanup(const MLPIHANDLE connection){return MLPI_S_OK;}
MLPI_API MLPIRESULT mlpiSystemGetDisplayedDiagnosis(const MLPIHANDLE connection, MlpiDiagnosis *diagnosis){return MLPI_S_OK;}
MLPI_API MLPIRESULT mlpiSystemGetNewestDiagnosisIndex(const MLPIHANDLE connection, ULONG *index){return MLPI_S_OK;}
MLPI_API MLPIRESULT mlpiSystemGetOldestDiagnosisIndex(const MLPIHANDLE connection, ULONG *index){return MLPI_S_OK;}
MLPI_API MLPIRESULT mlpiSystemGetDiagnosisLog(const MLPIHANDLE connection, const ULONG index, MlpiDiagnosis *diagnosisLog, const ULONG numElements, ULONG *numElementsRet){return MLPI_S_OK;}

MLPI_API MLPIRESULT mlpiParameterWriteDataString(const MLPIHANDLE connection, const ULLONG address, const ULLONG sidn, const WCHAR16 *data){return MLPI_S_OK;}
MLPI_API MLPIRESULT mlpiParameterReadName(const MLPIHANDLE connection, const ULLONG address, const ULLONG sidn, WCHAR16 *name, const ULONG numElements){return MLPI_S_OK;}
MLPI_API MLPIRESULT mlpiParameterReadDataString(const MLPIHANDLE connection, const ULLONG address, const ULLONG sidn, WCHAR16 *data, const ULONG numElements, ULONG *numElementsRet){return MLPI_S_OK;}

#endif




