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

#include <io.h>
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


//--- Externals ---------------------------------------------------------------



//--- Modlue Globals -----------------------------------------------------------------
static RX_SOCKET		_ppws=INVALID_SOCKET;
static HANDLE			_mutex=NULL;

//--- Prototypes --------------------------------------------------------------
int	ppws_delete_CostomerJobs(const char *customer);


//--- esrip_init ----------------------------------------------------------------
void errip_init(void)
{
	_mutex = rx_mutex_create();
}

//--- esrip_end ------------------------------------------------------------------
void esrip_end(void)
{
	rx_mutex_destroy(&_mutex);
}


//--- _get_line ------------------------------------------
static int _get_line(char *buffer, int *pos, char *str, int strlen)
{
	while (buffer[*pos])
	{
		if (--strlen<0) return FALSE;
		if  (buffer[*pos]=='\r' && buffer[(*pos)+1]=='\n')
		{
			*str=0;
			(*pos)+=2;
			return TRUE;
		}
		*str++ = buffer[(*pos)++];
	}
	return FALSE;
}

//--- _get_id ------------------------------------------
static void _get_id(char *buffer, int *pos, char *id, int len)
{
	while (buffer[*pos]>='0' && buffer[*pos]<='9') *id++=buffer[(*pos)++];
	while (buffer[*pos]==' ') (*pos)++;
	*id=0;
}

//--- _gettag ------------------------------------------
static int _gettag(char *buffer, int *pos, char *str, int strlen)
{
	*str=0;
	if (!buffer[*pos]) return FALSE;

	while (buffer[*pos])
	{
		if (--strlen<0) return FALSE;
		if  (buffer[*pos]==' ')
		{
			*str=0;
			(*pos)++;
			return TRUE;
		}
		*str++ = buffer[(*pos)++];
	}
	*str=0;
	return TRUE;
}

//--- _get_prop_int ----------------------------------------
static int _get_prop_int(const char *xml, const char *tag)
{
	char tagstr[32];
	char *str;
	sprintf(tagstr, "<%s>", tag);
	str = strstr(xml, tagstr);
	if (str) return atoi(&str[strlen(tagstr)]);
	return 0;
}


//--- ppws_open ------------------------------------------------
static int ppws_open(void)
{
	int ppws=_ppws;
	int  ret;
	if (_ppws==INVALID_SOCKET) 
	{ 
		if (ret =sok_open_client(&_ppws, RX_Config.ergosoft.ipAddr, ESRIP_PORT, SOCK_STREAM)==REPLY_OK) 
		{
			ppws_delete_CostomerJobs(RX_Process_Name);
			esrip_send_print_env(NO_SOCKET);
		}
		else return FALSE;
	}
	return TRUE;
}

//--- ppws_send ----------------------------------------------------
static int ppws_send(void *msg, int len)
{
	int ret;
	ret = send(_ppws, msg, len, 0);
	if (ret==-1)
	{
		ret = GetLastError();
		ret=sok_open_client(&_ppws, RX_Config.ergosoft.ipAddr, ESRIP_PORT, SOCK_STREAM);
		if (ret!=REPLY_OK) return Error(ERR_CONT, 0, "Cound not connect to Ergosoft Web Interface");
		return send(_ppws, msg, len, 0);
	}
	return ret;
}

