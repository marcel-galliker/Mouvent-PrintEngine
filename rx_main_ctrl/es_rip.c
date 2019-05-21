// ****************************************************************************
//
//	es_rip.c	Ergosoft Rip Interface
//
// ****************************************************************************
//
//	Copyright 2014 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************
//
//
//
// Documentation for HTML messages: http://www.html-world.de/209/request-methoden/
//

// #include <io.h>
#include <wchar.h>
#include "rx_common.h"
#include "rx_def.h"
#include "rx_error.h"
#include "rx_file.h"
#include "rx_trace.h"
#include "rx_threads.h"
#include "tcp_ip.h"
#include "gui_svr.h"
#include "bmp.h"
#include "es_rip.h"

//--- Defines -----------------------------------------------------------------
#define ESRIP_PORT	8080

#define ESRIP_PATH_PRINTENV	"Data\\PrEnvFiles\\"
#define MAX_RIP_JOBS	10

//--- Externals ---------------------------------------------------------------



//--- Modlue Globals -----------------------------------------------------------------

static char _path_user[MAX_PATH];
static char _path_exe [MAX_PATH];
static char _path_jobQueue[MAX_PATH];

typedef struct
{
	char	name[MAX_PATH];
	int		pos;
} SRipJobInfo;

static SRipJobInfo	_rip_job[MAX_RIP_JOBS];
static UINT64		_rip_jq_time;

//--- Prototypes --------------------------------------------------------------
static int _start_processes(void);
static int _print_client_running(const char *printEnv);
static int _print_client_start(const char *printEnv);
static int _printEnv_exists(const char *printEnv);
// static void _get_rip_state(const char *filepath, char *ripState);
static void _rip_done	  (const char *jobName);


//--- esrip_init ----------------------------------------------------------------
void esrip_init(void)
{
	if (!*_path_user)
	{
		FILE *file;
		char line[MAX_PATH];
		int idx;

		file = fopen("C:\\Users\\Public\\Documents\\ES_HKLM.ini", "rb");
		if (file)
		{
			while (fgets(line, SIZEOF(line), file))
			{
				if (idx=str_start(line, "UserDir="))
				{
					strcpy(_path_user, &line[idx]);
					for (idx=(int)strlen(_path_user)-1; idx>=0 && _path_user[idx]<' '; idx--) _path_user[idx]=0;
					sprintf(&_path_user[idx+1], "\\");
				}
				else if (idx=str_start(line, "InstallDir="))
				{
					strcpy(_path_exe, &line[idx]);
					for (idx=(int)strlen(_path_exe)-1; idx>=0 && _path_exe[idx]<' '; idx--)
						_path_exe[idx]=0;
					sprintf(&_path_exe[idx+1], "\\");					
				}
			}
			fclose(file);

			if (*_path_user) sprintf(_path_jobQueue, "%sData\\JQServJobs\\$JqState.jqm", _path_user);
			else memset(_path_jobQueue, 0, sizeof(_path_jobQueue));

			memset(_rip_job, 0, sizeof(_rip_job));
		}
	}
}

//--- esrip_end ------------------------------------------------------------------
void esrip_end(void)
{
}

/*
//--- esrip_map_drive ------------------------------------
void esrip_map_drive(const char *path)
{	
	/*
	{
		_print_client_running("Radex 4C");
		_print_client_start("Radex 4C");
	//	esrip_rip_file("D:\\Develop\\Data\\GT-Print-Data\\Textile\\Scan-Test-01.tif", "Radex 4C", 0, 2, 200000);
	}
	*/

	/*
	#ifdef linux
	if (*path)
	{		
		// Giving access right in ubuntu:
		// > sudo visudo
		// >> add line "radex ALL = NOPASSWD: /bin/mount"
		// >> add line "radex ALL = NOPASSWD: /bin/mkdir"
		
		char command[256];
		char user[] = "Guest";
		char pwd[]  = "Guest";
		char mnt_path[]= "/mnt/ergosoft";
		int ret;

		if (access(mnt_path, 0))
		{
			sprintf(command, "sudo mkdir -m777 %s", mnt_path); 
			ret=system(command);
		}
		
		int uid = getuid();	// actual user ID
		int gid = getgid();	// actual group ID
		sprintf(command, "sudo mount -t cifs %s %s -o user=%s,password=%s,uid=%d,gid=%d", path, mnt_path, user, pwd, uid, gid); 
		ret=system(command);
	}
	#endif
	* /
}
*/


//--- esrip_local_path -------------------------------------
void esrip_local_path(const char *global, char *local)
{
	int pos;
	char *ch;
	if (pos=str_start(global, PATH_RIPPED_DATA_DRIVE))
		sprintf(local, "//%s/%s%s", RX_CTRL_MAIN, PATH_RIPPED_DATA_DIR,  &global[pos]);
	else strcpy(local, global);
	for (ch=local; *ch; ch++) if(*ch=='\\') *ch='/';
}

