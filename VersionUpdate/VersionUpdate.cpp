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

//	const char version[32]="1.0.81.3";

//--- END OF FILE ----------------

static int _ver_major;
static int _ver_minor;
static int _ver_revision;
static int _ver_build = -1;


//--- get_version ---------------------------------
void get_version()
{
    char psBuffer[128];
    FILE *pipe =
        _popen("git describe --match \"v[0-9]*.[0-9]*\"", "rt");

	fgets(psBuffer, 128, pipe);

	sscanf(psBuffer, "v%d.%d-%d", &_ver_major, &_ver_minor, &_ver_revision);
    char* buildid=getenv("BUILD_BUILDID");
    if (buildid != NULL)
    {
        sscanf(buildid, "%d", &_ver_build);
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
int update(char *path)
{
	FILE *src;
	FILE *dst;
	char  line[2048];
	int	  build=0;

	printf("update >>%s<<\n", path);

	get_version();
    if (_ver_build == -1)
    {
        src = fopen(path, "rt");
        _ver_build = 0;
        if (src != NULL)
        {
            int major, minor, revision, build;
            fgets(line, sizeof(line), src);
            sscanf(line, "const char version[32]=\"%d.%d.%d.%d\";\n", &major, &minor, &revision, &build);
            if (revision == _ver_revision) _ver_build = build + 1;
            fclose(src);
        }
    }
        
	dst = fopen(path, "wt");
	if (!dst)
	{
		printf("cound not create file >>%s<<\n", path);
		return 1;
	}

	sprintf(line, "const char version[32]=\"%d.%d.%d.%d\";\n", _ver_major,
            _ver_minor, _ver_revision, _ver_build);
    puts(line);
	fputs(line, dst);
	fclose(dst);	
	write_txt_file(path, _ver_major, _ver_minor, _ver_revision, _ver_build);
	return 0;
}

//--- main ------------------------
int main(int argc, char* argv[])
{
	if (argc>1) return update(argv[1]);
}

