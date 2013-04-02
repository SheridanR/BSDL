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

int keystatus[323];
int mousestatus[5];
int mousex, mousey, omousex, omousey;
SDL_Event event;

// general vars
long fps = 0;             // frames per second
double t, ot, timesync;   // used to calculate speed of last frame
int gameloop;             // determines whether or not the main game loop should continue
char *message_str;        // onscreen message string
int message_time;         // the time before a message will disappear
int message_y;            // the vertical position of the game messages
unsigned long cycles = 0; // number of cycles the game has been running

// collision variables
int gfh, gch;
int stepclimb, sfh;
double targetx, targety; int targetz;

// player definitions
entity_t *player;
double vx, vy, vz, va, la;        // player velocities
double bob1, bob2, bob3;          // controls camera bobbing
int run;                          // determines whether or not the player is running

// camera variables
double camx;
double camy;
int camz;
double camang;
int vang;

int selected_weapon = 2; // currently selected weapon
unsigned int weap_anim = 0;
int gunx, guny;
int weap_swap[3];
int weap_skill[3];

// game world
map_t map;
unsigned int num_ents=0;

SDL_Surface *screen, *screen2;

float darkness = 1.1; // depth shading strength; higher number = more darkness

int hx, hy, hz;                   // view aspect ratios
float zbuffer[yres][xres];        // used to sort objects front to back
int floorbuffer[yres][xres][2];   // holds information that will be used to draw floors
int floorbuffer_s[yres][xres][2]; // holds information that will be used to draw floors
char rowbuffer[yres];             // used to determine which floor/ceiling rows need to be drawn
int drawsky;                      // used to clip the sky to regions of the screen where it needs to be drawn

double sprsize; // used to resize sprites correctly regardless of resolution
double texsize; // same, only for walls

// the font bitmap
SDL_Surface *font8_bmp;
SDL_Surface *font16_bmp;

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
bitmap_t shotgun_bmp[9];

// sound effects (uses SDL_mixer)
int audio_rate = 22050;
Uint16 audio_format = AUDIO_S16;
int audio_channels = 2;
int audio_buffers = 512;
Mix_Chunk **sounds;
unsigned int sound_num;
Mix_Music *music;
int musicplaying;