// ****************************************************************************
//
//	rx_embrip_test.c
//
// ****************************************************************************
//
//	Copyright 2015 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#include "rx_common.h"
#include "rx_error.h"
#include "rx_file.h"
#include "rx_mem.h"
#include "rx_threads.h"
#include "rx_trace.h"
#include "version.h"
#include "rx_tif.h"
#include "rx_embrip.h"
#include "rx_def.h"

SColorSplitCfg	RX_Color[MAX_COLORS];

static void args_init(int argc, char** argv);
static int _do_rip_file(char * printEnv, char * inputpath, int rotation, int columns, int colDist);

//--- external variables -------------------
char * arg_input = NULL;
char * arg_printenv = NULL;
int arg_rotation = 0;
int arg_columns = 1;
int arg_colDist = 0;


//--- args_init -----------------------------------
static void args_init(int argc, char** argv)
{
	int i;
	for (i = 1; i<argc; i++)
	{
		if (!strcmp(argv[i], "-i")) {
			if (i+1<argc) 
				arg_input = argv[++i];
		}
		else if (!strcmp(argv[i], "-p")) {
			if (i + 1<argc)
				arg_printenv = argv[++i];
		}
		else if (!strcmp(argv[i], "-r")) {
			if (i + 1<argc)
				arg_rotation = strtol(argv[++i], NULL, 10);
		}
		else if (!strcmp(argv[i], "-c")) {
			if (i + 1<argc)
				arg_columns = strtol(argv[++i], NULL, 10);
		}
		else if (!strcmp(argv[i], "-d")) {
			if (i + 1<argc)
				arg_colDist = strtol(argv[++i], NULL, 10);
		}
		else { printf("argument >>%s<< not known\n", argv[i]); exit(1); }
	}

	if (!arg_input)
		{	printf("argument input path: -i  not known\n");  exit(1);	}
	if (!arg_printenv)
		{	printf("argument printenv: -p  not known\n"); exit(1);	}
	if ((arg_rotation != 0) && (arg_rotation != 90) && (arg_rotation != 180) && (arg_rotation != 270))
		{	printf("argument rotation: -r %d wrong, must be 0, 90, 180 or 270\n", arg_rotation); exit(1);	}
	if ((arg_columns < 1) || (arg_columns >32))
		{	printf("argument columns: -c %d wrong, must be >0 and <33\n", arg_columns); exit(1);	}
	if ((arg_colDist < 0) || (arg_colDist > 1000*1000))
		{	printf("argument colDist: -d %d wrong, must be >=0 and < 1000000\n", arg_colDist); exit(1);	}

}


static int _do_rip_file(char * printEnv, char * inputpath, int rotation, int columns, int colDist)
{
	int		ret = REPLY_OK;
	char jobName[MAX_PATH];
	char filepath[MAX_PATH];
	char ext[MAX_PATH];
	char *pchar;
	char search[MAX_PATH];

	split_path(inputpath, NULL, jobName, ext);
	pchar = ext;
	while (*pchar)
		*pchar++ = tolower(*pchar);

	TrPrintfL(TRUE, "_do_rip_file >>%s %s<<", inputpath, printEnv);
	rx_mkdir(PATH_RIPPED_DATA);
	sprintf(filepath, "%s%s", PATH_RIPPED_DATA, printEnv);
	rx_mkdir(filepath);
	sprintf(filepath, "%s/%s", filepath, jobName);
	rx_mkdir(filepath);
	sprintf(search, "%s/*.*", filepath);
	rx_remove_old_files(search, 0); // clean directory

	if (!strncmp(ext, ".tif", 4))
		ret = rx_tiff_rip(printEnv, inputpath, rotation, columns, colDist, filepath, jobName);
	else if (!strncmp(ext, ".pdf", 4) || !strncmp(ext, ".ps", 3) || !strncmp(ext, ".eps", 4))
		ret = rx_pdf_rip(printEnv, inputpath, rotation, columns, colDist, filepath, jobName);
	else
		TrPrintfL(TRUE, "_do_rip_file extension : %s not supported", ext);

	return ret;
}


//--- main --------------------------------------
int main(int argc, char* argv[])
{
	int running = TRUE;
	
	rx_process_name(argv[0]);
	args_init(argc, argv);

	err_init(FALSE, 100);
	Trace_init(argv[0]);

	TrPrintfL(TRUE, "rx_embrip_test V %s", version);

	SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);


	//Init
	memset(RX_Color, 0, sizeof (SColorSplitCfg) * MAX_COLORS);
	strcpy(RX_Color[0].color.name, RX_ColorName[1].name); // Cyan
	strcpy(RX_Color[1].color.name, RX_ColorName[2].name); // Magenta
	strcpy(RX_Color[2].color.name, RX_ColorName[3].name); // Yellow
	strcpy(RX_Color[3].color.name, RX_ColorName[0].name); // Black
	RX_Color[0].color.colorCode= RX_ColorName[1].code;
	RX_Color[1].color.colorCode= RX_ColorName[2].code;
	RX_Color[2].color.colorCode= RX_ColorName[3].code;
	RX_Color[3].color.colorCode= RX_ColorName[0].code;

	RX_Color[0].lastLine = RX_Color[1].lastLine = RX_Color[2].lastLine = RX_Color[3].lastLine = 1200 * 36;

	_do_rip_file(arg_printenv, arg_input, arg_rotation, arg_columns, arg_colDist);

//	rx_sleep(10*1000);
	return 0;

}

