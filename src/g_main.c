/*-------------------------------------------------------------------------------

	BUBBENSTEIN/SDL
	File: g_main.c
	Desc: main game source file. initializes engine and starts game.

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

	main

	Contains the main game loop.

-------------------------------------------------------------------------------*/

int main( int argc, char **argv ) {
	int a;
	char *maptoload = NULL;
	char *configtoload = NULL;
	SDL_Rect src, dest;
	
	// get a new random seed
	srand(time(0));
	
	// read arguments
	if( argc > 1 && argv[1] != NULL )
	{
		for(a=1; a<argc; a++) {
			if( argv[a] != NULL ) {
				// set windowed mode
				if( !strcmp(argv[a], "-windowed") ) {
					windowed = 1;
				}
				// set x resolution
				else if( !strncmp(argv[a], "-w", 2) ) {
					xres = max(atoi(argv[a]+2),320);
				}
				// set y resolution
				else if( !strncmp(argv[a], "-h", 2) ) {
					yres = max(atoi(argv[a]+2),200);
				}
				// set config file to read
				else if( !strncmp(argv[a], "-config=", 8) ) {
					configtoload = argv[a]+8;
				}
				// choose map to load
				else
					maptoload = argv[a];
			}
		}
	}
	vidsize = xres*yres;
	src.x = 0; dest.x = 0;
	src.y = 0; dest.y = 0;
	src.w = xres; dest.w = 0;
	src.h = yres; dest.h = 0;
	
	// read the config file
	if( configtoload == NULL)
		i_ReadConfig("default.cfg");
	else
		i_ReadConfig(configtoload);
	
	// start the engine
	if( maptoload == NULL )
		g_Open("testmap.bsm");
	else
		g_Open(maptoload);
	
	while(gameloop) {
		// run game loop functions
		i_GetFrameRate();
		i_ReceiveInput();
		e_Cycle();
		
		// rendering
		r_DrawSky( camang, vang );
		r_ClearBuffers();
		r_DrawColumns( camx, camy, camz, camang, vang );
		r_DrawFloors( camx, camy, camz, camang, vang );
		r_DrawSprites( camx, camy, camz, camang, vang );
		if( !thirdperson )
			r_DrawWeapons();
		
		// interface
		i_PrintMessages();
		i_PrintText( font8_bmp, 4, yres-20, "FPS: %d", fps ); // fps counter
		
		// blit screen to screen2 (faster than locking/unlocking screen)
		SDL_BlitSurface(screen, &src, screen2, &dest);
		
		// flip the page
		SDL_Flip( screen2 );
		cycles++;
	}
	
	g_Close(); // stop the engine
	return(0);
}
