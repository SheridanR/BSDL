/*-------------------------------------------------------------------------------

	BUBBENSTEIN/SDL
	File: bsdl.h
	Desc: contains some prototypes as well as various type definitions

	Copyright 2011 (c) Sheridan Rathbun, all rights reserved.
	See LICENSE.TXT for details.

-------------------------------------------------------------------------------*/

#include "SDL.h"
#include "sprig.h"
#include "SDL_mixer.h"

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
	double x, y; int z;              // world coordinates
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

extern int keystatus[323];
extern int mousestatus[5];
extern int mousex, mousey, omousex, omousey;
extern SDL_Event event;

// general vars
extern long fps;               // frames per second      
extern double t, ot, timesync; // used to calculate speed of last frame
extern int gameloop;           // determines whether or not the main game loop should continue
extern char message_str[64];   // onscreen message string
extern int message_time;       // the time before a message will disappear
extern unsigned long cycles;   // number of cycles the game has been running

// collision variables
extern int gfh, gch;
extern int stepclimb, sfh;
extern double targetx, targety; int targetz;

// player definitions
#define STEPHEI 20       // maximum player stepheight

extern entity_t *player;
extern double vx, vy, vz, va, la;        // player velocities
extern double bob1, bob2, bob3;          // controls camera bobbing
extern int run;                          // determines whether or not the player is running

// camera variables
extern double camx;
extern double camy;
extern int camz;
extern double camang;
extern int vang;

extern int selected_weapon; // currently selected weapon
extern unsigned int weap_anim;
extern int gunx, guny;
extern int weap_swap[3];
extern int weap_skill[3];

// game world
extern map_t map;
extern unsigned int num_ents;

extern SDL_Surface *screen, *screen2;

#define xres 640 // window size x
#define yres 480 // window size y
#define screenscale 1 // for scaling low resolution pictures to high-res displays

extern float darkness; // depth shading strength; higher number = more darkness

extern int hx, hy, hz;                   // view aspect ratios
extern float zbuffer[yres][xres];        // used to sort objects front to back
extern int floorbuffer[yres][xres][2];   // holds information that will be used to draw floors
extern int floorbuffer_s[yres][xres][2]; // holds information that will be used to draw floors
extern char rowbuffer[yres];             // used to determine which floor/ceiling rows need to be drawn

extern double sprsize; // used to resize sprites correctly regardless of resolution
extern double texsize; // same, only for walls

// the font bitmap
extern SDL_Surface *font8_bmp;
extern SDL_Surface *font16_bmp;

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
extern bitmap_t shotgun_bmp[9];

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
extern int e_CheckCells( double tx, double ty, int tz, entity_t* me );
extern int e_ClipVelocity( double *x, double *y, int *z, double vx, double vy, double vz, entity_t* me );
extern void e_MoveTrace( double *x1, double *y1, int *z1, double x2, double y2, int z2, entity_t* me );
extern hit_t e_LineTrace( entity_t *my, double x1, double y1, int z1, double angle, double vangle );
extern void e_CheckHit( hit_t hitspot );
extern void e_FreeAll(void);
extern void e_CreateEntity(void);
extern void e_DestroyEntity(entity_t *handle);

// global functions
extern int g_Open( char *file );
extern void g_Close(void);

// interface functions
extern void i_PrintText( SDL_Surface *font_bmp, int x, int y, char *fmt, ... );
extern void i_Message( char *fmt, ... );
extern void i_GetFrameRate(void);
extern void i_ReceiveInput(void);

// render functions
extern void r_ClearBuffers(void);
extern void r_FreeBmp( bitmap_t *bmp );
extern void r_LoadBmp( char *file, bitmap_t *bmp );
extern void r_DrawWeapons(void);
extern void r_DrawSky( double angle, double vangle );
extern void r_DrawColumns( double ox, double oy, int oz, double angle, double vangle );
extern void r_DrawFloors( double ox, double oy, int oz, double angle, double vangle );
extern void r_DrawSprites( double ox, double oy, int oz, double angle, double vangle );
extern SDL_Surface* r_ScaleSurface(SDL_Surface *Surface, Uint16 Width, Uint16 Height);

// sound functions
extern int a_EntitySound(entity_t *entity, Mix_Chunk *snd, int vol);