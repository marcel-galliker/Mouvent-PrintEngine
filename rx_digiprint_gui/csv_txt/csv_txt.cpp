// ****************************************************************************
//
//	CSV to TXT converter
//
// ****************************************************************************
//
//	This help software converts a ";" separated CSV language file
//	into one TXT file per language that can be processed by "resgen"
//
// ****************************************************************************
//
//	Copyright 2014 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#include <stdio.h>
#include <memory.h>

#pragma warning(disable:4996)

int convert(char *filename)
{
	FILE *src;
	FILE *dst[20];
	char  line[2048];
	char  *ch, *start;
	char  str[2048];
	int	  i;

	printf("convert >>%s<<\n", filename);

	memset(dst, 0, sizeof(dst));
	src = fopen(filename, "r");
	if (src!=NULL)
	{	
		//--- open destination files ---
		memset(line, 0, sizeof(line));
		if (fgets(line, sizeof(line), src))
		{
			for (ch=line, start=NULL, i=0; *ch; ch++)
			{
				if (*ch==';' || *ch<' ')
				{
					if (start)
					{
						*ch=0;
						if (*start)
						{
							sprintf(str, "%s.txt", start);
							dst[i++]=fopen(str, "w");
						}
					}
					start=++ch;
				}
			}
		}
		
		
		//--- read texts ---
		while(1) 
		{
			memset(line, 0, sizeof(line));
			if (!fgets(line, sizeof(line), src)) break;
			for (ch=line, start=NULL, i=0; *ch; ch++)
			{
				if (*ch==';' || *ch<' ')
				{
					*ch=0;
					if (start && *start)
					{
						if (dst[i]) fprintf(dst[i], "%s=%s\n", line, start);
						i++;
					}
					start=++ch;
				}
			}
		}

		//--- close files ---------------------------------
		for (i=0; dst[i]; i++)
		{
			fclose(dst[i]);
		}
		fclose(src);
		return 0;
	}
	
	printf("File >>%s<< could not be opened.", filename);
	return 1;
}

int main(int argc, char* argv[])
{	
	if (argc<2)
	{
		printf("Call: csv_txt <inputfilename>\n");
		return 0;
	}
		
	return convert(argv[1]);
}

