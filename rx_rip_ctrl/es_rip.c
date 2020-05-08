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
#include "rx_common.h"
#include "rx_def.h"
#include "rx_error.h"
#include "rx_file.h"
#include "rx_trace.h"
#include "rx_threads.h"
#include "tcp_ip.h"
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
	char		name[MAX_PATH];
	char		path[MAX_PATH];
	int			pos;
	RX_SOCKET	socket;
} SRipJobInfo;

static HANDLE		_check_thread_id;
static int			_check_thread_running;
static SRipJobInfo	_rip_job[MAX_RIP_JOBS];
static UINT64		_rip_jq_time;

//--- Prototypes --------------------------------------------------------------
static int _start_processes(void);
static int _print_client_running(const char *printEnv);
static int _print_client_start(const char *printEnv);
static int _printEnv_exists	(const char *printEnv);
static void _get_rip_state	(const char *filepath, char *ripState);
static void _rip_done		(const char *jobName);

static void *_check_thread	(void *par);

static void _copy_files		(RX_SOCKET *socket, const char *path);


//--- esrip_init ----------------------------------------------------------------
int esrip_init(void)
{
	if (!*_path_user)
	{
		FILE *file;
		char line[MAX_PATH];
		int idx;

		rx_mkdir(PATH_TEMP);

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

			_check_thread_running = TRUE;
			_check_thread_id = rx_thread_start(_check_thread, NULL, 0, "ESRIP_CHECK_THREAD");
			return REPLY_OK;
		}
	}
	return REPLY_ERROR;
}

//--- esrip_end ------------------------------------------------------------------
void esrip_end(void)
{
	_check_thread_running = FALSE;
}

//--- esrip_map_drive ------------------------------------
void esrip_map_drive(const char *path)
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

			/*
			Error(WARN, 0, "TEST");
			strcpy(_rip_jobname, "Scan-Test-01");
			_rip_jq_pos = (int)rx_file_get_size(_path_jobQueue);
			if (_rip_jq_pos>5000)
				_rip_jq_pos -= 5000;
			*/
		}
	}

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
	*/
}

//--- _print_client_running ----------------------------------
static int _print_client_running(const char *printEnv)
{
	char path[MAX_PATH];
	char line[MAX_PATH];
	wchar_t wline[MAX_PATH];
	char    checkStr[100];
	wchar_t wcheckStr[100];
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
			fgets(line, 3, file);
			if ((UCHAR)line[0]==0xff && (UCHAR)line[1]==0xfe)
			{
				// unicode
				l=0;
				wcheckStr[l++]=L'"';
				for (ch=printEnv; *ch;)
				{
					wcheckStr[l++]=*ch++;
				}
				wcheckStr[l++]=L'"';
				wcheckStr[l++]=0;

				while (!found && fgetws(wline, SIZEOF(wline), file))
				{
					if (wcsstr(wline, wcheckStr)) found=TRUE;
				}
			}
			else 
			{
				// ascii
				fseek(file, 0, SEEK_SET);
				sprintf(checkStr, "\"%s\"", printEnv);
				while (!found && fgets(line, SIZEOF(line), file))
				{
					if (strstr(line, checkStr)) found=TRUE;
				}
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
	
		#ifdef _WIN64 // is this neccessary?
			char param[MAX_PATH];
			sprintf(process, "\"%s%s\"", _path_exe, "PpDesp.exe");
			sprintf(param, "\"%s\"", printEnv);
			rx_process_start(process, param);
		#else
			sprintf(process, "\"%s%s\" \"%s\"", _path_exe, "PpDesp.exe", printEnv);
			rx_process_start(process, NULL);
		#endif
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
		sok_send(&socket, &msg);
	}

	if (*_path_user)
	{
		msg.hdr.msgId = BEG_GET_PRINT_ENV;
		sok_send(&socket, &msg);

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
					sok_send(&socket, &msg);
				}
			}
		}
		rx_search_close(search);

		msg.hdr.msgLen = sizeof(msg.hdr);
		msg.hdr.msgId = END_GET_PRINT_ENV;
		sok_send(&socket, &msg);
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
int  esrip_file_ripped(const char *filepath, const char *printEnv, char *dataPath, char *ripState)
{
	char filename[MAX_PATH];
	char ext[16];

	if (!*filepath)
	{
		*dataPath = 0;
		return REPLY_OK;
	}

	if (printEnv==0)
	{
		strcpy(dataPath, filepath);
		return REPLY_OK;
	}

	split_path(filepath, NULL, filename, ext);
	
	//--- check tiff ---------------------------------------------------------------
	sprintf(dataPath, "%s/%s/%s/%s_K.tif", PATH_RIPPED_DATA, printEnv, filename, filename);
	if (!_access(dataPath, 0x02)) 
	{
		sprintf(dataPath, "%s/%s/%s/%s.tif", PATH_RIPPED_DATA, printEnv, filename, filename);
		_rip_done(filename);
		return REPLY_OK;
	}

	//--- check with subfolder ---
	sprintf(dataPath, "%s/%s/%s/%s_Ticket.txt", PATH_RIPPED_DATA, printEnv, filename, filename);
	if (!_access(dataPath, 0x02)) 
	{
		sprintf(dataPath, "%s/%s/%s/%s.usf", PATH_RIPPED_DATA, printEnv, filename, filename);
		if (_access(dataPath, 0x02))
			sprintf(dataPath, "%s/%s/%s/%s_%%s.usf", PATH_RIPPED_DATA, printEnv, filename, filename);
		_rip_done(filename);
		return REPLY_OK;
	}

	//--- check without subfolder ---
	sprintf(dataPath, "%s/%s/%s_Ticket.txt", PATH_RIPPED_DATA, printEnv, filename);
	if (!_access(dataPath, 0x02)) 
	{
		sprintf(dataPath, "%s/%s/%s.usf", PATH_RIPPED_DATA, printEnv, filename);
		if (_access(dataPath, 0x02))
			sprintf(dataPath, "%s/%s/%s_%%s.usf", PATH_RIPPED_DATA, printEnv, filename);
		_rip_done(filename);
		return REPLY_OK;
	}

	//--- get rip state ------------------------------------------------------------
	_get_rip_state(filepath, ripState);

	return REPLY_NOT_RIPPED;
}

