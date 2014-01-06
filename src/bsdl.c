/*-------------------------------------------------------------------------------

	BUBBENSTEIN/SDL
	File: bsdl.h
	Desc: defines all external prototypes in bsdl.h

	Copyright 2013 (c) Sheridan Rathbun, all rights reserved.
	See LICENSE for details.

-------------------------------------------------------------------------------*/

#include "bsdl.h"

// entity list
entity_t *firstentity = NULL; // first entity in list
entity_t *lastentity = NULL;  // last entity in list

int sgn( double expr ) {
	if( expr < 0 ) return(-1);
	else if( expr == 0 ) return(0);
	else return(1);
}

int keystatus[256]; // sym requires 323
int in_toggle1, in_toggle2, in_toggle3, in_toggle4, in_toggle5, in_toggle6, in_toggle7; // crap
int mousestatus[5];
int mousex, mousey, omousex, omousey;
SDL_Event event;

// input vars
int in_commands[TOTAL_COMMANDS];

// general vars
long fps = 0;             // frames per second
double t, ot, timesync;   // used to calculate speed of last frame
int gameloop;             // determines whether or not the main game loop should continue
char message_str[100];        // onscreen message string
int message_time;         // the time before a message will disappear
int message_y;            // the vertical position of the game messages
unsigned long cycles = 0; // number of cycles the game has been running

// multiplayer vars
IPaddress ip, *remoteIP;
UDPsocket sd, csd;
UDPpacket *packet;
int server=0;
int client=0;
char *address=NULL; // ip address to the server
int dialing=0;
int client_input[TOTAL_COMMANDS];
int client_keystatus[256]; // sym requires 323
int client_mousestatus[5];

// collision variables
int gfh, gch;
int stepclimb, sfh;
double targetx, targety; int targetz;

// player definitions
entity_t *player;
double bob1, bob2, bob3;          // controls camera bobbing
int run;                          // determines whether or not the player is running
int noclip=0;
int fly=0;

// camera variables
double camx;
double camy;
int camz;
double camang;
int vang;
int thirdperson=0;

int selected_weapon = 2; // currently selected weapon
unsigned int weap_anim = 0;
int weap_sound = 0;
int gunx, guny;
int weap_swap[3];
int weap_skill[3];
int weap_mag[9];
int weap_ammo[9];

// game world
map_t map;
unsigned int num_ents=0;

SDL_Surface *screen, *screen2;

// default resolution
int xres = 640;
int yres = 400;
int vidsize = 307200; // xres*yres
int windowed = 0; // determines whether the engine will run in a window or not

float darkness = 1.1; // depth shading strength; higher number = more darkness

int hx, hy, hz;     // view aspect ratios
float *zbuffer;     // used to sort objects front to back
int *floorbuffer;   // holds information that will be used to draw floors
int *floorbuffer_s; // holds information that will be used to draw floors
char *rowbuffer;    // used to determine which floor/ceiling rows need to be drawn
int drawsky;        // used to clip the sky to regions of the screen where it needs to be drawn

double sprsize; // used to resize sprites correctly regardless of resolution
double texsize; // same, only for walls

// interface bitmaps
SDL_Surface *font8_bmp;
SDL_Surface *font16_bmp;
bitmap_t console_bmp;

// wall textures
unsigned int texture_num;
bitmap_t *walltex_bmp;
//bitmap_t sky_bmp;
SDL_Surface *sky_bmp, *sky2_bmp;

// sprite textures
unsigned int sprite_num;
bitmap_t *sprite_bmp;

// weapon textures
bitmap_t pistol_bmp[5];
bitmap_t shotgun_bmp[12];

// sound effects (uses SDL_mixer)
int audio_rate = 22050;
Uint16 audio_format = AUDIO_S16;
int audio_channels = 2;
int audio_buffers = 512;
Mix_Chunk **sounds;
unsigned int sound_num;
Mix_Music *music;
int musicplaying;