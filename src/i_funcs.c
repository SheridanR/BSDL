/*-------------------------------------------------------------------------------

	BUBBENSTEIN/SDL
	File: i_funcs.c
	Desc: contains various interface and input functions

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

	i_Message
	
	Prints a message onto the screen that disappears over time.

-------------------------------------------------------------------------------*/

void i_Message( char *fmt, ... )
{
	char str[100];
	va_list argptr;
	
	// format the string
	va_start( argptr, fmt );
	vsprintf( str, fmt, argptr );
	
	// move the string to the message string
	strcpy( message_str, str );
	message_time = 2500;
}

/*-------------------------------------------------------------------------------

	i_GetFrameRate
	
	Calculates the frame rate factor "timesync" by comparing the execution time
	between the current frame and the last.

-------------------------------------------------------------------------------*/

void i_GetFrameRate(void)
{
	// calculate frame rate
	t = SDL_GetTicks();
	timesync = (t - ot)/2;
	ot = t;
	fps = (int)(1/timesync*500);
}

/*-------------------------------------------------------------------------------

	i_ReceiveInput
	
	Polls various SDL events to collect keyboard and mouse input data.

-------------------------------------------------------------------------------*/

int numshots=0;
void i_ReceiveInput(void)
{
	char filename[24];
	char shots[8];
	SDL_Rect src, dest;
	SDL_Surface *temp;
	
	mousex = 0;
	mousey = 0;
	while( SDL_PollEvent(&event) ) // poll SDL events
	{
		// Global events
		switch( event.type )
		{
			case SDL_QUIT: // if SDL receives the shutdown signal
				gameloop = 0;
				break;
			case SDL_KEYDOWN: // if a key is pressed...
				keystatus[event.key.keysym.sym] = 1; // set this key's index to 1
				break;
			case SDL_KEYUP: // if a key is unpressed...
				keystatus[event.key.keysym.sym] = 0; // set this key's index to 0
				break;
			case SDL_MOUSEBUTTONDOWN: // if a mouse button is pressed...
				mousestatus[event.button.button] = 1; // set this mouse button to 1
				break;
			case SDL_MOUSEBUTTONUP: // if a mouse button is released...
				mousestatus[event.button.button] = 0; // set this mouse button to 0
				break;
			case SDL_MOUSEMOTION: // if the mouse is moved...
				mousex = event.motion.xrel;
				mousey = event.motion.yrel;
				break;
		}
	}
	
	// if ESC is pressed, close the game!
	if( keystatus[SDLK_ESCAPE] )
		gameloop = 0;
	
	// take screenshots
	if( keystatus[SDLK_F6] )
	{
		keystatus[SDLK_F6] = 0;
		
		strcpy( filename, "shots/shot" );
		sprintf( shots, "%d", numshots );
		strcat( filename, shots );
		strcat( filename, ".BMP" );
		
		src.x=0; dest.x=0;
		src.y=0; dest.y=0;
		src.w=xres; dest.w=0;
		src.h=yres; dest.h=0;
		
		temp = SDL_CreateRGBSurface(SDL_HWSURFACE,xres,yres,32,0,0,0,0);
		SDL_BlitSurface( screen2, &src, temp, &dest );
		SDL_SaveBMP( temp, filename );
		SDL_FreeSurface( temp );
		numshots++;
		
		i_Message( "Screenshot taken." );
	}
}