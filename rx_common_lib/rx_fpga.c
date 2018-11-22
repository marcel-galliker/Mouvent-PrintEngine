// ****************************************************************************
//
//	rx_fpga.c
//
// ****************************************************************************
//
//	Copyright 2016 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#ifdef soc

//--- includes ---------------------------------------------------------------
#include <sys/mman.h>
#include <errno.h>

#include "rx_common.h"
#include "rx_def.h"
#include "rx_error.h"
#include "rx_trace.h"
#include "rx_fpga.h"



//---  statics ---------------------------------------------------------------
static SVersion	_LinuxVersion={0};

//--- prototypes -------------------------------------------------------------
static void _linux_version(void);
static void _check_offset(void* base, void* var, UINT32 expected);

//--- rx_fpga_init --------------------------------
void rx_fpga_init(void)
{
	_linux_version();		
}

//--- _linux_version ----------------------------------------------
static void _linux_version(void)
{
	FILE *file;
	char str[1024];
	
	file = fopen("/proc/version", "rb");
	if (file)
	{
		fgets(str, sizeof(str), file);
		sscanf(str, "Linux version %d.%d.%d-%d", &_LinuxVersion.major, &_LinuxVersion.minor, &_LinuxVersion.revision, &_LinuxVersion.build);
		fclose(file);
	}
}

//--- rx_fpga_load -----------------------------------
int rx_fpga_load(char *path_rbf)
{
	TrPrintfL(TRUE, "Loading FPGA >>%s<<", path_rbf);
	
	if (_LinuxVersion.major==0) _linux_version();
	
	#ifdef soc
		if (!rx_file_exists(path_rbf)) 
		{
			Error(ERR_CONT, 0, "Could not find >>%s<<", path_rbf);
			return REPLY_ERROR;
		}

		char command[MAX_PATH];
		if (_LinuxVersion.major==3)	sprintf(command, "su -c '/opt/radex/load-fpga %s'", path_rbf);
		else						sprintf(command, "su -c '/etc/init.d/S50devicetree_overlay restart'");
		system(command);				
		if (rx_fpga_running())	
		{
			TrPrintf(TRUE, "FPGA loaded");
			return REPLY_OK;
		}

	#endif
	Error(ERR_CONT, 0, "file >>%s<< found but not loaded", path_rbf);
	return REPLY_ERROR;
}

//--- rx_fpga_running ----------------------------------------------------
int rx_fpga_running(void)
{
	FILE *file;
	char str[128];
	int res=FALSE;

	if (_LinuxVersion.major==0) _linux_version();

	if (_LinuxVersion.major==3)
	{
		file = fopen("/sys/class/fpga/fpga0/status", "rb");
		if (file)
		{
			fgets(str, sizeof(str), file);
			printf("FPGA State: >>%s<<\n", str);
			if (!strcmp(str, "user mode\n")) res=TRUE;
			fclose(file);
		}		
	}
	else
	{
		//--- FPGA -----------
		file = fopen("/sys/class/fpga_manager/fpga0/state", "rb");
		if (file)
		{
			fgets(str, sizeof(str), file);
			printf("FPGA State: >>%s<<\n", str);
			if (!strcmp(str, "operating\n")) res=TRUE;
			fclose(file);
		}
	
		//--- Bridge 0 --------------------
		file = fopen("/sys/class/fpga_bridge/br0/state", "rb");
		if (file)
		{
			fgets(str, sizeof(str), file);
			printf("Bridge-0 State: >>%s<<\n", str);
			if (strcmp(str, "enabled\n")) res=FALSE;
			fclose(file);
		}

		//--- Bridge 1 --------------------
		file = fopen("/sys/class/fpga_bridge/br1/state", "rb");
		if (file)
		{
			fgets(str, sizeof(str), file);
			printf("Bridge-1 State: >>%s<<\n", str);
			if (strcmp(str, "enabled\n")) res=FALSE;
			fclose(file);
		}		
	}
	return res;
}

//--- rx_fpga_map_page ---------------------
void* rx_fpga_map_page(int memId, UINT64 addr, UINT32 size, UINT32 size_expected)
{
	if (size!=size_expected)
	{
		Error(ERR_ABORT, 0, "Map Memory size mismatch, size=0x%04x, expected=0x%04x", size, size_expected);
		int i=0;
		i=1/i;
		return NULL;
	}

#ifdef linux
	void *a;
	UINT32 pagesize = sysconf(_SC_PAGE_SIZE);
    UINT32 pagemask = (pagesize - 1);

	size = (size+pagesize-1)/pagesize*pagesize;

	if (addr & pagemask) {Error(ERR_CONT, 0, "_map_page address not on a page address"); return NULL;}

    a = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, memId, addr);
    if(a ==  MAP_FAILED) {Error(ERR_CONT, 0, "_map_page could not map. errno=%d", errno); return NULL;}

    //    addr = ((unsigned long *)m->base) + (offset & pagemask); ???
    return a;
#else
	return NULL;
#endif
}

//--- _check_offset -------------------------------------------------
static void _check_offset(void* base, void* var, UINT32 expected)
{
	UINT32 offset;
	offset = (UINT32)var - (UINT32)base;
	if (offset!=expected)
	{
		Error(ERR_ABORT, 0, "Struct offset mismatch, size=0x%04x, expected=0x%04x", offset, expected);
		int i=0;
		i = 1/i;		
	}
}

#endif // soc