//--- _find_in_buf ----------------------------------------
static int _find_in_buf(char *buf, int size, int start, char *find)
{
	int len=(int)strlen(find);
	int pos;

//	if ((int)strlen(buf)<start) return -1;

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

//--- _read_job_info -----------------------------------------------------------------
static void _read_job_info(char *buffer, int *id, char *jobName, char *info)
{
	int  pos;
	int	 status;
	int	 permil, elapsed, remain;
	
	pos = _find_in_buf(buffer, 256, 10, "\\[");
	if (pos>0) *id  = atoi(&buffer[pos+2]);
	else *id=0;
	buffer+= pos;
	_get_str(buffer, "JobName", jobName);
	
	status				= _get_val(buffer, "Status");
	TrPrintfL(TRUE, "status=0x%08x", status);
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
						sprintf(info, "Writing File: %d%% Time Elapsed: %d:%02d Remaining: %d:%02d", 
										permil/10, 
										elapsed/60,   elapsed%60,
										remain/60, remain%60);
						break;
	case RS_ERROR:		sprintf(info, "Rip Error"); break;
	default:			sprintf(info, "Unknown rip state 0x%08x", status);
	}
}

//--- _get_rip_state ----------------------------------------------------------------
static void _get_rip_state(const char *filepath, char *ripState)
{
	char jobName[MAX_PATH];
	char jqName[MAX_PATH];

	char ext[32];
	FILE *file;
	int size, pos, job, read, jobId;
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
		read = (int)fread(buffer, 1, size, file);
		fclose(file);
		pos=0;
		while ((pos = _find_in_buf(buffer, size, pos, "JOB "))>=0)
		{
			_read_job_info(&buffer[pos], &jobId, jqName, ripState);
			if (!strcmp(jqName, _rip_job[job].name))
			{
				TrPrintfL(TRUE, "RipState job[%d]>>%s<<, state >>%s<<", jobId, jqName, ripState);
				_rip_job[job].pos += pos;
			}
			pos += 10;
		}
		free(buffer);
	}	
}

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
int  esrip_rip_file(RX_SOCKET socket, const char *jobName, const char *filepath, const char *printEnv, int orientation, int columns, int colDist)
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

	rx_mkdir(PATH_RIPPED_DATA);
	sprintf(fname, "%s%s", PATH_RIPPED_DATA, printEnv);
	rx_mkdir(fname);

	//--- create XML-File ----------
	split_path(filepath, NULL, fname, xmlPath);

	if (jobName && *jobName) strcpy(name, jobName);
	else split_path(filepath, NULL, name, fname);

 	len = sprintf(xmlPath, "%s%s%s.xml", ESRIP_ROOT, ESRIP_HOTFOLDER, name);