//--- ppws_get ------------------------------------------------------
static int ppws_get(char *resourceType, char *resourceId, char *reply, int replySize)
{
	int				ret, len;
	char			str[150];
	char			rsc[100];
	char			*plen;
	char			*src;

	//--- connect --------------------------------------------------------
	if (!ppws_open()) return REPLY_ERROR;

	//--- request -------------------------------------
	len = sprintf(rsc, "/ppws/%s/", resourceType);
	if (resourceId!=NULL && *resourceId)
	{
		for (src=resourceId; *src; src++)
		{
			if (*src==' ')  len += sprintf(&rsc[len], "%%20");
			else            rsc[len++] = *src;
		}
		rsc[len++]=0;
	}
	len = sprintf(str, "GET %s HTTP/1.1\r\n"
					   "Connection: keep-alive\r\n"
					   "Cache-Control: max-age=0\r\n"
//					   "Accept: text/html\r\n"
					   "\r\n"
					  , rsc);
					  
	ret=ppws_send(str, len);

	//--- get header ---
	memset(str, 0, sizeof(str));
	if (len=recv(_ppws, str, sizeof(str), 0)>0)
	{
		//--- get reply ---
		len = strlen(str);
		plen=strstr(str, "Content-Length: ");
		memset(reply, 0, sizeof(reply));
		if (plen!=NULL  && atoi(&plen[15])>0) len=recv(_ppws, reply, replySize-1, 0);
		else len=0;
		if (len>=0)
		{
			reply[len]=0;
//			sok_close(&_ppws);
			return REPLY_OK;
		}
	}
//	sok_close(&_ppws);
	return REPLY_ERROR;
}

//--- ppws_put -------------------------------------------------------------------------
static int ppws_put(char *resourceType, char *resourceId, char *sendXml, char *reply, int replySize)
{
	int				ret, len, pos, replyLen;
	char			cmd[16];
	char			buffer[2048];	
	char			str[1024];
	char			rsc[100];
	char			*src;

	//--- connect --------------------------------------------------------
	if (!ppws_open()) return REPLY_ERROR;

	//--- request -------------------------------------
	len = sprintf(rsc, "/ppws/%s/", resourceType);
	strcpy(cmd, "PUT");
	if (resourceId!=NULL && *resourceId)
	{
		for (src=resourceId; *src; src++)
		{
			if (*src==' ')  len += sprintf(&rsc[len], "%%20");
			else            rsc[len++] = *src;
		}
		rsc[len++]=0;
		strcpy(cmd, "POST");
	}
	
	len = sprintf(str, "%s %s HTTP/1.1\r\n"
					   "Accept: text/html\r\n"
					   "Content-type: text/html\r\n"
					   "Content-length: %ld\r\n"
					   "\r\n"
					   "%s"
					   "\r\n"
					  , cmd, rsc, strlen(sendXml), sendXml);

	if (len>sizeof(str))
	{
		pos=0;
		len=1/pos;
	}
	ret=ppws_send(str, len);
	if (ret<=0)
		ret=GetLastError();
	memset(buffer, 0, sizeof(buffer));
	memset(str, 0, sizeof(str));
	//--- get header ---
	len = recv(_ppws, buffer, sizeof(buffer), 0);
	if (len<0)
		len=GetLastError();
	pos = 0;
	replyLen = 0;
	*reply=0;
	if (len>0 && pos<len)
	{
		while (_get_line(buffer, &pos, str, sizeof(str)))
		{
			if (str[0]==0)
			{
				memcpy(reply, &buffer[pos], len-pos); // copy rest of buffer
				replyLen  -= (len-pos);
				replySize -= (len-pos);
				reply     += (len-pos);
				// receive additional messages if needed
				while (replyLen)
				{
					len = recv(_ppws, reply, replySize, 0);
					reply += len;
					replyLen-=len;
					replySize-=len;
				}
				*reply=0;
				sok_close(&_ppws);
				return REPLY_OK;
			}		
			else if (str_start(str, "Content-Length: "))
			{
				replyLen=atoi(&str[16]);
			}
		}
	}
	sok_close(&_ppws);
	return REPLY_ERROR;
}

