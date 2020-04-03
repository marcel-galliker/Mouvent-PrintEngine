// ****************************************************************************
//
//	hex2bin.c
//
// ****************************************************************************
//
//	Copyright 2015 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#include "rx_def.h"
#include "hex2bin.h"


//--- hexval --------------------
static UINT32 hexval(const char *str, int len)
{
	UINT32 val=0;

	while(len>0)
	{
		if (*str>='0' && *str<='9') val = 0x10*val + *str-'0';
		if (*str>='a' && *str<='f') val = 0x10*val + *str-'a'+10;
		if (*str>='A' && *str<='F') val = 0x10*val + *str-'A'+10;
		len--;
		str++;
	}

	return val;
}

//--- hex2bin ------------------------------------------------------------------
void hex2bin(const char *hexfilepath, BYTE *buffer, INT32 bufSize, INT32 *pusedsize)
{
	FILE	*file;
	int		line, len, i, n;
	char	str[256];
	BYTE	cnt, type;
	UINT32	addr;
	UINT32  segaddr=0;

	*pusedsize = 0;
	memset(buffer, 0, bufSize);
	file = fopen(hexfilepath, "rb");
	if (file)
	{
		line=0;
		while (fgets(str, sizeof(str), file))
		{
			line++;
		//	printf("%s\n", str);
			len = (int)strlen(str);
			if (str[0]!=':') { printf("Line[%d]: Error ':' missing", line); return;}
			cnt  = hexval(&str[1], 2);
			addr = 4*hexval(&str[3], 4);
			type  = hexval(&str[7], 2);
			if (type==0x01) break; 
			switch (type)
			{
			case 0x00:	for (i=9; cnt; i+=2, addr++, cnt--)
						{
							n = segaddr+addr;
							n = (n&~3) + 3-(n&3);
							if (n<bufSize) buffer[n]=hexval(&str[i],2);
							if (n>*pusedsize) *pusedsize=n;
						}
						break;	// Data Record
			case 0x01:	break;	// End of File Record
			case 0x02:	segaddr=16*hexval(&str[9], 4); break;	// Extended Segment Address Record
			default:	break;
			}
		}
		fclose(file);
		/*
		file = fopen("/tmp/test.bin", "wb");
		fwrite(buffer, 1, bufSize, file);
		fclose(file);
		*/
	}
}
