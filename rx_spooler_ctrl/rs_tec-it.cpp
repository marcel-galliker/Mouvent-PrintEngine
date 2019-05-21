#include <stdio.h>
#include "rs_tec-it.h"
#include "tbarcode.h"

//--- tecit_version ------------------------------
void tecit_version(void)
{
	int major, minor, release, revision;
	const char *version;
	const char *date;

	BCGetLibraryVersionA(&major, &minor, &release, &revision, &version, &date);

	printf("TEC-IT Version: %d.%d.%d.%d  %s  %s\n",  major, minor, release, revision, version, date);
}