//--- ppws_delete -------------------------------------------------------------------------
static int ppws_delete(char *resourceType, int id)
{
	int				ret, len, pos, replyLen;
	char			buffer[2048];	
	char			str[150];
	char			reply[128];
	int				replySize = sizeof(reply);

	//--- connect --------------------------------------------------------
	if (!ppws_open()) return REPLY_ERROR;

	//--- request -------------------------------------
	len = sprintf(str, "DELETE /ppws/%s/%d HTTP/1.1\r\n"
					   "Connection: keep-alive\r\n"
					   "Cache-Control: max-age=0\r\n"
					   "\r\n", 
					   resourceType, id);
					  
	ret=ppws_send(str, len);

	memset(buffer, 0, sizeof(buffer));
	//--- get header ---
	len = recv(_ppws, buffer, sizeof(buffer), 0);
	pos = 0;
	replyLen = 0;
	if (len>0 && pos<len)
	{
		while (_get_line(buffer, &pos, str, sizeof(str)))
		{
			if (str[0]==0)
			{
				memcpy(reply, &buffer[pos], len-pos); // copy rest of buffer
				replyLen  -= (len-pos);
				replySize -= (len-pos);
		//		reply     += (len-pos);
				// receive additional messages if needed
				while (replyLen)
				{
					len = recv(_ppws, reply, replySize, 0);
		//			reply += len;
					replyLen-=len;
					replySize-=len;
				}
				*reply=0;
				return REPLY_OK;
			}
			else if (str_start(str, "Content-Length: "))
			{
				replyLen=atoi(&str[16]);
			}
		}
	}
//	sok_close(&ppws);
	return REPLY_ERROR;
}

//--- ppws_add_Customer -------------------------------------
int	ppws_add_Customer(const char *customer)
{
	char xml[128];
	char reply[128];
	char id[16];
	int pos;

	ppws_get("Customer",  NULL, reply, sizeof(reply));
	pos=0;
	while(TRUE)
	{
		_get_id(reply, &pos, id, sizeof(id));
		if (!*id) break;
		ppws_get("Customer",  id, xml, sizeof(xml));
		if (strstr(xml, customer)) return atoi(id);
	}

	sprintf(xml, "<Customer><Name>%s</Name></Customer>\n", customer);		
	if (ppws_put("Customer", NULL, xml, reply, sizeof(reply))==REPLY_OK) return atoi(reply);
	return 0;
}

//--- ppws_add_PrintTarget -------------------------------------
int	ppws_add_PrintTarget(const char *printTarget)
{
	char xml[128];
	char reply[128];
	char id[16];
	int pos;

	ppws_get("PrintTarget",  NULL, reply, sizeof(reply));
	pos=0;
	while(TRUE)
	{
		_get_id(reply, &pos, id, sizeof(id));
		if (!*id) break;
		ppws_get("PrintTarget",  id, xml, sizeof(xml));
		if (strstr(xml, printTarget)) return atoi(id);
	}

	sprintf(xml, "<PrintTarget><Name>%s</Name></PrintTarget>\n", printTarget);		
	if (ppws_put("PrintTarget", NULL, xml, reply, sizeof(reply))==REPLY_OK) return atoi(reply);
	return 0;
}

//--- ppws_check_PrintEnv ---------------------------------------
int ppws_check_PrintEnv(const char *printEnv)
{
	char	reply[4096];
	char	str[MAX_PATH];
	int		pos;

	if (!*printEnv) return REPLY_ERROR;

	ppws_get("PrintEnv",  NULL, reply, sizeof(reply));
	pos=0;
	while(_get_line(reply, &pos, str, sizeof(str)))
	{
		if (!strcmp(str, printEnv)) return REPLY_OK;
	}
	return REPLY_ERROR;
}

//--- ppws_delete_CostomerJobs --------------------------
int	ppws_delete_CostomerJobs(const char *customer)
{
	int  customerId;
	int  jobId;
	char id[10];
	char reply[1024];
	char *ch;

	if (customerId = ppws_add_Customer(customer))
	{
		sprintf(id, "%d", customerId);
		ppws_get("CustomerJobList", id, reply, sizeof(reply));
		
		for(ch=reply; jobId=atoi(ch); ch++)
		{
			ppws_delete("CustomerJob", jobId);
			while (*ch>'0') ch++;
		}
	}

	return REPLY_OK;
}

//--- esrip_tick --------------------------------------------
void esrip_tick(void)
{
	rx_mutex_lock(_mutex);
	if (_ppws==INVALID_SOCKET)
		ppws_open();
	rx_mutex_unlock(_mutex);
}

