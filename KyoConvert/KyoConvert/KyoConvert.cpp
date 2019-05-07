// KyoConvert.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define DST_FNAME   "D:\\Projekte\\Kyocera\\1200 DPI\\1200dpi Nozzle-Data list.txt"
#define SRC_FNAME_1 "D:\\Projekte\\Kyocera\\1200 DPI\\1200dpi Nozzle-Data table 1.txt"
#define SRC_FNAME_2 "D:\\Projekte\\Kyocera\\1200 DPI\\1200dpi Nozzle-Data table 2.txt"

int val(char *str, int pos)
{
	int n=0;
	char *end;
	char ch=0;

	while(pos)
	{
		if (*str=='\t') pos--;
		str++;
	}
	for(end=str; *end; end++)
	{
		if (*end=='\t') 
		{
			ch = *end;
			*end=0;
			break;
		}
	}

	sscanf(str, "%d\t", &n);
	if (n==0) sscanf(str, "Y+%d", &n);
	*end=ch;
	return n;
}

void add_nozzles(FILE *dst, char *fname, int reg0)
{
	FILE	*src;
	int		reg, i, n, col, skip;
	char  line[2048];
	char  bitStr[2048];
	char  xStr[2048];
	char  yStr[2048];

	int	bit, x, y;

	src = fopen(fname, "r");
	if (src)
	{
		for (reg=0, col=3; reg<10; reg++)
		{
			fseek(src, 0, SEEK_SET);
			//--- skip title ---
			if (reg<5) skip = 3;
			else skip=96;
			col = 3+19*(reg%5);

			for (i=0; i<skip; i++) fgets(line, sizeof(line), src);

			for (i=0; i<21; i++)
			{
				fgets(bitStr, sizeof(bitStr), src);
				fgets(xStr, sizeof(xStr), src);
				fgets(yStr, sizeof(yStr), src);
				fgets(line, sizeof(line), src);

				for (n=0; n<16; n++)
				{
					bit = val(bitStr, col+n);
					x   = val(xStr,   col+n);
					y   = val(yStr,   col+n);
					if (bit) fprintf(dst, "%d\t%d\t%d\t%d\n", x, y, reg0+reg, bit);
				}
			}
		}
		fclose(src);
	}
}

int main(int argc, char* argv[])
{
	FILE *dst;

	dst = fopen(DST_FNAME, "w");
	if (dst)
	{
		fprintf(dst, "X\tY\tReg\tBit\n");
		add_nozzles(dst, SRC_FNAME_1, 1);
		add_nozzles(dst, SRC_FNAME_2, 11);
		fclose(dst);
	}
		
	return 0;
}