//--- _print_client_running ----------------------------------
static int _print_client_running(const char *printEnv)
{
	char path[MAX_PATH];
	wchar_t line[MAX_PATH];
	wchar_t checkStr[100];
	const char	*ch;
	FILE *file;
	int found=FALSE;
	int l;

	sprintf(path, "%s%s", PATH_TEMP, "PrintClients.txt");
	if (!rx_process_execute("wmic process where caption=\"ppdesp.exe\" get commandline", path, 0))
	{
		file = fopen(path, "rb");
		if (file)
		{
			l=0;
			checkStr[l++]=L'"';
			for (ch=printEnv; *ch;)
			{
				checkStr[l++]=*ch++;
			}
			checkStr[l++]=L'"';
			checkStr[l++]=0;

			fgetws(line, 2, file);
			while (!found && fgetws(line, SIZEOF(line), file))
			{
				if (wcsstr(line, checkStr)) found=TRUE;
			}
			fclose(file);
		}	
	}
	remove(path);
//	esrip_send_print_env(NULL);
//	l = _printEnv_exists("Radex 4C");
	return found;
}

//--- _print_client_start ----------------------------------
static int _print_client_start(const char *printEnv)
{
	if (!_print_client_running(printEnv))
	{
		char process[MAX_PATH];
		sprintf(process, "\"%s%s\" \"%s\"", _path_exe, "PpDesp.exe", printEnv);
		rx_process_start(process, NULL);
		/*
	//	char path[MAX_PATH];
	//	char params[MAX_PATH];
		sprintf(path, "\"%s%s\"", _path_exe, "PpDesp.exe");
		sprintf(params, "\"%s\"", printEnv);
		rx_process_start(path, params);
		*/
	}
	return REPLY_OK;
}

//--- _printEnv_exists ------------------------------
static int _printEnv_exists(const char *printEnv)
{
	char path[MAX_PATH];
	sprintf(path, "%s%s%s.PrEnv", _path_user, ESRIP_PATH_PRINTENV, printEnv);
	return !_access(path, 0);
}

//--- esrip_send_print_env -----------------------------
void esrip_send_print_env	(RX_SOCKET socket)
{
	SPrintEnvEvt	msg;
	char			path[MAX_PATH];
	SEARCH_Handle	search;
	int				len;
	int				isDir;

	if (socket)
	{
		TrPrintf(TRUE, "do_get_print_env");
		msg.hdr.msgLen = sizeof(msg.hdr);
		msg.hdr.msgId = REP_GET_PRINT_ENV;
		gui_send_msg(socket, &msg);
	}

	if (*_path_user)
	{
		msg.hdr.msgId = BEG_GET_PRINT_ENV;
		gui_send_msg(socket, &msg);

		msg.hdr.msgLen = sizeof(msg);
		msg.hdr.msgId = ITM_GET_PRINT_ENV;

		sprintf(path, "%s%s", _path_user, ESRIP_PATH_PRINTENV);
		search = rx_search_open(path, "*.PrEnv");
		while (rx_search_next(search, msg.printEnv, sizeof(msg.printEnv), NULL, NULL, &isDir))
		{
			if (!isDir)
			{
				len = (int)strlen(msg.printEnv);
				if (len>6) 
				{
					msg.printEnv[strlen(msg.printEnv)-6] = 0;
					gui_send_msg(socket, &msg);
				}
			}
		}
		rx_search_close(search);

		msg.hdr.msgLen = sizeof(msg.hdr);
		msg.hdr.msgId = END_GET_PRINT_ENV;
		gui_send_msg(socket, &msg);
	}
}

//--- _start_processes --------------------------
static int _start_processes(void)
{
	char path[MAX_PATH];
	int cnt, ret;

	cnt=rx_process_running_cnt("PpHotFld.exe", NULL);
	TrPrintfL(TRUE, "HotFolder cnt=%d", cnt);
	if (cnt==0)
	{
		sprintf(path, "\"%s%s\"", _path_exe, "PpHotFld.exe");
		ret=rx_process_start(path, NULL);
	}

	cnt=rx_process_running_cnt("PpRipSrv.exe", NULL);
	TrPrintfL(TRUE, "RIP-Server cnt=%d", cnt);
	if (cnt==0)
	{
		sprintf(path, "\"%s%s\"", _path_exe, "PpRipSrv.exe");
		TrPrintfL(TRUE, "starting >>%s<<", path);
		ret=rx_process_start(path, NULL);
	}

	return REPLY_OK;
}

