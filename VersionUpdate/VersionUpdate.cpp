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

#pragma warning(disable:4996)

// *************************************************************************
// This accessory upgrades the version info of an application.
// The source file must use the following entries to be upgraded:
// *************************************************************************

//--- source file -------------------------------------------------------

//	static char *revisionStr = "$Revision: 81 $";

//	static int ver_major = 1;
//	static int ver_minor = 0;
//	static int ver_revision = 81;
//	static int ver_build = 3;
//	const char version[32]="1.0.81.3";

//--- END OF FILE ----------------

static int  _ver_major;
static int  _ver_minor;
static int  _ver_revision;

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

//--- write_txt_file ---------------------
void write_txt_file(const char *hdr_path, int major, int minor, int revision, int build)
{
	char path[256];
	int len;
	FILE *dst;

	strcpy(path, hdr_path);
	len =strlen(path);
	while (len && path[len]!='\\') 
		len--;
	if (len>0) strcpy(&path[len+1], "version.txt");
	else       strcpy(path, "version.txt");
	printf("txt file >>%s<<\n", path);
	dst = fopen(path, "w");
	if (dst)
	{
		fprintf(dst, "Version=%04d.%04d.%04d.%04d", major, minor, revision, build);
		fclose(dst);
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
	int	  build=0;

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
			if (ch=strstr(line, "version.h"))
			{
				get_version(versionPath, path, line);
			}
			else if (ch=strstr(line, "version[32]"))
			{
				int major, minor, revision, cnt;
				while (*ch!='"') ch++;
				ch++;
				cnt=sscanf(ch, "%d.%d.%d.%d", &major, &minor, &revision, &build);
				if (revision==_ver_revision) build++;
				else						 build=1;
				sprintf(line, "const char version[32]=\"%d.%d.%d.%d\";\n", _ver_major, _ver_minor, _ver_revision, build);
			}

			fputs(line, dst);
		}
		fclose(dst);	
		fclose(src);
		remove(path);
		rename(tempname, path);
		write_txt_file(path, _ver_major, _ver_minor, _ver_revision, build);
		return 0;
	}
	return 1;
}

//--- main ------------------------
int main(int argc, char* argv[])
{
	if (argc>1) return update(argv[1], argv[2]);
}