//	if (rx_def_is_scanning(RX_Config.printer.type)) 
//		orientation=(orientation+270)%360;
	
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
				_rip_job[i].socket = socket;
				strcpy(_rip_job[i].name, name);
				sprintf(_rip_job[i].path, "%s%s/%s/",  PATH_RIPPED_DATA, printEnv, name);
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

//--- _check_thread -------------------
static void *_check_thread(void *par)
{
//	char ext[32];
	FILE *file;
	int size, pos, startPos, job, read;
	int		done;
	UINT64	time;
	int		bufSize;
	char	*buffer;
	SRipEvt msg;

	bufSize  = 0;
	buffer	 = NULL;
	startPos = 0;
	msg.hdr.msgId  = EVT_RIP_START;
	msg.hdr.msgLen = sizeof(msg);
	while (_check_thread_running)
	{
		rx_sleep(1000);

		time = rx_file_get_mtime(_path_jobQueue);
		if (time<=_rip_jq_time) continue;

		//--- read the file ---
		size = (int)rx_file_get_size(_path_jobQueue);
		TrPrintfL(TRUE, "file size=%d, startPos=%d", size, startPos);
		size -= startPos;
		if (size>bufSize)
		{
			if (buffer) free(buffer);
			bufSize = size+0x1000;
			buffer = malloc(bufSize);
		}
		if (buffer==NULL) 
		{
			Error(ERR_STOP, 0,  "Could not allocate buffer"); 
			return NULL;
		}
		file = rx_fopen(_path_jobQueue, "rb", _SH_DENYNO);
		if (file==NULL) continue;
		rx_file_seek(file, startPos, SEEK_SET);
		read = (int)fread(buffer, 1, size, file);
		fclose(file);
		TrPrintfL(TRUE, "file size=%d, read=%d", size, read);
		_rip_jq_time = time;

		pos=10;
		done = 0;
		while ((pos = _find_in_buf(buffer, size, pos, "JOB "))>=0)
		{
			if (done==0) done=1;
			_read_job_info(&buffer[pos], &msg.jobId, msg.jobName, msg.state);
			if (*msg.state) 
			{
				if (done==1) 
				{	
					startPos += pos-20;
					done=2;
				} 
			}
			TrPrintfL(TRUE, "RipState job[%d]>>%s<<, state >>%s<<", msg.jobId, msg.jobName, msg.state);
			for (job=0; job<SIZEOF(_rip_job); job++)
			{
				if (!strcmp(msg.jobName, _rip_job[job].name))
				{
					if (_rip_job[job].socket && _rip_job[job].socket!=INVALID_SOCKET)
						sok_send(&_rip_job[job].socket, &msg);
					if  (*msg.state) _rip_job[job].pos += pos;
					else 
					{
//						TrPrintfL(TRUE, "Ripped, copy now");
//						_copy_files(&_rip_job[job].socket, _rip_job[job].path);
						memset(&_rip_job[job], 0, sizeof(_rip_job[job]));
					}
				}
			}
			pos += 10;
		}
		if (done==1) startPos += size-20;
	}
	if (buffer) free(buffer);
	return NULL;
}

//--- _copy_files -------------------------------------------------------------------
static void _copy_files(RX_SOCKET *socket, const char *path)
{
	SEARCH_Handle	search;
	int				isDir;
	char			fname[MAX_PATH];
	SFSDirEntry		msg;

	msg.hdr.msgId	= EVT_RFS_DIR_ENTRY;
	msg.hdr.msgLen	= sizeof(msg);
	search = rx_search_open(path, "*");
	while (rx_search_next(search, fname, sizeof(fname), &msg.timeModified, &msg.size, &isDir))
	{
		if (!isDir)
		{
			TrPrintfL(TRUE, "Copy >>%s<<", fname);
			sprintf(msg.name, "%s/%s", path, fname);
			sok_send(socket, &msg);
		}
	}
	rx_search_close(search);	
}