//--- esrip_file_ripped ----------------------------------------------------------
int  esrip_file_ripped(const char *filepath, const char *printEnv, char *dataPath)
{
	char filename[MAX_PATH];
	char ext[16];

	if (!*filepath)
	{
		*dataPath = 0;
		return REPLY_OK;
	}

	split_path(filepath, NULL, filename, ext);

	//--- check the directory ------------------------
	{
		SEARCH_Handle hsearch;
		char			fname[100], str[100];
		char			sharename[100], *ch;
		UINT64			time, time0;
		UINT32			size, isdir;
		int pos;

		if (*printEnv) sprintf(dataPath, "%s%s/%s", PATH_RIPPED_DATA, printEnv, filename);
		else		   strcpy (dataPath, filepath);
		
		for (ch=dataPath; *ch; ch++) if(*ch=='\\') *ch='/';
		sprintf(sharename, "//%s/%s", RX_CTRL_MAIN, PATH_RIPPED_DATA_DIR);
		if (pos=str_start(dataPath, sharename))
		{	
			strcpy(str, &dataPath[pos]);
			sprintf(dataPath, "%s%s", PATH_RIPPED_DATA, str);
		}

		hsearch = rx_search_open(dataPath, "*");
		time0 = 0;
		while(rx_search_next(hsearch, fname, sizeof(fname), &time, &size, &isdir))
		{
			if (!isdir)
			{
				if (str_end(fname, ".xml") && str_end(fname, ".bmp"))
				{
					TrPrintfL(TRUE, "file >>%s<< time0=%d, time=%d, diff=%d", fname, (int)time0, (int)time, abs((int)time-(int)time0));
					if (!time0)	time0 = time;
					else if ((int)(time-time0)>20)
						return REPLY_TIME_ERROR;
				}
			}
		}
		rx_search_close(hsearch);
		if (time0) return REPLY_OK;
		return REPLY_NOT_RIPPED;
	}
}

//--- _find_in_buf ----------------------------------------
static int _find_in_buf(char *buf, int size, int start, char *find)
{
	int len=(int)strlen(find);
	int pos;

	size-=len;
	for (pos=start; pos<size; pos++)
	{
		if (!strncmp(&buf[pos], find, len)) return pos;
	}
	return -1;
}

//--- _get_str -----------------------
static char* _get_str(char *data, char *name, char *str)
{
	char *find;
	int i=0;
	
	*str=0;
	find = strstr(data, name);
	if (find) 
	{
		find+=strlen(name)+1;
		while (*find>=' ') str[i++]=*find++;
		str[i]=0;
	}
	return str;
}

//--- _get_val -----------------------
static int _get_val(char *data, char *name)
{
	char *str;
	int val;
	
	str = strstr(data, name);
	sscanf(&str[strlen(name)+1], "%X", &val);
	return val;
}

//--- read_job_info -----------------------------------------------------------------
/*
static void read_job_info(char *buffer, char *jobName, char *info)
{
	int  pos;
	int  id;
	int	 status;
	int	 permil, elapsed, remain;
	
	pos = _find_in_buf(buffer, 256, 10, "\\[");
	if (pos>0) id  = atoi(&buffer[pos+2]);
	else id=0;
	buffer+= pos;
	_get_str(buffer, "JobName", jobName);
	
	status				= _get_val(buffer, "Status");
	switch(status)
	{
	case RS_IDLE:		strcpy (info, ""); break;
	case RS_WAIT_RIP:	sprintf(info, "Waiting for RIP"); break;
	case RS_WAIT_PRINT:	sprintf(info, "Waiting for PrintClient"); break;

	case RS_RIPPING:	permil	= _get_val(buffer, "RipPerMilDone");
 						elapsed	= _get_val(buffer, "RipSecElapsed");
						remain	= _get_val(buffer, "RipSecRemaining");
						sprintf(info, "Ripping: %d%% Time Elapsed: %d:%02d Remaining: %d:%02d", 
										permil/10, 
										elapsed/60,   elapsed%60,
										remain/60, remain%60);
						break;

	case RS_PRINTING:	permil	= _get_val(buffer, "PrintPerMilDone");
 						elapsed	= _get_val(buffer, "PrintSecElapsed");
						remain	= _get_val(buffer, "PrintSecRemaining");	
						sprintf(info, "Ripping: %d%% Time Elapsed: %d:%02d Remaining: %d:%02d", 
										permil/10, 
										elapsed/60,   elapsed%60,
										remain/60, remain%60);
						break;
	case RS_ERROR:		sprintf(info, "Rip Error"); break;
	default:			sprintf(info, "Unknown rip state 0x%08x", status);
	}
}
*/

