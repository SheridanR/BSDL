/*-------------------------------------------------------------------------------

	BUBBENSTEIN/SDL
	File: i_draw.c
	Desc: interface drawing functions

	Copyright 2013 (c) Sheridan Rathbun, all rights reserved.
	See LICENSE for details.

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

void i_PrintText( SDL_Surface *font_bmp, int x, int y, char *fmt, ... ) {
	int c;
	int numbytes;
	char str[100];
	va_list argptr;
	SDL_Rect src, dest, odest;
	
	// format the string
	va_start( argptr, fmt );
	numbytes = vsprintf( str, fmt, argptr );
	
	// define font dimensions
	dest.x = x;
	dest.y = y;
	dest.w = font_bmp->w/16; src.w = font_bmp->w/16;
	dest.h = font_bmp->h/16; src.h = font_bmp->h/16;
	
	// print the characters in the string
	for( c=0; c<numbytes; c++ ) {
		// edge of the screen prompts an automatic newline
		if( xres-dest.x < src.w ) {
			dest.x = x;
			dest.y += src.h;
		}
		
		src.x = (str[c]*src.w)%font_bmp->w;
		src.y = floor((str[c]*src.w)/font_bmp->w)*src.h;
		odest.x=dest.x; odest.y=dest.y;
		SDL_BlitSurface( font_bmp, &src, screen, &dest );
		dest.x=odest.x; dest.y=odest.y;
		switch( str[c] ) {
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

/*-------------------------------------------------------------------------------

	i_PrintMessages

	Prints a fading list of game messages at the top of the screen

-------------------------------------------------------------------------------*/

void i_PrintMessages(void) {
	if( message_time > 0 ) {
		i_PrintText( font16_bmp, 4, message_y/100, "%s", message_str );
		message_y += timesync*40;
		if( message_y > 400 )
			message_y = 400;
		message_time -= timesync;
	}
	else {
		i_PrintText( font16_bmp, 4, message_y/100, "%s", message_str );
		message_y -= timesync*40;
		if( message_y < -yres*100 )
			message_y = -yres*100;
	}
}