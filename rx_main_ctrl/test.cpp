#include "rx_def.h"
#include "mlpi_load.h"
#ifdef linux
#include <mlpiGlobal.h>
#include <mlpiApiLib.h>
#include <memory.h>
#endif

int TEST(void)
{
//	printf("sizeof(char)=%d\n", (int)sizeof(char));
//	printf("sizeof(USHORT)=%d\n", (int)sizeof(USHORT));
//	printf("sizeof(ULONG)=%d\n", (int)sizeof(ULONG));

	MLPIRESULT result;
//	MlpiVersion versionInfo;
//	result = mlpiApiGetClientCoreVersion(&versionInfo);
	MLPIHANDLE	_Connection = MLPI_INVALIDHANDLE;
	result = mlpiApiConnect(L"192.168.200.3", &_Connection);
	mlpiApiDisconnect(&_Connection);
	return 0;
}