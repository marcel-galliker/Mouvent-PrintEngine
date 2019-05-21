// ****************************************************************************
//
//	rx_term.h
//
// ****************************************************************************
//
//	Copyright 2015 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#pragma once


#define PORT_TERM				777

#ifdef __cplusplus
extern "C"{
#endif

#define BLACK	0x000000
#define RED	    0xFF0000
#define GREEN	0x00FF00
#define BLUE	0x0000FF
#define YELLOW	0xFFFF00
#define GREY	0x808080
	
	
void term_printf(const char *format, ...); // printf Remote
void term_color (int rgb);
int  term_get_str(char *str, int size);
void term_flush(void);
void term_save(char *filepath);
	
#ifdef __cplusplus
}
#endif
