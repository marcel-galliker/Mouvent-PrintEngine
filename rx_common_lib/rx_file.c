// ****************************************************************************
//
//	rx_file.c
//
// ****************************************************************************
//
//	Copyright 2014 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#include <stdio.h>
#ifdef linux
	#include <stdlib.h>
	#include <unistd.h>
	#include <sys/stat.h>
	#include <sys/types.h>
	#include <sys/times.h>
	#include <string.h>
	#include <utime.h>
	#include <time.h>
	#include <errno.h>
#else
	#include <io.h>
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <sys/utime.h>
	#include <direct.h>
	#include <time.h>
#endif

#include "rx_error.h"
#include "rx_file.h"


#ifdef WIN32
int rx_file_exists(const char *path)
{
	return (_access(path, 0x00)==0);
}

int rx_dir_exists(const char *path)
{
	WIN32_FILE_ATTRIBUTE_DATA type;
	if (GetFileAttributesExA(path, GetFileExInfoStandard, &type))
	if (type.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		return 1;
	return 0;
}

//--- struct SDir ---------------------------------
typedef struct
{
	HANDLE	hFind;
	char	match[256];
} SSearchWin;

//--- rx_search_open ---------------------------------
SEARCH_Handle rx_search_open(const char *path, const char *match)
{
	SSearchWin *psearch = (SSearchWin*)malloc(sizeof(SSearchWin));
	memset(psearch, 0, sizeof(SSearchWin));
	sprintf(psearch->match, "%s/%s", path, match);
	return (SEARCH_Handle) psearch;
}

//--- rx_search_next ----------------------------------
int rx_search_next(SEARCH_Handle hsearch, char *filename, int size, UINT64 *writeTime, UINT32 *filesize, UINT32 *isdir)
{
	SSearchWin *psearch = (SSearchWin*)hsearch;
	WIN32_FIND_DATA ffd;
	memset(&ffd, 0, sizeof(ffd));

	if (hsearch!=NULL)
	{
		if (psearch->hFind==NULL)		
			psearch->hFind = FindFirstFile(psearch->match, &ffd);
		else if (psearch->hFind!=INVALID_HANDLE_VALUE)	
			FindNextFile(psearch->hFind, &ffd);
		strncpy(filename, ffd.cFileName, size);
		if (writeTime) *writeTime = FiletimeToTimet(&ffd.ftLastWriteTime);
		if (filesize)  *filesize  = ffd.nFileSizeLow;
		if (isdir)	   *isdir     = (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)==FILE_ATTRIBUTE_DIRECTORY;
	}
	return *filename!=0;
}

//--- rx_search_close --------------------------------
int rx_search_close(SEARCH_Handle  hsearch)
{
	SSearchWin *psearch = (SSearchWin*)hsearch;
	if (psearch!=NULL) FindClose(psearch->hFind);
	free(psearch);
	return 0;
}

//--- rx_file_get_mtime ---------------------------------------------------------
time_t rx_file_get_mtime (const char *path)
{
	struct __stat64 st;
	if(_stat64(path, &st)) return 0;
	return st.st_mtime;
}


//--- rx_file_del_if_older ---------------------------------------
int rx_file_del_if_older(const char *path, const char *comparePath)
{
	time_t compareTime = rx_file_get_mtime(comparePath);
	time_t time		   = rx_file_get_mtime(path);
	if (time && (!compareTime || time<compareTime))
	{
		remove(path);
		return TRUE;
	}
	return FALSE;
}

//--- rx_file_set_mtime ---------------------------------------------
void rx_file_set_mtime (const char *path, time_t time)
{
	struct __utimbuf64 t;
	struct __stat64 st;

	_stat64(path, &st);
	
	t.modtime = time;
	t.actime  = st.st_atime;
		
	_utime64(path, &t);
}

//--- rx_file_get_size ------------------------------------
INT64 rx_file_get_size(const char *path)
{
	struct __stat64 st;
	if (_stat64(path, &st)) return 0;
	return st.st_size;
}

//--- rx_file_seek -----------------------------------
INT64	rx_file_seek	  (FILE *file, INT64 offset, int origin)
{
	return _fseeki64(file, offset, origin);
}
#endif

#ifdef linux
int rx_file_exists(const char *path)
{
	return (access( path, F_OK ) != -1);
}

int rx_dir_exists(const char *path)
{
	struct stat stat;
	if (lstat(path, &stat) == 0) {
		return (S_ISDIR(stat.st_mode)); 

	}
	return 0;
}

//--- struct SDir ---------------------------------
typedef struct
{
	DIR		*handle;
	char	path[MAX_PATH];
	char	match[MAX_PATH];
} SSearchLx;

//--- rx_search_open ---------------------------------
SEARCH_Handle rx_search_open (const char *path, const char *match)
{
	SSearchLx *psearch;

	psearch = (SSearchLx*)malloc(sizeof(SSearchLx));
	strncpy(psearch->path,  path,  sizeof(psearch->path));
	strncpy(psearch->match, match, sizeof(psearch->match));
	psearch->handle = opendir(path);
	if  (psearch->handle==NULL)
	{
		int err=errno;
		char str[128];
		err_system_error(err, str, sizeof(str));
		printf("errno=%d: %s\n", err, str);
	}
	return (SEARCH_Handle) psearch;
}

//--- rx_search_next ----------------------------------
int	rx_search_next (SEARCH_Handle hsearch, char *filename, int size, UINT64 *writeTime, UINT32 *filesize, UINT32 *isdir)
{
	if (hsearch!=NULL)
	{
		SSearchLx *psearch = (SSearchLx*)hsearch;
		struct dirent	*item;
		struct stat 	info;
		int				dir=FALSE;
		char			path[MAX_PATH];
		if (psearch->handle!=NULL)
		{
			while ((item=readdir(psearch->handle))!=NULL)
			{
				if(rx_fnmatch(psearch->match, item->d_name))
				{
					strncpy(filename, item->d_name, size);
					sprintf(path, "%s/%s", psearch->path, item->d_name);
					if (/*item->d_name[0]!='.'&& */ stat(path, &info)==0)
					{
						if (writeTime)	memcpy(writeTime, &info.st_mtim, sizeof(*writeTime));
						if (filesize)	*filesize = info.st_size;
						if (isdir) *isdir = (info.st_mode&0x4000)==0x4000;
					}
					return (*filename!=0);
				}
			}
		}
	}
	return 0;
}

//--- rx_search_close --------------------------------
int	rx_search_close(SEARCH_Handle hsearch)
{
	if (hsearch!=NULL)
	{
		SSearchLx *psearch = (SSearchLx*)hsearch;
		int ret = closedir(psearch->handle);
		free(psearch);
		return ret;
	}
	return 0;
}

//--- rx_file_get_mtime ---------------------------------------------------------
time_t rx_file_get_mtime (const char *path)
{
	struct stat st;
	if(stat(path, &st)) return 0;
	return st.st_mtime;
}

//--- rx_file_del_if_older ---------------------------------------
int rx_file_del_if_older(const char *path, const char *comparePath)
{
	time_t compareTime = rx_file_get_mtime(comparePath);
	time_t time		   = rx_file_get_mtime(path);
	if (time && (!compareTime || time<compareTime))
	{
		remove(path);
		return TRUE;
	}
	return FALSE;
}

//--- rx_file_set_mtime ---------------------------------------------
void rx_file_set_mtime (const char *path, time_t time)
{
	struct utimbuf t;
	struct stat st;

	stat(path, &st);
	
	t.modtime = time;
	t.actime  = st.st_atime;
		
	utime(path, &t);
}

//--- rx_file_get_size ------------------------------------
INT64 rx_file_get_size(const char *path)
{
	struct stat st;
	if (stat(path, &st)) return 0;
	return st.st_size;
}

INT64	rx_file_seek	  (FILE *file, INT64 offset, int origin)
{
	return fseek(file, offset, origin);	// not 64 Bit!!!
}

#endif

//--- rx_fnmatch ------------------------------------------
int	rx_fnmatch(const char * pmatch, const char *pstr)
{
	const char *restartStr=NULL;
	const char *restartMatch=NULL;
	char smatch[MAX_PATH];
	char sstr[MAX_PATH];

	strcpy(smatch, pmatch);
	strcpy(sstr, pstr);
	char_to_lower(smatch, smatch);
	char_to_lower(sstr, sstr);

	const char *match = smatch;
	const char *str	  = sstr;

	while (*match && *str)
	{
		if (*match=='*')
		{
			restartMatch = match++;
			while (*str && *str!=*match) 
				str++;
			if (*str) restartStr=str+1;
		}
		else if (*match==*str) 
		{
			match++;
			str++;
		}
		else if (restartStr)
		{
			str=restartStr;
			match=restartMatch;
			restartStr=NULL;		
		}
		else return FALSE;
	}
	if (*str || *match) return FALSE;
	return TRUE;
}

//--- rx_mkdir ------------------------------------
#ifdef linux
	int rx_mkdir(const char *dirname)
	{
		int ret;
		mode_t process_mask = umask(0);
		if (ret=mkdir(dirname, S_IRWXU | S_IRWXG | S_IRWXO))
		{
			if (errno!=EEXIST) printf("Error %d: %s\n", errno, strerror(errno));
		}
//		printf("mkdir >>%s<< ret=%d, errno=%d, str=>>%s<<\n", dirname, ret, errno, strerror(errno));
		umask(process_mask);
	}
#else
	int rx_mkdir(const char *dirname)
	{
		return _mkdir(dirname);
	}
#endif

//--- rx_mkdir_path --------------------------
void rx_mkdir_path(const char *dirname)
{
	char *ch, old;
	char dir[MAX_PATH];
	
	if (!*dirname) return;

	strcpy(dir, dirname);
	for (ch=&dir[1]; *ch; ch++)
	{
		if (*ch=='/' || *ch=='\\')
		{
			old = *ch;
			*ch=0;
			rx_mkdir(dir);
			*ch=old;
		}
	}
}

//--- rx_rmdir -------------------------------------------
int		rx_rmdir(const char *dirname)
{
	#ifdef linux
		return rmdir(dirname);
	#else
		return _rmdir(dirname);
	#endif
}

//--- rx_fopen --------------------------------------------------------------
FILE * rx_fopen(const char * path, const char * mode, int sharemode)
{
#ifdef linux
	return fopen(path, mode);
#else
	if (sharemode)	return _fsopen(path, mode, sharemode);
	else			return fopen(path, mode);
#endif
}

//--- remove_old_files --------------------------------------
#ifdef linux
int rx_remove_old_files(const char *searchStr, int days)
{
	int no=0;

	return no;
}
#else
int rx_remove_old_files(const char *searchStr, int days)
{
	__time64_t		time;
	ULONGLONG		ltime, lftime, diff, day;
	struct	tm		t;
	int				no, n;
	HANDLE			hFind;
	WIN32_FIND_DATA findFileData;
	char			str[MAX_PATH], path[MAX_PATH];

	_time64(&time);
	TimetToFileTime(time, (FILETIME*)&ltime);
	day = (ULONGLONG)10000000 * 24*60*60;
	ltime = (ltime/day)*day;
	_localtime64_s(&t, &time); 

	strcpy(path, searchStr);
	n=(int)strlen(path)-1;
	while (n>0)
	{
		if (path[n]=='\\' || path[n]=='/') break;
		path[n--]=0;
	}

	hFind = FindFirstFile(searchStr, &findFileData);
	no = -1;

	if (hFind!=INVALID_HANDLE_VALUE) 
	{
		do
		{
			memcpy(&lftime, &findFileData.ftLastWriteTime, sizeof(lftime));
			lftime = (lftime/day)*day;
			diff = (ltime-lftime) / day;
			if (diff>days || days==0) // delete older files
			{
				sprintf(str, "%s%s", path, findFileData.cFileName);
				DeleteFile(str);
			}
			else if (diff==0)
			{
				// n = atoi_tab(&findFileData.cFileName[wcslen(findFileData.cFileName)-7], NULL);
				n = atoi(&findFileData.cFileName[strlen(findFileData.cFileName)-7]);
				if (n<0) n=0;
				if (n>no) no=n;
			}
		} 
		while (FindNextFile(hFind, &findFileData));
		FindClose(hFind);
	}

	return no;
}
#endif

//--- rx_file_mount -------------------------------------------------------------------
#ifdef linux
int rx_file_mount(const char *remotePath, const char *mntPath, const char *user, const char *pwd)
{
	#define OUTPUT "/tmp/mount"
	if (*mntPath)
	{		
		// Giving access right in ubuntu:
		// > sudo visudo
		// >> add line "radex ALL = NOPASSWD: /bin/mount"
		// >> add line "radex ALL = NOPASSWD: /bin/mkdir"
		
		char command[2048];
		int ret;

		//--- check wether already mounted ---
		{
			FILE* fp;
			char str[100];
			fp = popen("mount -l -tcifs", "r");
			while (!feof(fp))
			{
				fgets(str, 100, fp);
				// printf("%s\n", str);
				if (str_start(str, remotePath)) 
				{
					// TrPrintfL(TRUE, "sr_mnt_drive: already mounted: end");
					pclose(fp);
					return REPLY_OK;
				}
			}
			pclose(fp);
		}

		if (access(mntPath, 0))
		{
			sprintf(command, "mkdir -m777 %s", mntPath); 
			ret=system(command);
		}

		sprintf(command, "mount -t cifs %s %s -o user=%s,password=%s", remotePath, mntPath, user, pwd); 
		if (system(command)==0) return REPLY_OK;
		sprintf(command, "umount %s", mntPath); 
		return REPLY_ERROR;
	}
}
#else

int rx_file_mount(const char *remotePath, const char *mntPath, const char *user, const char *pwd)
{
	int i=0;
	i=1/i;
	return REPLY_ERROR;
}
#endif
