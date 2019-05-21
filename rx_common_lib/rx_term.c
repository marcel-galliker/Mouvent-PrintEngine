// ****************************************************************************
//
//	rx_trace.c
//
// ****************************************************************************
//
//	Copyright 2015 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include "rx_common.h"
#include "rx_file.h"
#include "rx_sok.h"
#include "rx_term.h"
#include "rx_threads.h"
#include "tcp_ip.h"

#ifdef WIN32
#pragma warning( disable: 4996 )
#endif

//--- defines ---------------------------------------------------
#define BUF_SIZE (8*1024)

//--- global variables -----------------------------------------
static HANDLE	 _hTermServer	= NULL;
static BYTE		 *_Buffer       = NULL;
static int		 _BufSize		= 0;

//--- prototypes --------------------------------------------
static int _rgb2termcolor(int rgb);
static int _fontcolor(int rgb);

// color look-up table
static const int CLUT[] = {
    // Primary 3-bit (8 colors). Unique representation!
	0x000000,
	0x800000,
	0x008000,
	0x808000,
	0x000080,
	0x800080,
	0x008080,
	0xc0c0c0,

	    // Equivalent "bright" versions of original 8 colors.
	0x808080,
	0xff0000,
	0x00ff00,
	0xffff00,
	0x0000ff,
	0xff00ff,
	0x00ffff,
	0xffffff,

	    // Strictly ascending.
	0x000000,
	0x00005f,
	0x000087,
	0x0000af,
	0x0000d7,
	0x0000ff,
	0x005f00,
	0x005f5f,
	0x005f87,
	0x005faf,
	0x005fd7,
	0x005fff,
	0x008700,
	0x00875f,
	0x008787,
	0x0087af,
	0x0087d7,
	0x0087ff,
	0x00af00,
	0x00af5f,
	0x00af87,
	0x00afaf,
	0x00afd7,
	0x00afff,
	0x00d700,
	0x00d75f,
	0x00d787,
	0x00d7af,
	0x00d7d7,
	0x00d7ff,
	0x00ff00,
	0x00ff5f,
	0x00ff87,
	0x00ffaf,
	0x00ffd7,
	0x00ffff,
	0x5f0000,
	0x5f005f,
	0x5f0087,
	0x5f00af,
	0x5f00d7,
	0x5f00ff,
	0x5f5f00,
	0x5f5f5f,
	0x5f5f87,
	0x5f5faf,
	0x5f5fd7,
	0x5f5fff,
	0x5f8700,
	0x5f875f,
	0x5f8787,
	0x5f87af,
	0x5f87d7,
	0x5f87ff,
	0x5faf00,
	0x5faf5f,
	0x5faf87,
	0x5fafaf,
	0x5fafd7,
	0x5fafff,
	0x5fd700,
	0x5fd75f,
	0x5fd787,
	0x5fd7af,
	0x5fd7d7,
	0x5fd7ff,
	0x5fff00,
	0x5fff5f,
	0x5fff87,
	0x5fffaf,
	0x5fffd7,
	0x5fffff,
	0x870000,
	0x87005f,
	0x870087,
	0x8700af,
	0x8700d7,
	0x8700ff,
	0x875f00,
	0x875f5f,
	0x875f87,
	0x875faf,
	0x875fd7,
	0x875fff,
	0x878700,
	0x87875f,
	0x878787,
	0x8787af,
	0x8787d7,
	0x8787ff,
	0x87af00,
	0x87af5f,
	0x87af87,
	0x87afaf,
	0x87afd7,
	0x87afff,
	0x87d700,
	0x87d75f,
	0x87d787,
	0x87d7af,
	0x87d7d7,
	0x87d7ff,
	0x87ff00,
	0x87ff5f,
	0x87ff87,
	0x87ffaf,
	0x87ffd7,
	0x87ffff,
	0xaf0000,
	0xaf005f,
	0xaf0087,
	0xaf00af,
	0xaf00d7,
	0xaf00ff,
	0xaf5f00,
	0xaf5f5f,
	0xaf5f87,
	0xaf5faf,
	0xaf5fd7,
	0xaf5fff,
	0xaf8700,
	0xaf875f,
	0xaf8787,
	0xaf87af,
	0xaf87d7,
	0xaf87ff,
	0xafaf00,
	0xafaf5f,
	0xafaf87,
	0xafafaf,
	0xafafd7,
	0xafafff,
	0xafd700,
	0xafd75f,
	0xafd787,
	0xafd7af,
	0xafd7d7,
	0xafd7ff,
	0xafff00,
	0xafff5f,
	0xafff87,
	0xafffaf,
	0xafffd7,
	0xafffff,
	0xd70000,
	0xd7005f,
	0xd70087,
	0xd700af,
	0xd700d7,
	0xd700ff,
	0xd75f00,
	0xd75f5f,
	0xd75f87,
	0xd75faf,
	0xd75fd7,
	0xd75fff,
	0xd78700,
	0xd7875f,
	0xd78787,
	0xd787af,
	0xd787d7,
	0xd787ff,
	0xd7af00,
	0xd7af5f,
	0xd7af87,
	0xd7afaf,
	0xd7afd7,
	0xd7afff,
	0xd7d700,
	0xd7d75f,
	0xd7d787,
	0xd7d7af,
	0xd7d7d7,
	0xd7d7ff,
	0xd7ff00,
	0xd7ff5f,
	0xd7ff87,
	0xd7ffaf,
	0xd7ffd7,
	0xd7ffff,
	0xff0000,
	0xff005f,
	0xff0087,
	0xff00af,
	0xff00d7,
	0xff00ff,
	0xff5f00,
	0xff5f5f,
	0xff5f87,
	0xff5faf,
	0xff5fd7,
	0xff5fff,
	0xff8700,
	0xff875f,
	0xff8787,
	0xff87af,
	0xff87d7,
	0xff87ff,
	0xffaf00,
	0xffaf5f,
	0xffaf87,
	0xffafaf,
	0xffafd7,
	0xffafff,
	0xffd700,
	0xffd75f,
	0xffd787,
	0xffd7af,
	0xffd7d7,
	0xffd7ff,
	0xffff00,
	0xffff5f,
	0xffff87,
	0xffffaf,
	0xffffd7,
	0xffffff,

	    // Gray-scale range.
	0x080808,
	0x121212,
	0x1c1c1c,
	0x262626,
	0x303030,
	0x3a3a3a,
	0x444444,
	0x4e4e4e,
	0x585858,
	0x626262,
	0x6c6c6c,
	0x767676,
	0x808080,
	0x8a8a8a,
	0x949494,
	0x9e9e9e,
	0xa8a8a8,
	0xb2b2b2,
	0xbcbcbc,
	0xc6c6c6,
	0xd0d0d0,
	0xdadada,
	0xe4e4e4,
	0xeeeeee,
};


