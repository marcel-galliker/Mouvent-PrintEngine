// ****************************************************************************
//
//	Version Update
//
// ****************************************************************************
//
//	Copyright 2014 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctime>

static int  _ver_major=0;
static int  _ver_minor=0;
static int  _ver_revision=0;

#pragma warning(disable:4996)

//--- val -----
int val(char *str)
{
	while (*str )
	{
		if (*str>='0' && *str<='9') return atoi(str);
		str++;
	}
	return 0;
}

//--- get_version ---------------------------------
void get_version(char *versionPath, char *root, char *str)
{
	FILE *src;
	char path[_MAX_PATH];
	char line[2048];
	char *ch;
	int i;

	if (versionPath!=NULL)
	{
		strcpy(path, versionPath);
	}
	else
	{
		while (*str!='"') str++;
		str++;
		if (root[0]=='.') strcpy(path, str);
		else
		{
			strcpy(path, root);
			i=strlen(path);
			while (--i>=0 && path[i]!='\\' && path[i]!='/') path[i]=0;
			sprintf(&path[i], "/%s", str);
		}
		i=strlen(path);
		while(--i>0 && (path[i]<=' ' || path[i]=='"')) path[i]=0;
	}
	src = fopen(path, "r");
	if (src!=NULL)
	{
		while (fgets(line, sizeof(line), src))
		{
			if      (ch=strstr(line, "VER_MAJOR"))		_ver_major    = val(ch);
			else if (ch=strstr(line, "VER_MINOR"))		_ver_minor    = val(ch);
			else if (ch=strstr(line, "VER_REVISION"))	_ver_revision = val(ch);
		}
		fclose(src);
	}
}

//--- update ----------------------------------------
int update(char *path, char *versionPath)
{
	char tempname[256];
	FILE *src;
	FILE *dst;
	char  line[2048];
	char  *ch;
	int major, minor, revision, build;

	if (versionPath) get_version(versionPath, NULL, NULL);

	printf("update >>%s<<\n", path);

	src = fopen(path, "r");
	if (src!=NULL)
	{
		sprintf(tempname, "%s.tmp", path);
		dst = fopen(tempname, "w");
		if (!dst)
		{
			printf("cound not create file >>%s<<\n", tempname);
			return 1;
		}
		while (fgets(line, sizeof(line), src))
		{
			if (ch=strstr(line, "AssemblyFileVersion"))
			{
				sscanf(ch, "AssemblyFileVersion(\"%d.%d.%d.%d", &major, &minor, &revision, &build);
				if (revision==_ver_revision) build++;
				else						 build=1;
				sprintf(ch, "AssemblyFileVersion(\"%d.%d.%d.%d\")]\n", _ver_major, _ver_minor, _ver_revision, build);
			}
			else if (ch=strstr(line, "AssemblyVersion"))
			{
				sscanf(ch, "AssemblyVersion(\"%d.%d.%d.%d", &major, &minor, &revision, &build);
				if (revision==_ver_revision) build++;
				else						 build=1;
				sprintf(ch, "AssemblyVersion(\"%d.%d.%d.%d\")]\n", _ver_major, _ver_minor, _ver_revision, build);
			}
			else if (ch = strstr(line, "AssemblyInformationalVersion"))
			{
				time_t t = time(0);
				struct tm * timeStruct = localtime(&t);
				sprintf(ch, "AssemblyInformationalVersion(\"BuiltOn=%d.%d.%d\")]\n", timeStruct->tm_mday, timeStruct->tm_mon+1, timeStruct->tm_year+1900);
			}
			fputs(line, dst);
		}
		fclose(dst);	
		fclose(src);
		remove(path);
		rename(tempname, path);
		return 0;
	}
	return 1;
}

//--- main --------------------------------------
int main(int argc, char* argv[])
{
	//--- get application path ----
	char path[256];
	
	strcpy(path, argv[0]);

	if (argc!=3)
	{
		printf("Need path to AssemplyInfo.cs file as parameter!");
		return -1;
	}

	return update(argv[1], argv[2]);
}