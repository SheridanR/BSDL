/*-------------------------------------------------------------------------------
	
	BUBBENSTEIN/SDL
	File: g_data.c
	Desc: contains various functions used to open and close the engine.
	
	Copyright 2013 (c) Sheridan Rathbun, all rights reserved.
	See LICENSE for details.
	
-------------------------------------------------------------------------------*/

#include "bsdl.h"

/*-------------------------------------------------------------------------------
	
	g_Open
	
	Initializes several variables and loads several files the engine needs.
	
-------------------------------------------------------------------------------*/

int g_Open( char *file ) {
	char c; int i; float f;
	float screenfactor;
	unsigned long x, y;
	FILE *fp;
	char name[32];
	char valid_data[10];
	Mix_Chunk *psize;

	gameloop = 1;
	hx=xres>>1; hy=yres>>1; hz = hx;
	
	screenfactor = ((float)(xres))/320.0;
	//SDL_Rect src, dest;
	//src.x=0; dest.x=0;
	//src.y=0; dest.y=0;
	//src.w=1280*screenfactor; dest.w=0;
	//src.h=468*screenfactor; dest.h=0;
	
	// allocate memory for tables
	zbuffer = (float *) malloc(sizeof(f)*yres*xres);
	floorbuffer = (int *) malloc(sizeof(i)*yres*xres*2);
	floorbuffer_s = (int *) malloc(sizeof(i)*yres*xres*2);
	rowbuffer = (char *) malloc(sizeof(c)*yres);

	// build some tables
	map.loaded = 0;
	for( x=0; x<xres; x++ ) {
		for( y=0; y<yres; y++ ) {
			floorbuffer_s[y+x*yres]=16383;
			floorbuffer_s[y+x*yres+vidsize]=-1;
		}
	}

	// load general bitmaps
	font8_bmp = SDL_LoadBMP("images/8font.bmp");
	SDL_SetColorKey( font8_bmp, SDL_SRCCOLORKEY, SDL_MapRGB( font8_bmp->format, 255, 0, 255 ) );
	font16_bmp = SDL_LoadBMP("images/16font.bmp");
	SDL_SetColorKey( font16_bmp, SDL_SRCCOLORKEY, SDL_MapRGB( font16_bmp->format, 255, 0, 255 ) );
	r_LoadBmp("images/console.bmp", &console_bmp);
	
	// load textures
	sky2_bmp = SDL_LoadBMP("images/sky.bmp");
	sky_bmp = r_ScaleSurface(sky2_bmp, 1280*screenfactor,468*screenfactor);
	//sky_bmp = SDL_CreateRGBSurface(SDL_HWSURFACE,1280*screenfactor,468*screenfactor,32,0,0,0,0);
	//SDL_BlitSurface( SPG_Scale( sky2_bmp, screenfactor, screenfactor ), &src, sky_bmp, &dest );
	//SDL_BlitSurface( sky2_bmp, &src, sky_bmp, &dest );
	//SDL_BlitSurface( sky2_bmp, &src, sky_bmp, &dest );
	fp = fopen("images/textures.txt","r");
	for( texture_num=0; !feof(fp); texture_num++ ) {
		while( fgetc(fp) != '\n' ) if( feof(fp) ) break;
	}
	fclose(fp);
	walltex_bmp = (bitmap_t *) malloc(sizeof(bitmap_t)*texture_num);
	fp = fopen("images/textures.txt","r");
	for( x=0; !feof(fp); x++ ) {
		fscanf(fp,"%s",name); while( fgetc(fp) != '\n' ) if( feof(fp) ) break;
		r_LoadBmp(name, &walltex_bmp[x]);
	}
	fclose(fp);
	
	// load sprites
	fp = fopen("images/sprites.txt","r");
	for( sprite_num=0; !feof(fp); sprite_num++ ) {
		while( fgetc(fp) != '\n' ) if( feof(fp) ) break;
	}
	fclose(fp);
	sprite_bmp = (bitmap_t *) malloc(sizeof(bitmap_t)*sprite_num);
	fp = fopen("images/sprites.txt","r");
	for( x=0; !feof(fp); x++ ) {
		fscanf(fp,"%s",name); while( fgetc(fp) != '\n' ) if( feof(fp) ) break;
		r_LoadBmp(name, &sprite_bmp[x]);
	}
	fclose(fp);
	
	// load weapon bitmaps
	r_LoadBmp( "images/pistol1.bmp", &pistol_bmp[0] );
	r_LoadBmp( "images/pistol2.bmp", &pistol_bmp[1] );
	r_LoadBmp( "images/pistol3.bmp", &pistol_bmp[2] );
	r_LoadBmp( "images/pistol4.bmp", &pistol_bmp[3] );
	r_LoadBmp( "images/pistol5.bmp", &pistol_bmp[4] );
	r_LoadBmp( "images/shotgun1.bmp", &shotgun_bmp[0] );
	r_LoadBmp( "images/shotgun3.bmp", &shotgun_bmp[1] );
	shotgun_bmp[2] = shotgun_bmp[0];
	shotgun_bmp[3] = shotgun_bmp[0];
	shotgun_bmp[4] = shotgun_bmp[0];
	shotgun_bmp[5] = shotgun_bmp[0];
	r_LoadBmp( "images/shotgun4.bmp", &shotgun_bmp[6] );
	shotgun_bmp[7] = shotgun_bmp[6];
	r_LoadBmp( "images/shotgun5.bmp", &shotgun_bmp[8] );
	shotgun_bmp[9] = shotgun_bmp[6];
	shotgun_bmp[10] = shotgun_bmp[6];
	r_LoadBmp( "images/shotgun2.bmp", &shotgun_bmp[11] ); // swapped with shotgun[1] when needed

	// precompute some things
	sprsize = 2.45*((double)yres/240);
	texsize = 2.5*((double)yres/240);

	// load a map
	if( strstr(file,".bsm") == NULL )
		strcat(file,".bsm");
	fp = fopen(file, "rb");
	if( fp == NULL ) {
		printf( "ERROR: Could not load map file: %s\n\n", file );
		g_Close();
		exit(15);
	}
	
	// validate the file
	fread(valid_data, sizeof(char), strlen("BSDLMAP"), fp);
	if( strncmp(valid_data,"BSDLMAP",7) || fgetc(fp) != MAPVERSION ) {
		printf( "ERROR: Not a valid map file: %s\n\n", file );
		fclose(fp);
		g_Close();
		exit(46);
	}
	else
		map.loaded = 1; // lets the engine know that there's junk to be freed
	
	// header
	fread(map.name, sizeof(char), 32, fp);   // map name
	fread(map.author, sizeof(char), 32, fp); // map author
	fread(&map.width, sizeof(int), 1, fp);    // map width
	fread(&map.height, sizeof(int), 1, fp);   // map height
	
	// allocate data
	map.floors = (int *) malloc(sizeof(int)*map.width*map.height);
	map.floors_tex = (int *) malloc(sizeof(int)*map.width*map.height);
	map.floors_tex2 = (int *) malloc(sizeof(int)*map.width*map.height);
	map.ceilings = (int *) malloc(sizeof(int)*map.width*map.height);
	map.ceilings_tex = (int *) malloc(sizeof(int)*map.width*map.height);
	map.ceilings_tex2 = (int *) malloc(sizeof(int)*map.width*map.height);
	
	// map data
	fread(map.floors, sizeof(int), map.width*map.height, fp);        // floor height
	fread(map.floors_tex, sizeof(int), map.width*map.height, fp);    // lower wall texture
	fread(map.floors_tex2, sizeof(int), map.width*map.height, fp);   // floor texture
	fread(map.ceilings, sizeof(int), map.width*map.height, fp);      // ceiling height
	fread(map.ceilings_tex, sizeof(int), map.width*map.height, fp);  // upper wall texture
	fread(map.ceilings_tex2, sizeof(int), map.width*map.height, fp); // ceiling texture
	
	// close the file
	fclose(fp);
	
	// spawn the player
	if( !server ) {
		e_CreateEntity();
		player = lastentity;
		player->behavior = &e_ActPlayer;
		
		player->sizex = .5;
		player->sizey = .5;
		player->sizez = 52;
		
		player->x=2.5;
		player->y=2.5;
		player->z=0;
		player->ang=0;
	}
	vang=0;
	bob1 = 0; bob2 = 0; bob3 = 1;
	weap_mag[2]=2;
	
	// set camera
	camx=8;
	camy=2.5;
	camz=0;
	camang=0;

	// initiate SDL
	if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER ) == -1 ) {
		printf("ERROR: Could not initialize SDL. Aborting...\n\n");
		g_Close();
		exit(4);
	}
	//SDL_WM_GrabInput(SDL_GRAB_ON);
	
	// open audio
	if(Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers)) {
		printf("Unable to open audio!\n");
		exit(1);
	}

	// create a screen surface
	screen = SDL_CreateRGBSurface(SDL_HWSURFACE,xres,yres,32,0,0,0,0);
	if( !windowed )
		screen2 = SDL_SetVideoMode( xres, yres, 32, SDL_HWSURFACE | SDL_FULLSCREEN );
	else
		screen2 = SDL_SetVideoMode( xres, yres, 32, SDL_HWSURFACE );
	if( screen == NULL || screen2 == NULL ) {
		printf("ERROR: Could not create video surface. Aborting...\n\n");
		g_Close();
		exit(5);
	}
	SDL_WM_SetCaption( "Bubbenstein/SDL\n\n", 0 );
	SDL_ShowCursor(SDL_DISABLE);
	
	// reset the clock
	ot=SDL_GetTicks();
	i_GetFrameRate();
	
	// load sound effects
	fp = fopen("sound/sounds.txt","r");
	for( sound_num=0; !feof(fp); sound_num++ ) {
		while( fgetc(fp) != '\n' ) if( feof(fp) ) break;
	}
	fclose(fp);
	sounds = (Mix_Chunk **) malloc(sizeof(psize)*sound_num);
	fp = fopen("sound/sounds.txt","r");
	for( x=0; !feof(fp); x++ ) {
		fscanf(fp,"%s",name); while( fgetc(fp) != '\n' ) if( feof(fp) ) break;
		sounds[x] = Mix_LoadWAV(name);
	}
	fclose(fp);
	
	// load music
	music = Mix_LoadMUS("music/dead.ogg");
	Mix_VolumeMusic(64);
	//Mix_PlayMusic(music, -1);
	musicplaying=0;
	
	// multiplayer
	if (SDLNet_Init() < 0) {
		g_Close();
		printf("ERROR: SDLNet_Init: %s\n", SDLNet_GetError());
		exit(EXIT_FAILURE);
	}

	// starting a server
	if( server ) {
		// listen on the host's port
		if( !(sd = SDLNet_UDP_Open(PORT)) ) {
			g_Close();
			printf("ERROR: SDLNet_UDP_Open: %s\n", SDLNet_GetError());
			exit(EXIT_FAILURE);
		}
		
		// Allocate memory for the packet
		if( !(packet = SDLNet_AllocPacket(512)) ) {
			printf("ERROR: SDLNet_AllocPacket: %s\n", SDLNet_GetError());
			exit(EXIT_FAILURE);
		}
	}
	
	// joining a server
	else if( address != NULL ) {
		if( !(sd = SDLNet_UDP_Open(0)) ) {
			g_Close();
			printf("ERROR: SDLNet_UDP_Open: %s\n", SDLNet_GetError());
			exit(EXIT_FAILURE);
		}
		
		if( SDLNet_ResolveHost(&ip, address, PORT) < 0 ) {
			printf("ERROR: SDLNet_ResolveHost: %s\n", SDLNet_GetError());
			exit(EXIT_FAILURE);
		}
		
		// Allocate memory for the packet
		if( !(packet = SDLNet_AllocPacket(512)) ) {
			printf("ERROR: SDLNet_AllocPacket: %s\n", SDLNet_GetError());
			exit(EXIT_FAILURE);
		}
	}
	
	// report a success!
	if( address == NULL )
		i_Message( "Map loaded: %s", file );
	else
		i_Message( "Map loaded: %s\nConnected to %s", file, address );
	return(0);
}

