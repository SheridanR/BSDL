/*-------------------------------------------------------------------------------

	BUBBENSTEIN/SDL
	File: i_draw.c
	Desc: interface drawing functions

	Copyright 2011 (c) Sheridan Rathbun, all rights reserved.
	See LICENSE.TXT for details.

-------------------------------------------------------------------------------*/

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <malloc.h>
#include <time.h>
#include "bsdl.h"

/*-------------------------------------------------------------------------------

	i_PrintText

	Draws formatted text on the screen using characters from a font bitmap.

-------------------------------------------------------------------------------*/

void i_PrintText( SDL_Surface *font_bmp, int x, int y, char *fmt, ... )
{
	int c;
	int numbytes;
	char str[100];
	va_list argptr;
	SDL_Rect src, dest;
	
	// format the string
	va_start( argptr, fmt );
	numbytes = vsprintf( str, fmt, argptr );
	//for( c=0; str[c] != '\0'; c++ )
	//{
	//	if( str[c] > 96 && str[c] < 123 )
	//		str[c] -= 32;
	//}
	
	// define font dimensions
	dest.x = x;
	dest.y = y;
	dest.w = font_bmp->w/16; src.w = font_bmp->w/16;
	dest.h = font_bmp->h/16; src.h = font_bmp->h/16;
	
	// print the characters in the string
	for( c=0; c<numbytes; c++ )
	{
		src.x = (str[c]*src.w)%font_bmp->w;
		src.y = floor((str[c]*src.w)/font_bmp->w)*src.h;
		SDL_BlitSurface( font_bmp, &src, screen, &dest );
		switch( str[c] )
		{
			case 10: // line feed
				dest.x = x;
				dest.y += src.h;
				break;
			default:
				dest.x += src.w; // move over one character
				break;
		}
	}
	va_end( argptr );
}