//--- esrip_map_drive ------------------------------------
void esrip_map_drive(const char *path)
{
	//--- test ---
//	ppws_delete_CostomerJobs(RX_Process_Name);
	if (TRUE)
	{
	//	esrip_rip_file("D:\\Develop\\Data\\GT-Print-Data\\Textile\\Scan-Test-01.tif", "Radex 4C", 0, 2, 200000);
	}
	if (FALSE)
	{
		char xml[1024];
		char reply[1024];
		char reply2[1024]; 
		char str[128];
//		char id[10];
		int  ret, pos;

		sprintf(xml, 
			"<Customer>\n"
				"<Name>"
					"Mr_smith"
				"</Name>"
			"</Customer>\n");
		
		ret = ppws_put("Customer", NULL, xml, reply, sizeof(reply));
		if (ret) return;
		printf("Add new Customer:: id=%s\n", reply);

		ret = ppws_get("Customer", NULL, reply, sizeof(reply));
		printf("\nCustomers: %s\n", reply);
		pos=0;
		while(_gettag(reply, &pos, str, sizeof(str)))
		{	
			ret = ppws_get("Customer", str, reply2, sizeof(reply2));
			printf("Customer %s: >>%s<<n", str, reply2);
		}
		ret=ret;
	}
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
}


//--- esrip_send_print_env -----------------------------
void esrip_send_print_env	(RX_SOCKET socket)
{
	SPrintEnvEvt	msg;
	char			reply[4096];
	int pos;

	rx_mutex_lock(_mutex);

	if (socket)
	{
		TrPrintf(TRUE, "do_get_print_env");
		msg.hdr.msgLen = sizeof(msg.hdr);
		msg.hdr.msgId = REP_GET_PRINT_ENV;
		GUI_send_msg(socket, &msg);
	}

	if (_ppws)
	{
		msg.hdr.msgId = BEG_GET_PRINT_ENV;
		GUI_send_msg(socket, &msg);

		msg.hdr.msgLen = sizeof(msg);
		msg.hdr.msgId = ITM_GET_PRINT_ENV;

		ppws_get("PrintEnv",  NULL, reply, sizeof(reply));
		pos=0;
		while(_get_line(reply, &pos, msg.printEnv, sizeof(msg.printEnv)))
		{
			GUI_send_msg(socket, &msg);
	//		ppws_get("PrintEnv",  msg.printEnv, printEnv, sizeof(printEnv));
		}

		msg.hdr.msgLen = sizeof(msg.hdr);
		msg.hdr.msgId = END_GET_PRINT_ENV;
		GUI_send_msg(socket, &msg);
	}

	/*	
	//--- Media Types ------------------------
	TrPrintf(TRUE, "");
	TrPrintf(TRUE, "Media Types");
	ppws_get("MediaType",  NULL, reply, sizeof(reply));
	pos=0;
	while(_get_line(reply, &pos, msg.printEnv, sizeof(msg.printEnv)))
	{
		TrPrintf(TRUE, msg.printEnv);
		ppws_get("MediaType",  msg.printEnv, printEnv, sizeof(printEnv));
	}

	//--- Media Sizes ------------------------
	TrPrintf(TRUE, "");
	TrPrintf(TRUE, "Media Sizes");
	ppws_get("MediaSize",  NULL, reply, sizeof(reply));
	pos=0;
	while(_get_line(reply, &pos, msg.printEnv, sizeof(msg.printEnv)))
	{
		TrPrintf(TRUE, msg.printEnv);
		ppws_get("MediaSize",  msg.printEnv, printEnv, sizeof(printEnv));
	}

	//--- Image Presets ------------------------
	TrPrintf(TRUE, "");
	TrPrintf(TRUE, "Image Presets");
	ppws_get("ImagePreset",  NULL, reply, sizeof(reply));
	pos=0;
	while(_get_line(reply, &pos, msg.printEnv, sizeof(msg.printEnv)))
	{
		TrPrintf(TRUE, msg.printEnv);
		ppws_get("ImagePreset",  msg.printEnv, printEnv, sizeof(printEnv));
	}
	*/
	rx_mutex_unlock(_mutex);
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

	if (printEnv==0)
	{
		strcpy(dataPath, filepath);
		return REPLY_OK;
	}

	split_path(filepath, NULL, filename, ext);
	
	//--- check with subfolder ---
	sprintf(dataPath, "%s/%s/%s/%s_Ticket.txt", RX_Config.ergosoft.root, printEnv, filename, filename);
	if (!_access(dataPath, 0x02)) 
	{
		sprintf(dataPath, "%s/%s/%s/%s.usf", RX_Config.ergosoft.root, printEnv, filename, filename);
		if (_access(dataPath, 0x02))
			sprintf(dataPath, "%s/%s/%s/%s_%%s.usf", RX_Config.ergosoft.root, printEnv, filename, filename);
		return REPLY_OK;
	}

	//--- check without subfolder ---
	sprintf(dataPath, "%s/%s/%s_Ticket.txt", RX_Config.ergosoft.root, printEnv, filename);
	if (!_access(dataPath, 0x02)) 
	{
		sprintf(dataPath, "%s/%s/%s.usf", RX_Config.ergosoft.root, printEnv, filename);
		if (_access(dataPath, 0x02))
			sprintf(dataPath, "%s/%s/%s_%%s.usf", RX_Config.ergosoft.root, printEnv, filename);
		return REPLY_OK;
	}
	return REPLY_NOT_RIPPED;
}