//--- term_printf ---------------------------------
void term_printf(const char *format, ...)
{
	va_list ap;
	int len, size;

	if (_hTermServer==NULL) sok_start_server(&_hTermServer, NULL, PORT_TERM, SOCK_STREAM, 5, NULL, NULL, NULL);
	if (_Buffer==NULL)
	{
		_Buffer=(BYTE*)malloc(BUF_SIZE);
		_BufSize = 0;
	}
	
	size =BUF_SIZE-_BufSize-1;
	if (size>0)
	{
		va_start(ap, format);
		len = vsnprintf(&_Buffer[_BufSize], size, format, ap);
		_BufSize += len;
		if (_BufSize>BUF_SIZE-256) term_flush();
		va_end(ap);
	}

//	sok_send_to_clients_telnet(_hTermServer, str, (int)strlen(str));
}

//--- _rgb2termcolor ------------------------------------------------
static int _rgb2termcolor(int rgbval)
{
	int i, c;
	int closest = 0;
	int res = 0, match = 0;
	int rgb[3] = { 0 };
	static const int incs[6] = { 0x00, 0x5f, 0x87, 0xaf, 0xd7, 0xff };

	rgb[0] = (rgbval >> 16) & 0xff;
	rgb[1] = (rgbval >>  8) & 0xff;
	rgb[2] = (rgbval >>  0) & 0xff;
	
	for (c = 0; c < 3; c++)
	{
		for (i = 0; i < 5; i++)
		{
			if (incs[i] <= rgb[c] && rgb[c] <= incs[i + 1])
			{
				if (abs(incs[i] - rgb[c]) < abs(incs[i + 1] - rgb[c]))
					closest = incs[i];
				else
					closest = incs[i + 1];

				res += (closest << (8 * (2 - c)));
			}
		}
	}

	for (i = 0; i < 256; i++)
	{
		if (CLUT[i] == res)
			match = i;
	}

	return match;
}

//--- _fontcolor -------------------------------------
static int _fontcolor(int rgb)
{
#define DARK_FONT 	30
#define LIGHT_FONT	39
	
	int r = (rgb >> 16) & 0xff;
	int g = (rgb >>  8) & 0xff;
	int b = (rgb >>  0) & 0xff;
	
	int d = 0;
	int a = 1000 - (299 * r + 587 * g + 114 * b) / 255;

//	printf("a = %d\n", a);
	
	if (a < 600)
		d = DARK_FONT;
	else
		d = LIGHT_FONT;

	return d;
}

//--- term_color -----------------------------------------------------
void term_color(int rgb)
{
	term_printf("\033[%dm", _fontcolor(rgb));
	term_printf("\033[48;5;%dm", _rgb2termcolor(rgb));
}

//--- term_get_str ------------------------------------------
int  term_get_str(char *str, int size)
{
	int len;
	if (sok_get_term_str(_hTermServer, str, size)) return TRUE;
	str[0]=getchar_nolock();
	if (str[0]) 
	{	
		// gets(&str[1]);
		fgets(&str[1], size , stdin);
		len = (int)strlen(str);
		while (len>0 && str[--len]=='\n') str[len]=0;
		return TRUE;
	}
	return FALSE;
}

//--- term_flush ------------------------------------------------
void term_flush(void)
{
	if (_BufSize)
	{
		printf("%s", _Buffer);
		sok_send_to_clients_telnet(_hTermServer, _Buffer, _BufSize);
		_BufSize=0;
	}
}

//--- term_save ---------------------------------------------------
void term_save(char *filepath)
{
	if (_BufSize)
	{
		FILE *file = rx_fopen(filepath, "wb", _SH_DENYNO);
		if (file) 
		{
			fputs((const char*)_Buffer, file);
			fclose(file);
		}
		_BufSize=0;		
	}
}