/*-------------------------------------------------------------------------------
	
	g_Close
	
	Frees all memory currently allocated to the data used by the engine.
	
-------------------------------------------------------------------------------*/

void g_Close(void) {
	long x;
	
	// stop sound effects
	Mix_HaltChannel(-1);
	Mix_HaltMusic();
	
	// free sound effects
	for(x=0; x<sound_num; x++)
		Mix_FreeChunk(sounds[x]);
	free(sounds);
	Mix_FreeMusic(music);

	// free all entities
	e_FreeAll();

	// free world
	if( map.loaded ) {
		free(map.floors);
		free(map.floors_tex);
		free(map.floors_tex2);
		free(map.ceilings);
		free(map.ceilings_tex);
		free(map.ceilings_tex2);
		map.loaded = 0;
	}

	// free bitmaps
	SDL_FreeSurface(font8_bmp);
	SDL_FreeSurface(font16_bmp);
	r_FreeBmp( &console_bmp );
	
	SDL_FreeSurface(sky_bmp);
	SDL_FreeSurface(sky2_bmp);
	for( x=0; x<texture_num; x++ )
		r_FreeBmp( &walltex_bmp[x] );
	for( x=0; x<sprite_num; x++ )
		r_FreeBmp( &sprite_bmp[x] );

	r_FreeBmp( &pistol_bmp[0] );
	r_FreeBmp( &pistol_bmp[1] );
	r_FreeBmp( &pistol_bmp[2] );
	r_FreeBmp( &pistol_bmp[3] );
	r_FreeBmp( &pistol_bmp[4] );
	r_FreeBmp( &shotgun_bmp[0] );
	r_FreeBmp( &shotgun_bmp[1] );
	r_FreeBmp( &shotgun_bmp[6] );
	r_FreeBmp( &shotgun_bmp[8] );
	r_FreeBmp( &shotgun_bmp[11] );
	
	SDL_FreeSurface(screen);
	
	// close down net connections
	SDLNet_FreePacket(packet);
	if( address == NULL )
		SDLNet_UDP_Close(csd);
	SDLNet_UDP_Close(sd);
	SDLNet_Quit();
	
	// close SDL
	Mix_CloseAudio();
	SDL_Quit();
	
	// free video buffers
	free(zbuffer);
	free(floorbuffer);
	free(floorbuffer_s);
	free(rowbuffer);
}