//--- esrip_rip_file -----------------------------------------------------------------

int  esrip_rip_file_HOT_FOLDER	(const char *filepath, const char *printEnv, int orientation)
{
	char str[MAX_PATH];
	char rotate[64];	
	char xml[1024];
	char fname[100];
	FILE	*file;
	
	//--- check servers ---------------------------------------------------------
	#ifdef WIN32
		if (rx_process_running_cnt("PpHotFld.exe", NULL)==0)
		{
			Error(ERR_STOP, 0, "Ergosoft Hot Folder not runnning");
			return REPLY_ERROR;
		}
		/*
		if (rx_process_running_cnt("PpRipSrv.exe", NULL)==0)
		{
			Error(ERR_CONT, 0, "Ergosoft Rip Server not runnning");
			return REPLY_ERROR;
		}
		*/
	#endif

	//--- create XML-File ----------
	split_path(filepath, NULL, fname, str);

 	sprintf(str, "%s/%s/%s%s-%d°.xml", RX_Config.ergosoft.root, printEnv, ESRIP_HOTFOLDER, fname, orientation);

	file = fopen(str, "w+b");
	if (file!=NULL)
	{
		if (rx_def_is_scanning(RX_Config.printer.type)) 
			orientation=(orientation+270)%360;
		switch(orientation)
		{
		case  90:	strcpy(rotate, "<Rotate Mode=\"90\"/>");	break; 
		case 180:	strcpy(rotate, "<Rotate Mode=\"180\"/>");	break; 
		case 270:	strcpy(rotate, "<Rotate Mode=\"270\"/>");	break; 
		default:	strcpy(rotate, "");	break; 
		}

		sprintf(xml, 
			"<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
			"<HotFile xmlns:xsi=\"http://www.w3.org//2001//XMLSchema-instance\"\n"          
			"		  xmlns:xsd=\"http://www.w3.org//2001//XMLSchema\"\n"           
			"		  xmlns=\"http://tempuri.org//XMLSchema.xsd\">\n"   
			"		  <Image FileName=\"%s\">\n"
			"		  %s\n"	// rotate
			"		  <Scale Mode=\"PERCENT\" Value=\"25\"/>"				  
			"		  </Image>\n"
			"</HotFile>\n", filepath, rotate);
		fwrite(xml, 1, strlen(xml), file);
		fclose(file);
		Error(WARN, 0, "Ergosoft Rip: SCALE for tests");
	}

	return REPLY_OK;
}

