/*-------------------------------------------------------------------------------

	BUBBENSTEIN/SDL
	File: bsdl.h
	Desc: contains some prototypes as well as various type definitions

	Copyright 2011 (c) Sheridan Rathbun, all rights reserved.
	See LICENSE for details.

-------------------------------------------------------------------------------*/

#include <math.h>
#include <time.h>
#include "SDL.h"
#include "sprig.h"
#include "SDL_mixer.h"
#include "SDL_net.h"

// game world structure
typedef struct map_t {
	int loaded;      // lets the engine know whether or not there's data to be freed on shutdown.
	char name[32];   // name of the map
	char author[32]; // author of the map
	
	unsigned int width, height;  // size of the map

	int *floors;        // floor height map
	int *floors_tex;    // floor walls texture map
	int *floors_tex2;   // floor texture map

	int *ceilings;      // ceiling height map
	int *ceilings_tex;  // ceiling walls texture map
	int *ceilings_tex2; // ceiling texture map
} map_t;

#define MAPVERSION 1

// bitmap structure
typedef struct bitmap_t {
	unsigned long width, height; // size of the image
	char *data;                  // 24bit image data in RGB
} bitmap_t;

// entity structure
typedef struct entity_t {
	// VARS
	double x, y, z;                  // world coordinates
	double ang;                      // entity angle
	double sizex, sizey; int sizez;  // bounding box size
	int onground, onground2;         // if 1, the entity is on the ground
	int skill[9];                    // general purpose integer variables
	float fskill[9];                 // general purpose floats

	// FLAGS
	char flags;                      // 8 on|off properties for the entity

	// POINTERS
	bitmap_t *texture;               // the bitmap used to represent the entity
	struct entity_t *next;           // points to the next entity in the list
	struct entity_t *previous;       // used when deleting entities

	// BEHAVIOR
	void (*behavior)(struct entity_t *handle); // pointer to behavior function
} entity_t;

// entity packet structure
typedef struct entity_packet_t {
	// VARS
	double x, y, z;                 // world coordinates
	double ang;                     // entity angle
	double sizex, sizey; int sizez; // bounding box size
	int onground, onground2;        // if 1, the entity is on the ground
	char flags;                     // 8 on|off properties for the entity
	int tex;                        // index to the sprite bitmap
	char player;				  // set to 1 when this entity is a player
	int skill[9];                   // general purpose integer variables
	float fskill[9];                // general purpose floats
} entity_packet_t;

// hit structure
typedef struct hit_t {
	double x, y; int z;
	entity_t *entity;
} hit_t;

// entity list
extern entity_t *firstentity; // first entity in list
extern entity_t *lastentity;  // last entity in list

// entity flag definitions
#define FLAG_PASSABLE 1
#define FLAG_HIDDEN 2
#define FLAG_BRIGHT 4
#define FLAG_LIVING 8
#define FLAG_KILLABLE 16
#define FLAG_GRAVITY 32
#define FLAG_UNUSED1 64
#define FLAG_UNUSED2 128

//#define PI 3.1415926536

#define max(a,b) \
		({ typeof (a) _a = (a); \
		typeof (b) _b = (b); \
		_a > _b ? _a : _b; })
#define min(a,b) \
		({ typeof (a) _a = (a); \
		typeof (b) _b = (b); \
		_a < _b ? _a : _b; })

extern int sgn( double expr );

extern int keystatus[256]; // sym requires 323
extern int in_toggle1, in_toggle2, in_toggle3, in_toggle4, in_toggle5, in_toggle6, in_toggle7; // crap
extern int mousestatus[5];
extern int mousex, mousey, omousex, omousey;
extern SDL_Event event;

// input vars
#define TOTAL_COMMANDS 17
extern int in_commands[TOTAL_COMMANDS];
enum {
	IN_FORWARD = 0,
	IN_LEFT = 1,
	IN_BACK = 2,
	IN_RIGHT = 3,
	IN_UP = 4,
	IN_DOWN = 5,
	IN_RUN = 6,
	IN_JUMP = 7,
	IN_THIRDPERSON = 8,
	IN_PRINTINFO = 9,
	IN_NOCLIP = 10,
	IN_TESTSOUND = 11,
	IN_TESTMUSIC = 12,
	IN_CENTERVIEW = 13,
	IN_SPAWN = 14,
	IN_KILL = 15,
	IN_ATTACK = 16,
};

// general vars
extern long fps;               // frames per second      
extern double t, ot, timesync; // used to calculate speed of last frame
extern int gameloop;           // determines whether or not the main game loop should continue
extern char message_str[100];      // onscreen message string
extern int message_time;       // the time before a message will disappear
extern int message_y;          // the vertical position of the game messages
extern unsigned long cycles;   // number of cycles the game has been running

// multiplayer vars
#define PORT 38756
extern IPaddress ip;
extern UDPsocket sd, csd;
extern UDPpacket *packet;
extern int server, client;
extern char *address; // ip address to the server
extern int dialing;
extern int client_input[TOTAL_COMMANDS];
extern int client_keystatus[256]; // sym requires 323
extern int client_mousestatus[5];

// collision variables
extern int gfh, gch;
extern int stepclimb, sfh;
extern double targetx, targety; int targetz;

