/*-------------------------------------------------------------------------------

	BUBBENSTEIN/SDL
	File: g_main.c
	Desc: main game source file. initializes engine and starts game.

	Copyright 2013 (c) Sheridan Rathbun, all rights reserved.
	See LICENSE for details.

-------------------------------------------------------------------------------*/

#include "bsdl.h"

/*-------------------------------------------------------------------------------

	main

	Contains the main game loop.

-------------------------------------------------------------------------------*/

int main( int argc, char **argv ) {
	int a, x;
	int c, c2;
	char *maptoload = NULL;
	char *configtoload = NULL;
	SDL_Rect src, dest;
	entity_t *entity;
	entity_packet_t packetdata;
	
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
				// host a multiplayer game
				else if( !strcmp(argv[a], "-server") ) {
					server=1;
					//dialing=1;
				}
				// join a multiplayer game
				else if( !strncmp(argv[a], "-connect=", 9) ) {
					address=argv[a]+9;
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
	c=0;
	
	// master status has precedence over slave
	if( server )
		address = NULL;
	
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
	
	// start with a console if waiting for a connection
	while(dialing) {
		// perform routine application interfacing
		i_GetFrameRate();
		i_ReceiveInput();
		
		// draw a console
		r_DrawConsole();
		i_PrintMessages();
		
		// flip the page
		SDL_BlitSurface(screen, &src, screen2, &dest);
		SDL_Flip( screen2 );
		
		// check for a connection
		if( SDLNet_UDP_Recv(sd, packet) ) { // made a connection
			// print join
			i_Message("Client connected");
			if( !(csd = SDLNet_UDP_Open(PORT+2)) ) {
				printf("ERROR: SDLNet_UDP_Open: %s\n", SDLNet_GetError());
				exit(EXIT_FAILURE);
			}
			if( SDLNet_ResolveHost(&ip, (char *)packet->data, PORT) < 0 ) {
				printf("ERROR: SDLNet_ResolveHost: %s\n", SDLNet_GetError());
				exit(EXIT_FAILURE);
			}
			
			// end dialing sequence
			dialing=0;
		}
		else if( !gameloop ) // abort dialing
			dialing=0;
	}
	
	// main game loop
	while(gameloop) {
		// perform routine application interfacing
		i_GetFrameRate();
		i_ReceiveInput();
		
		// singleplayer
		if( address==NULL && !server )
			e_Cycle();
		
		// server code
		if( address!=NULL ) {
			if( address!=NULL ) {
				// read input from clients
				//SDLNet_TCP_Recv(csd,client_input,sizeof(client_input));
				//SDLNet_TCP_Recv(csd,client_keystatus,sizeof(client_keystatus));
				//SDLNet_TCP_Recv(csd,client_mousestatus,sizeof(client_mousestatus));
			}
			e_Cycle();
			if( address!=NULL ) {
				// communicate gamestate to clients
				c=0; c2=0;
				for( entity=firstentity; entity!=NULL; entity=entity->next )
					c++;
				memcpy(packet->data,&c,sizeof(int));
				for( entity=firstentity; entity!=NULL; entity=entity->next ) {
					// create a packet filled with the necessary data
					packetdata.x=entity->x;
					packetdata.y=entity->y;
					packetdata.z=entity->z;
					packetdata.ang=entity->ang;
					packetdata.sizex=entity->sizex;
					packetdata.sizey=entity->sizey;
					packetdata.sizez=entity->sizez;
					packetdata.onground=entity->onground;
					packetdata.onground2=entity->onground2;
					packetdata.flags=entity->flags;
					for( x=0; x<sprite_num; x++ ) {
						if( entity->texture == &sprite_bmp[x] ) {
							packetdata.tex=x;
							break;
						}
					}
					packetdata.player=(entity->behavior==&e_ActChar);
					for( x=0; x<9; x++ ) {
						packetdata.skill[x] = entity->skill[x];
						packetdata.fskill[x] = entity->fskill[x];
					}
					
					// add this bunch of data to the packet
					memcpy(packet->data,&packetdata+sizeof(int)+sizeof(packetdata)*c2,sizeof(packetdata));
					c2++;
				}
				packet->address.host = ip.host;
				packet->address.port = PORT;
				packet->len = strlen((char *)packet->data) + 1;
				if( SDLNet_UDP_Send(sd, -1, packet)==0 )
					i_Message("failed");
			}
		}
		
		// client code
		if( server ) {
			// send input to the server
			//SDLNet_TCP_Send(sd, (void *)in_commands, sizeof(in_commands));
			//SDLNet_TCP_Send(sd, (void *)keystatus, sizeof(keystatus));
			//SDLNet_TCP_Send(sd, (void *)mousestatus, sizeof(mousestatus));
			
			// receive game state from the server
			if(SDLNet_UDP_Recv(sd, packet)>0) {
				player=NULL;
				e_FreeAll(); // destroy my current gamestate
				memcpy(&c,packet->data,sizeof(int));
				for( c2=0; c2<c; c2++ ) {
					e_CreateEntity();
					memcpy(&packetdata,packet->data+sizeof(int)+sizeof(packetdata)*c2,sizeof(packetdata));
					lastentity->x=packetdata.x;
					lastentity->y=packetdata.y;
					lastentity->z=packetdata.z;
					lastentity->ang=packetdata.ang;
					lastentity->sizex=packetdata.sizex;
					lastentity->sizey=packetdata.sizey;
					lastentity->sizez=packetdata.sizez;
					lastentity->onground=packetdata.onground;
					lastentity->onground2=packetdata.onground2;
					lastentity->flags=packetdata.flags;
					lastentity->texture=&sprite_bmp[packetdata.tex];
					if( packetdata.player )
						player = lastentity;
					for( x=0; x<9; x++ ) {
						lastentity->skill[x] = packetdata.skill[x];
						lastentity->fskill[x] = packetdata.fskill[x];
					}
				}
			}
			
			// move the camera
			if( player != NULL ) {
				camx = player->x;
				camy = player->y;
				camz = player->z+bob2+22;
				camang = player->ang;
				i_Message("x=%f y=%f z=%f",player->x,player->y,player->z);
			}
		}
		
		// world rendering
		r_DrawSky( camang, vang );
		r_ClearBuffers();
		r_DrawColumns( camx, camy, camz, camang, vang );
		r_DrawFloors( camx, camy, camz, camang, vang );
		r_DrawSprites( camx, camy, camz, camang, vang );
		if( !thirdperson )
			r_DrawWeapons();
		
		// display fps counter
		i_PrintText( font8_bmp, 4, yres-20, "FPS: %d", fps ); // fps counter
		
		// show printed messages
		i_PrintMessages();
		
		// blit screen to screen2 (faster than locking/unlocking screen)
		SDL_BlitSurface(screen, &src, screen2, &dest);
		
		// flip the page
		SDL_Flip( screen2 );
		cycles++;
	}
	
	g_Close(); // stop the engine
	return(0);
}