//--- esrip_rip_file -----------------------------------------------------------------
int  esrip_rip_file(const char *jobName, const char *filepath, const char *printEnv, int orientation, int columns, int colDist)
{
	int customerId;
	int jobPrintJobId;
	int customerJobId;
	int status, exit, len, i, ret;
	char xml[2048];
	char reply[2048];
	char rotate[64];
	char name[MAX_PATH];
	char jobIdStr[16];

	rx_mutex_lock(_mutex);

	if (jobName && *jobName) strcpy(name, jobName);
	else split_path(filepath, NULL, name, rotate);

	if (ppws_check_PrintEnv(printEnv)) 
	{
		rx_mutex_unlock(_mutex);
		return Error(ERR_CONT, 0, "RIP: PrintEvnvironment >>%s<< not found", printEnv);
	}

	if (customerId = ppws_add_Customer(RX_Process_Name))
	{
		//--- create job xml ----------------------
		if (rx_def_is_scanning(RX_Config.printer.type)) 
			orientation=(orientation+270)%360;
		switch(orientation)
		{
		case  90:	strcpy(rotate, "<Rotate Mode=\"90\"/>\n");	break; 
		case 180:	strcpy(rotate, "<Rotate Mode=\"180\"/>\n");	break; 
		case 270:	strcpy(rotate, "<Rotate Mode=\"270\"/>\n");	break; 
		default:	strcpy(rotate, "");	break; 
		}

		len = sprintf(xml, 
			"<JobPrintJob>\n"
			"  <Job Name=\"%s\">\n"
			"    <PrintEnvMedia>\n"
			"      <PrintEnv>%s</PrintEnv>\n"
			"    </PrintEnvMedia>\n"
			, name, printEnv);

		for (i=0; i<columns; i++)
		{
			len += sprintf(&xml[len],
				"    <Image FileName=\"%s\">\n"
				"      %s"							// rotate
				"       <Position>%d.%03d 0</Position>\n"
				"    </Image>\n"
				, filepath, rotate, i*colDist/1000, i*colDist%1000);
		}
		len += sprintf(&xml[len], 
			"  </Job>\n"
			"</JobPrintJob>");
		ppws_put("JobPrintJob", NULL, xml, reply, sizeof(reply));
		jobPrintJobId = atoi(reply);
		if (jobPrintJobId)
		{
			sprintf(xml, 
			"<CustomerJob>\n"
			"  <Customer>%d</Customer>\n"
			"  <JobPrintJobs>%d</JobPrintJobs>\n"
			"</CustomerJob>",
			customerId, jobPrintJobId
			);
			*reply=0;
			ppws_put("CustomerJob", NULL, xml, reply, sizeof(reply));
			if (customerJobId = atoi(reply))
			{
				TrPrintfL(TRUE, "EsRip: Starting job[%d]: >>%s<<", customerJobId, filepath);
				//--- start the job ---
				sprintf(xml,
				"<CustomerJob>\n"
				"  <Status>1</Status>\n"
				"</CustomerJob>/n"
				);
				sprintf(jobIdStr, "%d", customerJobId);
				ppws_put("CustomerJob", jobIdStr, xml, reply, sizeof(reply));
	
				//--- wait until done ---
				exit = FALSE;
				do
				{
					rx_sleep(100);
					ret = ppws_get("CustomerJob",  jobIdStr, reply, sizeof(reply));
					if (ret) 
					{
						rx_mutex_unlock(_mutex);
						return Error(ERR_CONT, 0, "RIP Error ripping >>%s<<", filepath); 
					}
					TrPrintfL(TRUE, ">>CustomerJob/%s<<\nreply:\n%s\n", jobIdStr, reply);
					status = _get_prop_int(reply, "Status");
					switch(status)
					{
					case    0:	exit = TRUE;
								Error(ERR_CONT, 0, "ripping >>%s<< Error >>%s<<", filepath, reply);
								break;

				//	case    1: exit = Error(ERR_STOP, 0, "Ripping >>%s<<: Waiting on PrintClient", filepath); break;
					case 1000: exit=TRUE; break;
					}
				} while (!exit);
				ppws_delete("CustomerJob",  customerJobId);
				rx_mutex_unlock(_mutex);
				return REPLY_OK;
			}
		}
	}
	rx_mutex_unlock(_mutex);
	return REPLY_ERROR;
}
