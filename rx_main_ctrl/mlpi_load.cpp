#include "rx_def.h"
#include "mlpi_load.h"
#ifdef linux
#include <mlpiGlobal.h>
#include <mlpiApiLib.h>
#include <memory.h>
#endif

int mlpi_load(void)
{
#ifdef linux
	MlpiVersion versionInfo;
	memset(&versionInfo, 0, sizeof(versionInfo));
	MLPIRESULT result = mlpiApiGetClientCoreVersion(&versionInfo);
//	MLPIHANDLE	_Connection = MLPI_INVALIDHANDLE;
//	result = mlpiApiConnect(L"192.168.200.3", &_Connection);
//	MLPIHANDLE	_Connection = MLPI_INVALIDHANDLE;
//	result = mlpiApiConnect(L"192.168.200.3", &_Connection);
	TEST();
#endif
	return 0;
}
