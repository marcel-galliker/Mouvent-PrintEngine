// ****************************************************************************
//
//	rx_rip_ctrl.c
//
// ****************************************************************************
//
//	Copyright 2015 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#include "rx_common.h"
#include "rx_error.h"
#include "rx_threads.h"
#include "rx_trace.h"
#include "version.h"
#include "rip_srv.h"

//--- startup -----------------------------------
void startup(int argc, char* argv[])
{
	int cnt;
	cnt = rx_process_running_cnt(argv[0], NULL);
	TrPrintf(1, "ProcessCnt >>%s<<=%d", argv[0], cnt);

	if (argc==2 && !strcmp(argv[1], "-debug"))
	{
		TrPrintfL(1, "debug version");
		while(1)
		{
			cnt = rx_process_running_cnt(argv[0], NULL);
			if (cnt<2) break;
			TrPrintf(1, "Kill >>%s<<", argv[0]);
			rx_process_kill(argv[0], NULL);
		}
	}
	else
	{
		TrPrintfL(1, "background version");
		if (cnt>1)
		{
			TrPrintfL(1, "Already running, cnt=%d", cnt);
			return;
		}
		rx_run_in_backgrund();
	}
}

//--- main --------------------------------------
int main(int argc, char* argv[])
{
	int running = TRUE;
	
	rx_process_name(argv[0]);
	startup(argc, argv);

	err_init(FALSE, 100);
	Trace_init(argv[0]);

	TrPrintfL(TRUE, "rx_rip_ctrl V %s", version);

	SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);

	rip_init();
	while (running)
	{
		rx_sleep(1000);
	}
	rip_end();
	return 0;
}