/*
//--- _get_rip_state ----------------------------------------------------------------
static void _get_rip_state(const char *filepath, char *ripState)
{
	char jobName[MAX_PATH];
	char jqName[MAX_PATH];

	char ext[32];
	FILE *file;
	int size, pos, job, read;
	UINT64 time;
	char *buffer;

	*ripState = 0;

	time = rx_file_get_mtime(_path_jobQueue);
	if (time<=_rip_jq_time) return;
	_rip_jq_time = time;

	split_path(filepath, NULL, jobName, ext);
	
	//--- search job in list ---
	for (job=0; TRUE; job++)
	{
		if (job==SIZEOF(_rip_job)) return;
		if (!strcmp(jobName, _rip_job[job].name)) break;
	}

	//--- job found ---
	size = (int)rx_file_get_size(_path_jobQueue);
	if (size>_rip_job[job].pos)
	{
		size -= _rip_job[job].pos;
		buffer = (char*)malloc(size);
		if (!buffer) return;
		file = rx_fopen(_path_jobQueue, "rb", _SH_DENYNO);
		if (!file) return;
		rx_file_seek(file, _rip_job[job].pos, SEEK_SET);
		read = fread(buffer, 1, size, file);
		fclose(file);
		pos=0;
		while ((pos = _find_in_buf(buffer, size, pos, "JOB "))>=0)
		{
			read_job_info(&buffer[pos], jqName, ripState);
			if (!strcmp(jqName, _rip_job[job].name))
			{
				TrPrintfL(TRUE, "RipState job>>%s<<, state >>%s<<", jqName, ripState);
				_rip_job[job].pos += pos;
			}
			pos += 10;
		}
		free(buffer);
	}	
}
*/

//--- _rip_done -------------------------------
static void _rip_done	  (const char *jobName)
{
	int job;

	//--- search job in list ---
	for (job=0; job<SIZEOF(_rip_job); job++)
	{
		if (!strcmp(jobName, _rip_job[job].name)) memset(&_rip_job[job], 0, sizeof(_rip_job[job]));
	}
}

//--- esrip_rip_file -----------------------------------------------------------------
int  esrip_rip_file(const char *jobName, const char *filepath, const char *printEnv, int orientation, int columns, int colDist)
{
	char xmlPath[MAX_PATH];
	char rotate[64];
	char name[MAX_PATH];	
	char xml[1024];
	char fname[100];
	FILE *file;
	int	 len, i;
	
	//--- check servers ---------------------------------------------------------
	#ifdef WIN32
		_start_processes();
	#endif

	if (!_printEnv_exists(printEnv)) return Error(ERR_CONT, 0, "Print Environment >>%s<< not found", printEnv);
	_print_client_start(printEnv);

	//--- create XML-File ----------
	split_path(filepath, NULL, fname, xmlPath);

	if (jobName && *jobName) strcpy(name, jobName);
	else split_path(filepath, NULL, name, fname);

// 	len = sprintf(xmlPath, "%s/%s%s.xml", RX_Config.ergosoft.root, ESRIP_HOTFOLDER, name);
 	len = sprintf(xmlPath, "%s%s%s.xml", ESRIP_ROOT, ESRIP_HOTFOLDER, name);

	if (rx_def_is_scanning(RX_Config.printer.type)) 
		orientation=(orientation+270)%360;
	
	if (orientation)
	{
		sprintf(rotate, "<Rotate Mode=\"%d\"/>\n", orientation);
		len += sprintf(&xmlPath[len], "-%d°", orientation);
	}
	else *rotate=0;
 	
	file = fopen(xmlPath, "w+b");
	if (file!=NULL)
	{
		for (i=0; i<SIZEOF(_rip_job); i++)
		{
			if (!*_rip_job[i].name)
			{
				strcpy(_rip_job[i].name, name);
				_rip_job[i].pos = (int)rx_file_get_size(_path_jobQueue)-100;
				break;
			}
		}
		if (i==SIZEOF(_rip_job)) Error(WARN, 0, "Rip Job List Overflow");

		len = sprintf(xml, 
			"<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
			"<HotFile xmlns:xsi=\"http://www.w3.org//2001//XMLSchema-instance\"\n"          
			"		  xmlns:xsd=\"http://www.w3.org//2001//XMLSchema\"\n"           
			"		  xmlns=\"http://tempuri.org//XMLSchema.xsd\">\n"
			"    <Job Name=\"%s\">\n"
			"		  <PrintEnvMedia>\n"
			"		    <PrintEnv>%s</PrintEnv>\n"
			"		  </PrintEnvMedia>\n"
			, name, printEnv);

			for (i=0; i<columns; i++)
			{
				len += sprintf(&xml[len],
			"		  <Image FileName=\"%s\">\n"
			"		    %s"							// rotate
			"		    <Position>%d.%03d 0</Position>\n"
			"		  </Image>\n"
			, filepath, rotate, i*colDist/1000, i*colDist%1000);
			}
			len += sprintf(&xml[len],
			"    </Job>\n"
			"</HotFile>\n");
		fwrite(xml, 1, strlen(xml), file);
		fclose(file);
	}

	return REPLY_OK;
}