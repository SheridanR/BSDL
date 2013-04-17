/*-------------------------------------------------------------------------------

	BUBBENSTEIN/SDL
	File: i_funcs.c
	Desc: contains various interface and input functions

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

	i_Message
	
	Prints a message onto the screen that disappears over time.

-------------------------------------------------------------------------------*/

void i_Message( char *fmt, ... ) {
	int c;
	int lines=1;
	char str[100];
	va_list argptr;
	
	// format the string
	va_start( argptr, fmt );
	vsprintf( str, fmt, argptr );
	
	// move the string to the message string
	strcpy( message_str, str );
	for( c=0; message_str[c]; c++ )
		if(message_str[c]==10) // line feed
			lines++;
	message_time = 2000;
	message_y = -font16_bmp->h*lines*10;
}

/*-------------------------------------------------------------------------------

	i_GetFrameRate
	
	Calculates the frame rate factor "timesync" by comparing the execution time
	between the current frame and the last.

-------------------------------------------------------------------------------*/

void i_GetFrameRate(void) {
	// calculate frame rate
	t = SDL_GetTicks();
	timesync = (t - ot)/2;
	ot = t;
	fps = (int)(1/timesync*500);
}

/*-------------------------------------------------------------------------------

	i_ReadConfig
	
	Reads the provided config file and executes the commands therein. Used
	primarily for keymapping. Return value represents number of errors in
	config file

-------------------------------------------------------------------------------*/

int i_ReadConfig(char *filename) {
	int errors = 0;
	char str[50];
	FILE *fp;
	
	if( strstr(filename,".cfg") == NULL )
		strcat(filename,".cfg");
	
	// open the config file
	if((fp = fopen(filename,"rb")) == NULL ) {
		g_Close();
		printf( "ERROR: config file '%s' does not exist\n\n", filename );
		exit(314);
	}
	
	// read commands from it
	while( fgets(str,50,fp) != NULL ) {
		if( str[0] != '#' ) // if this line is not a comment
		{
			// bind command
			if( !strncmp(&str[0], "bind", 4) ) {
				if( strstr(str,"IN_FORWARD") )
					in_commands[IN_FORWARD] = atoi(&str[5]);
				else if( strstr(str,"IN_LEFT") )
					in_commands[IN_LEFT] = atoi(&str[5]);
				else if( strstr(str,"IN_BACK") )
					in_commands[IN_BACK] = atoi(&str[5]);
				else if( strstr(str,"IN_RIGHT") )
					in_commands[IN_RIGHT] = atoi(&str[5]);
				else if( strstr(str,"IN_UP") )
					in_commands[IN_UP] = atoi(&str[5]);
				else if( strstr(str,"IN_DOWN") )
					in_commands[IN_DOWN] = atoi(&str[5]);
				else if( strstr(str,"IN_RUN") )
					in_commands[IN_RUN] = atoi(&str[5]);
				else if( strstr(str,"IN_JUMP") )
					in_commands[IN_JUMP] = atoi(&str[5]);
				else if( strstr(str,"IN_THIRDPERSON") )
					in_commands[IN_THIRDPERSON] = atoi(&str[5]);
				else if( strstr(str,"IN_PRINTINFO") )
					in_commands[IN_PRINTINFO] = atoi(&str[5]);
				else if( strstr(str,"IN_NOCLIP") )
					in_commands[IN_NOCLIP] = atoi(&str[5]);
				else if( strstr(str,"IN_TESTSOUND") )
					in_commands[IN_TESTSOUND] = atoi(&str[5]);
				else if( strstr(str,"IN_TESTMUSIC") )
					in_commands[IN_TESTMUSIC] = atoi(&str[5]);
				else if( strstr(str,"IN_CENTERVIEW") )
					in_commands[IN_CENTERVIEW] = atoi(&str[5]);
				else if( strstr(str,"IN_SPAWN") )
					in_commands[IN_SPAWN] = atoi(&str[5]);
				else if( strstr(str,"IN_KILL") )
					in_commands[IN_KILL] = atoi(&str[5]);
				else if( strstr(str,"IN_ATTACK") )
					in_commands[IN_ATTACK] = atoi(&str[5]);
				else
					errors++;
			}
			else
				errors++;
		}
	}
	fclose(fp);
	return errors;
}

/*-------------------------------------------------------------------------------

	i_GetStatus
	
	Returns the current status of a user command

-------------------------------------------------------------------------------*/

int i_GetStatus(int command) {
	if( in_commands[command] >= 0 ) { // keyboard scancode
		return keystatus[in_commands[command]];
	} else { // other devices (i.e. mouse)
		return mousestatus[in_commands[command]*-1];
	}
}

/*-------------------------------------------------------------------------------

	i_ReceiveInput
	
	Polls various SDL events to collect keyboard and mouse input data.

-------------------------------------------------------------------------------*/

int numshots=0;
void i_ReceiveInput(void) {
	char filename[24];
	char shots[8];
	SDL_Rect src, dest;
	SDL_Surface *temp;
	
	mousex = 0;
	mousey = 0;
	while( SDL_PollEvent(&event) ) { // poll SDL events
		// Global events
		switch( event.type ) {
			case SDL_QUIT: // if SDL receives the shutdown signal
				gameloop = 0;
				break;
			case SDL_KEYDOWN: // if a key is pressed...
				keystatus[event.key.keysym.scancode] = 1; // set this key's index to 1
				break;
			case SDL_KEYUP: // if a key is unpressed...
				keystatus[event.key.keysym.scancode] = 0; // set this key's index to 0
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
	if( keystatus[1] )
		gameloop = 0;
	
	// take screenshots
	if( keystatus[64] && in_toggle7 ) {
		in_toggle7=0;
		
		strcpy( filename, "shots/shot" );
		sprintf( shots, "%d", numshots );
		strcat( filename, shots );
		strcat( filename, ".BMP" );
		
		src.x=0; dest.x=0;
		src.y=0; dest.y=0;
		src.w=xres; dest.w=0;
		src.h=yres; dest.h=0;
		
		temp = SDL_CreateRGBSurface(SDL_HWSURFACE,xres,yres,32,0,0,0,0);
		SDL_BlitSurface( screen, &src, temp, &dest );
		SDL_SaveBMP( temp, filename );
		SDL_FreeSurface( temp );
		numshots++;
		
		i_Message( "Screenshot taken." );
	} else if( !keystatus[64] )
		in_toggle7=1;
}