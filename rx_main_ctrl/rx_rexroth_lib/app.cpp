// RX-Rexroth.cpp : Defines the exported functions for the DLL application.
//

#ifdef linux
	#include <unistd.h>
	#include <spawn.h>
	#include <sys/wait.h>
	#include "./ftplib/ftplib.h"

#else
	#include "stdafx.h"
	#include <io.h>
	#include <WinInet.h>
#endif

#include "rx_common.h"
#include "rx_def.h"
#include <mlpiGlobal.h>
#include <mlpiApiLib.h>
#include <mlpiSystemLib.h>
#include <mlpiLogicLib.h>
#include "rx_file.h"
#include "rx_rexroth.h"
#include "lc.h"
#include "mlpi.h"

//--- defines ---------------------------------------------
#define BOSCHREXROTH_USER	"boschrexroth"
#define BOSCHREXROTH_PW		"boschrexroth"

//---app_start ---------------------------
int app_start(void)
{
	return rex_check_result(mlpiLogicStartApplication(_Connection));
}

//--- app_stop ---------------------------
int app_stop(void)
{
	return rex_check_result(mlpiLogicStopApplication(_Connection));
}

//--- _app_reset ---------------------------
static int _app_reset(MlpiApplicationResetMode mode)
{
	return rex_check_result(mlpiLogicResetApplication(_Connection, mode));
}

//--- app_reset ---------------------------
int app_reset(void)
{
	return _app_reset(MLPI_RESET_WARM);
}

//--- app_reload_user_rights -------------------------
int app_reload_user_rights(void)
{
	return mlpiApiUserAccountControlReload(_Connection);
}

//--- process_entry -------------------------------------------------------
int process_entry(char *ent, unsigned len)
{
  printf("%s\n", ent);
  return 0;
}

//--- transfer ------------------------------------------------------------
#ifdef linux

static char _FileName[100];
static int	_FileSize;

static int _ls(char* str, unsigned len)
{
	if (strstr(str, _FileName)) 
	{
		_FileSize = atoi(str);
		printf("*** size=%d", _FileSize);
	}
	printf(">>%s<< %o\n", str, len);
	return 0;
}

static int _transfer(const char *srcDir, const char *name, const char *ext, const char *dstDir, int overwrite)
{
	char src[MAX_PATH];
	char dst[MAX_PATH];
	sprintf(src, "%s%s%s", srcDir, name, ext);
	sprintf(dst, "%s%s%s", dstDir, name, ext);
	ftp_open(_IpAddr, (char*)BOSCHREXROTH_USER, (char*)BOSCHREXROTH_PW);
	sprintf(_FileName, " %s%s ", name, ext);
	int size=rx_file_get_size(src);
	_FileSize=0;
	ftp_cd((char*)dstDir);
	ftp_ls(0, (char*)"", _ls);
	if (size!=_FileSize)	
		ftp_putfile(src, dst, 0, 0);
	ftp_close();
	if (size != _FileSize) return 4;
	else return 0;
}
#else
static int _transfer(const char *srcDir, const char *name, const char *ext, const char *dstDir, int overwrite)
{
	char src[MAX_PATH];
	char dst[MAX_PATH];
	HINTERNET hInet;
	HINTERNET hFtp;
	int		  exists=FALSE;

	sprintf(src, "%s%s%s", srcDir, name, ext);
	sprintf(dst, "%s%s%s", dstDir, name, ext);

	if ((hInet = InternetOpen(_IpAddr, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0)) == NULL) 
		return 1;

	if ((hFtp = InternetConnect(hInet, _IpAddr, INTERNET_DEFAULT_FTP_PORT, BOSCHREXROTH_USER, BOSCHREXROTH_PW, INTERNET_SERVICE_FTP, INTERNET_FLAG_PASSIVE, 0))==NULL)
		return 2;

	if (!overwrite)
	{
		HINTERNET hfile;
		hfile = FtpOpenFile(hFtp, dst, GENERIC_READ, 0, NULL);
		if (hfile)
		{
			InternetCloseHandle(hfile);
			InternetCloseHandle(hFtp);
			InternetCloseHandle(hInet);
			return 4;
		}
	}
	if(FtpPutFile(hFtp, src, dst, 0, 0) == NULL) 
		return 3;

	InternetCloseHandle(hFtp);
	InternetCloseHandle(hInet);

	return 0;
}
#endif

/*
//--- ftp_get_dir -------------------------------------------------------------------
static int ftp_get_dir(char *path)
{
	HINTERNET		hInet;
	HINTERNET		hFtp;
	HINTERNET		hFind;
	char			response[MAX_PATH];
	DWORD			err, size;
	int				ret;
	WIN32_FIND_DATA data;

	if ((hInet = InternetOpen(_IpAddr, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0)) == NULL) 
		return 1;

	if ((hFtp = InternetConnect(hInet, _IpAddr, INTERNET_DEFAULT_FTP_PORT, BOSCHREXROTH_USER, BOSCHREXROTH_PW, INTERNET_SERVICE_FTP, INTERNET_FLAG_PASSIVE, 0))==NULL)
		return 2;

	FtpSetCurrentDirectory(hFtp, path);
	hFind = FtpFindFirstFileA(hFtp, NULL, &data, 0, NULL);
	size = sizeof(response);
	InternetGetLastResponseInfo(&err, response, &size);
	if (hFind!=NULL) do
	{
		printf("%s\n", data.cFileName);
		ret = InternetFindNextFile(hFind, &data);
		size = sizeof(response);
		InternetGetLastResponseInfo(&err, response, &size);
	} while (ret);
	err = GetLastError();

	if(FtpGetFile(hFtp, "/ata0a/Application.crc", "D:\\Temp\\rexcrc.crc", 0, 0, 0, NULL) == NULL) 
		return 3;

	InternetCloseHandle(hInet);

	return 0;
}
*/