// player definitions
#define STEPHEI 20       // maximum player stepheight

extern entity_t *player;
#define VX fskill[3]
#define VY fskill[4]
#define VZ fskill[5]
#define VA fskill[6]
#define LA fskill[7]
#define PLAYER_NUM skill[0]
extern double bob1, bob2, bob3;          // controls camera bobbing
extern int run;                          // determines whether or not the player is running
extern int noclip;
extern int fly;

// camera variables
extern double camx;
extern double camy;
extern int camz;
extern double camang;
extern int vang;
extern int thirdperson;

extern int selected_weapon; // currently selected weapon
extern unsigned int weap_anim;
extern int weap_sound;
extern int gunx, guny;
extern int weap_swap[3];
extern int weap_skill[3];
extern int weap_mag[9];
extern int weap_ammo[9];

// game world
extern map_t map;
extern unsigned int num_ents;

extern SDL_Surface *screen, *screen2;

extern int xres; // window size x
extern int yres; // window size y
extern int vidsize;
extern int windowed; // determines whether the engine will run in a window or not

extern float darkness; // depth shading strength; higher number = more darkness

extern int hx, hy, hz;                   // view aspect ratios
extern float *zbuffer;        // used to sort objects front to back
extern int *floorbuffer;   // holds information that will be used to draw floors
extern int *floorbuffer_s; // holds information that will be used to draw floors
extern char *rowbuffer;             // used to determine which floor/ceiling rows need to be drawn
extern int drawsky;                      // used to clip the sky to regions of the screen where it needs to be drawn

extern double sprsize; // used to resize sprites correctly regardless of resolution
extern double texsize; // same, only for walls

// interface bitmaps
extern SDL_Surface *font8_bmp;
extern SDL_Surface *font16_bmp;
extern bitmap_t console_bmp;

// wall textures
extern unsigned int texture_num;
extern bitmap_t *walltex_bmp;
//bitmap_t sky_bmp;
extern SDL_Surface *sky_bmp, *sky2_bmp;

// sprite textures
extern unsigned int sprite_num;
extern bitmap_t *sprite_bmp;

// weapon textures
extern bitmap_t pistol_bmp[5];
extern bitmap_t shotgun_bmp[12];

// sound effects (uses SDL_mixer)
extern int audio_rate, audio_channels, audio_buffers;
extern Uint16 audio_format;
extern Mix_Chunk **sounds;
extern unsigned int sound_num;
extern Mix_Music *music;
extern int musicplaying;

// entity functions
extern void e_Cycle();
extern void e_ActChunk(entity_t* handle);
#define SPLAT_AIRTIME fskill[0]
#define SPLAT_FORCE fskill[1]
#define SPLAT_LIFE skill[0]
extern void e_ActSplat(entity_t* handle);
#define CHAR_HEALTH skill[0]
#define CHAR_ANIM skill[1]
#define CHAR_PAIN skill[2]
#define CHAR_OLDX fskill[0]
#define CHAR_OLDY fskill[1]
#define CHAR_DISTANCE fskill[2]
#define CHAR_VELOCITY fskill[3]
#define CHAR_MOVEDELAY fskill[4]
#define CHAR_DESTX fskill[5]
#define CHAR_DESTY fskill[6]
#define CHAR_FALL fskill[7]
extern void e_ActChar(entity_t* handle);
extern void e_ActPlayer(entity_t* handle);
extern int e_CheckCells( double tx, double ty, double tz, entity_t* me );
extern int e_ClipVelocity( double *x, double *y, double *z, double vx, double vy, double vz, entity_t* me );
extern void e_MoveTrace( double *x1, double *y1, double *z1, double x2, double y2, double z2, entity_t* me );
extern hit_t e_LineTrace( entity_t *my, double x1, double y1, double z1, double angle, double vangle );
extern void e_CheckHit( hit_t hitspot, int power );
extern void e_FreeAll(void);
extern void e_CreateEntity(void);
extern void e_DestroyEntity(entity_t *handle);

// global functions
extern int g_Open( char *file );
extern void g_Close(void);

// interface functions
extern void i_PrintText( SDL_Surface *font_bmp, int x, int y, char *fmt, ... );
extern void i_Message( char *fmt, ... );
extern void i_PrintMessages(void);
extern void i_GetFrameRate(void);
extern void i_ReceiveInput(void);
extern int i_GetStatus(int command, int playernum);
extern int i_ReadConfig(char *filename);

// render functions
extern void r_ClearBuffers(void);
extern void r_FreeBmp( bitmap_t *bmp );
extern void r_LoadBmp( char *file, bitmap_t *bmp );
extern void r_DrawWeapons(void);
extern void r_DrawConsole(void);
extern void r_DrawSky( double angle, double vangle );
extern void r_DrawColumns( double ox, double oy, int oz, double angle, double vangle );
extern void r_DrawFloors( double ox, double oy, int oz, double angle, double vangle );
extern void r_DrawSprites( double ox, double oy, int oz, double angle, double vangle );
extern SDL_Surface* r_ScaleSurface(SDL_Surface *Surface, Uint16 Width, Uint16 Height);

// sound functions
extern int a_EntitySound(entity_t *entity, Mix_Chunk *snd, int vol);