//--- remote_diff --------------------------------
static int _remote_diff(const char *localDir, const char *name, const char *remoteDir)
{
	char src[MAX_PATH];
	char dst[MAX_PATH];
	char localCrc[MAX_PATH];
	char remoteCrc[MAX_PATH];
	FILE *pfile;
	int ret;
			
	memset(localCrc,  0, sizeof(localCrc));
	memset(remoteCrc, 0, sizeof(remoteCrc));
	strcpy(localCrc, "local");
	strcpy(remoteCrc, "remote");

	sprintf(src, "%s%s.crc", remoteDir, name);
	sprintf(dst, "%sremote.crc", PATH_TEMP);

	#ifdef linux
		ftp_open(_IpAddr, (char*)BOSCHREXROTH_USER, (char*)BOSCHREXROTH_PW);
		ftp_getfile(src, dst, 0);
		ftp_close();
	#else
		{
			HINTERNET hInet;
			HINTERNET hFtp;
			if ((hInet = InternetOpen(_IpAddr, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0)) == NULL) 
				return FALSE;

			if ((hFtp = InternetConnect(hInet, _IpAddr, INTERNET_DEFAULT_FTP_PORT, BOSCHREXROTH_USER, BOSCHREXROTH_PW, INTERNET_SERVICE_FTP, INTERNET_FLAG_PASSIVE, 0))==NULL)
				return FALSE;

			if(FtpGetFile(hFtp, src, dst, 0, 0, 0, NULL) == NULL) 
				return TRUE;
			InternetCloseHandle(hInet);
		}
	#endif
	  
	pfile=rx_fopen(dst, "rb", _SH_DENYNO);
	if (pfile) 
	{	
		ret=(int)fread(remoteCrc, 1, sizeof(remoteCrc), pfile);
		fclose(pfile);
		remove(dst);
	}
	sprintf(src, "%s%s.crc", localDir, name);
	pfile=rx_fopen(src, "rb", _SH_DENYNO);
	if (pfile) 
	{	
		ret=(int)fread(localCrc, 1, sizeof(localCrc), pfile);
		fclose(pfile);
	}

	ret=memcmp(localCrc, remoteCrc, 7);
	return ret;
}

//--- app_download ----------------------------------------------------
int app_download(const char *filepath)
{
	wchar_t wstrDstPath[MAX_PATH] = L"";
	char	strDstPath[MAX_PATH];
	wchar_t wappname[64]=L"";
	char	dir[MAX_PATH];
	char	name[64];
	wchar_t wname[64];
	char	ext[16];

	if (_access(filepath, 0)) return 1; 

//	rex_check_result(mlpiSystemGetSpecialPath(_Connection, MLPI_PATH_PROJECTDATA_PLC, wstrDstPath, SIZEOF(wstrDstPath)));
//	wchar_to_char(wstrDstPath, strDstPath, SIZEOF(strDstPath));
	
	strcpy(strDstPath, "//ata0a//ProjectData//PLC//");
		
	split_path(filepath, dir, name, ext);

	if (_remote_diff(dir, name, strDstPath))
	{
		rex_message("Downloading %s", filepath);
	
		mlpiApiSetDefaultTimeout(10000);

		_app_reset(MLPI_RESET_ORIGIN);
		rex_check_result(mlpiApiConnect(_WIpAddr, &_Connection));
	
		_transfer(dir, name, ".app", strDstPath, TRUE);
		_transfer(dir, name, ".crc", strDstPath, TRUE);
	
		char_to_wchar(name, wname, SIZEOF(wname));
		rex_check_result(mlpiLogicLoadBootApplication(_Connection, wname, wstrDstPath, wappname, SIZEOF(wappname)));
		mlpiApiSetDefaultTimeout(MLPI_TIMEOUT);

		sys_reset_error();
		_app_reset(MLPI_RESET_COLD);
		app_start();
		rex_message("Download complete, Restarting", filepath);
		mlpiApiDisconnect(&_Connection);
		rex_connect(NULL, NULL, NULL, NULL);
		return REPLY_OK;
	}
	return 2;
}

//--- app_transfer_file ------------------------------------------------------
int app_transfer_file(const char *filepath, const char* destSubDir)
{
	wchar_t wstrDstPath[MAX_PATH] = L"";
	char	strDstPath[MAX_PATH];
	char	dir[MAX_PATH];
	char	name[64];
	char	ext[16];
	int		ret;

	if (_access(filepath, 0)) return 1;

	rex_check_result(mlpiSystemGetSpecialPath(_Connection, MLPI_PATH_OEM, wstrDstPath, SIZEOF(wstrDstPath)));
	wchar_to_char(wstrDstPath, strDstPath, SIZEOF(strDstPath));

	split_path(filepath, dir, name, ext);

	if (*destSubDir)
		sprintf(&strDstPath[strlen(strDstPath)], "%s/", destSubDir);
	ret = _transfer(dir, name, ext, strDstPath, FALSE);
	if (ret==4) return 2;
	if (ret==0) return REPLY_OK;
	return 3;
}

