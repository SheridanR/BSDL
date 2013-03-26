/*-------------------------------------------------------------------------------

	BUBBENSTEIN/SDL
	File: e_behaviors.c
	Desc: contains various entity behaviors

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

	e_Cycle

	Computes the behavior functions for every entity in the game world.

-------------------------------------------------------------------------------*/

void e_Cycle()
{
	entity_t *handle;
	
	for( handle=firstentity; handle!=NULL; handle=handle->next)
	{
		if( handle->behavior != NULL )
			(*handle->behavior)(handle); // execute the entity's behavior function
	}
}

/*-------------------------------------------------------------------------------

	e_ActChar

	The behavior code of an NPC.

-------------------------------------------------------------------------------*/

void e_ActChar(entity_t *handle)
{
	double opx, opy;
	int opz;
	int frame;
	float dir;
	float speed;
	
	// count to ten, choose a destination.
	handle->fskill[4] -= timesync;
	if( handle->fskill[4] <= 0 )
	{
		handle->fskill[4] = 3000;
		handle->fskill[5] = min(max(handle->x,8),map.width-8)-8+(rand()%16)+1;
		handle->fskill[6] = min(max(handle->y,8),map.height-8)-8+(rand()%16)+1;
	}
	
	// rotate the actor
	dir = handle->ang - atan2( handle->fskill[6]-handle->y, handle->fskill[5]-handle->x );
	while( dir >= PI )
		dir -= PI*2;
	while( dir < -PI )
		dir += PI*2;
	
	if( dir > .1 )
		handle->ang -= timesync*.004;
	if( dir < -.1 )
		handle->ang += timesync*.004;
	
	if( handle->ang >= PI*2 )
		handle->ang -= PI*2;
	if( handle->ang < 0 )
		handle->ang += PI*2;
	
	// move the actor
	if( sqrt( pow(handle->x - handle->fskill[5],2) + pow(handle->y - handle->fskill[6],2) ) > 3 )
		speed = 1;
	else
		speed = 0;
	
	// calculate movement forces
	handle->fskill[3] += speed*timesync;
	handle->fskill[3] *= max(1-.01*timesync,0);
	if( !handle->onground )
		handle->fskill[7] += (-1)+min(1-.006*timesync,1);
	else
		handle->fskill[7] = 0;
	
	opx=handle->x+timesync*.00005*handle->fskill[3]*cos(handle->ang);
	opy=handle->y+timesync*.00005*handle->fskill[3]*sin(handle->ang);
	opz=handle->z+timesync*.8*handle->fskill[7];
	
	// move the actor and record the horizontal distance of the move
	handle->fskill[0] = handle->x;
	handle->fskill[1] = handle->y;
	e_MoveTrace( &handle->x, &handle->y, &handle->z, opx, opy, opz, handle );
	handle->fskill[2] += sqrt( pow(handle->x - handle->fskill[0],2) + pow(handle->y - handle->fskill[1],2) )*2;
	if( handle->fskill[2] >= 4 ) handle->fskill[2] = 0;
	
	// change my image based on frame of animation + direction
	dir = handle->ang + PI/8 - atan2(camy-handle->y, camx-handle->x);
	while( dir >= PI*2 )
		dir -= PI*2;
	while( dir < 0 )
		dir += PI*2;
	
	frame = 1 + 8*(int)floor(handle->fskill[2]); // animation
	frame += (int)floor( dir * 4/PI ); // direction
	handle->texture = &sprite_bmp[frame];
}

/*-------------------------------------------------------------------------------

	e_ActPlayer

	The behavior code of the player entity.

-------------------------------------------------------------------------------*/

void e_ActPlayer(entity_t *handle)
{
	double f, s, v, turn, look;
	double co, si;
	int fly = 0;
	double opx, opy;
	int opz;
	int x;
	int frame;
	float dir;
	
	if( keystatus[SDLK_k] )
	{
		keystatus[SDLK_k] = 0;
		i_Message( "X=%d Y=%d Z=%d\nAng=%f", (int)floor(handle->x), (int)floor(handle->y), handle->z, handle->ang );
	}
	
	run = (keystatus[SDLK_LSHIFT]+1);
	turn = mousex*run*.05;
	look = -mousey*run*7;
	f = (keystatus[SDLK_w]-keystatus[SDLK_s])*run; // forward
	s = (keystatus[SDLK_d]-keystatus[SDLK_a])*run; // strafe
	v = (keystatus[SDLK_e]-keystatus[SDLK_q]);
	
	co = cos(handle->ang); si = sin(handle->ang);
	vx += (co*f - si*s)*timesync*.000125;
	vy += (si*f + co*s)*timesync*.000125;
	if( fly ) vz = v*timesync/12;
	else
	{
		if( !handle->onground ) vz += (-1)+min(1-.006*timesync,1);
		else
		{
			if( !keystatus[SDLK_SPACE] )
				vz = 0;
			else
			{
				vz = .5;
				handle->onground = 0;
			}
		}
	}
	va += turn;
	la += look;
	f = max(1-.01*timesync,0);
	vx*=f; vy*=f; va*=f*.25; la*=f*.25;
	
	if( keystatus[SDLK_i] ) // insert a sprite
	{
		keystatus[SDLK_i] = 0;
		e_CreateEntity();
		lastentity->behavior = &e_ActChar;
		lastentity->texture = &sprite_bmp[1];
		
		lastentity->sizex = .45;
		lastentity->sizey = .45;
		lastentity->sizez = 52;
	
		lastentity->x=player->x+3;
		lastentity->y=player->y;
		lastentity->z=player->z;
		lastentity->ang=player->ang;
	}
	
	// my old position
	handle->fskill[0] = handle->x;
	handle->fskill[1] = handle->y;
	
	if( keystatus[SDLK_n] ) // noclip cheat
	{
		handle->x += vx*timesync;
		handle->y += vy*timesync;
		handle->z += vz*timesync;
	}
	else
	{
		opx=handle->x+vx*timesync;
		opy=handle->y+vy*timesync;
		opz=handle->z+vz*timesync;
		e_MoveTrace(&handle->x,&handle->y,&handle->z,opx,opy,opz,handle);
		handle->x = min(max(handle->x,.36),map.width-.36);
		handle->y = min(max(handle->y,.36),map.height-.36);
	}
	
	if( handle->onground )
	{
		if( keystatus[SDLK_w] || keystatus[SDLK_s] || keystatus[SDLK_a] || keystatus[SDLK_d] )
		{
			bob1 += bob3*timesync/48;
			bob2 += bob1*timesync/12;
			if( bob2 >= 3 && bob3 == 1 ) bob3 = -1;
			if( bob2 <= -3 && bob3 == -1 )
			{
				bob1 = 0;
				bob2 = -3;
				bob3 = 1;
			}
		}
		else
		{
			bob1 = 0;
			bob3 = 1;
			if( bob2 > 0 )
			{
				bob2 -= timesync/12;
				if( bob2 < 0 ) bob2 = 0;
			}
			if( bob2 < 0 )
			{
				bob2 += timesync/12;
				if( bob2 > 0 ) bob2 = 0;
			}
		}
	}
	
	handle->ang += va;
	if( handle->ang > PI*2 ) handle->ang -= PI*2;
	if( handle->ang < 0 ) handle->ang += PI*2;
	if( la >= 1 || la <= -1 ) 
		vang += la;
	x=(yres/240)*114;
	if( vang > x ) vang = x;
	if( vang < -x ) vang = -x;
	if( keystatus[SDLK_END] ) vang = 0;
	
	// movement code ends
	
	if( darkness < 1.1 )
	{
		darkness += timesync/1024;
		if( darkness > 1.1 ) darkness = 1.1;
	}
	if( weap_anim == 0 && weap_swap[2] == 0 )
	{
		// fire the weapon
		if( mousestatus[SDL_BUTTON_LEFT] && weap_skill[2] <= 0 )
		{
			darkness = 1.02;
			weap_anim = 1;
		}
		// number keys to change weapons
		if( keystatus[SDLK_2] && selected_weapon != 2 )
			weap_swap[2] = 2;
		else if( keystatus[SDLK_3] && selected_weapon != 3 )
			weap_swap[2] = 3;
	}
	if( weap_swap[2] ) // swapping weapons
	{
		if( !weap_swap[1] )
		{
			weap_swap[0] += timesync;
			if( weap_swap[0] >= 224 )
			{
				weap_swap[0] = 224;
				weap_swap[1] = 1;
				selected_weapon = weap_swap[2];
			}
		}
		else
		{
			weap_swap[0] -= timesync;
			if( weap_swap[0] < 0 )
			{
				weap_swap[0] = 0;
				weap_swap[1] = 0;
				weap_swap[2] = 0;
			}
		}
	}
	if( weap_skill[2] > 0 ) weap_skill[2] -= timesync;
	if( weap_anim ) // firing weapons
	{
		if( selected_weapon == 2 )
		{
			weap_skill[2] = 60;
			weap_skill[0] += timesync;
			if( weap_skill[0] > 60 )
			{
				weap_skill[0] = 0;
				weap_anim++;
				if( weap_anim > 2 )
				{
					weap_anim = 0;
				}
			}
		}
		if( selected_weapon == 3 )
		{
			weap_skill[2] = 60;
			weap_skill[0] += timesync;
			if( weap_skill[0] > 80 )
			{
				weap_skill[0] = 0;
				weap_anim++;
				if( weap_anim > 5 )
				{
					weap_anim = 0;
				}
			}
		}
	}
	
	// change my image based on frame of animation + direction
	dir = handle->ang + PI/8 - atan2(camy-handle->y, camx-handle->x);
	while( dir >= PI*2 )
		dir -= PI*2;
	while( dir < 0 )
		dir += PI*2;
	
	handle->fskill[2] += sqrt( pow(handle->x - handle->fskill[0],2) + pow(handle->y - handle->fskill[1],2) )*2;
	if( handle->fskill[2] >= 4 ) handle->fskill[2] = 0;
	frame = 1 + 8*(int)floor(handle->fskill[2]); // animation
	frame += (int)floor( dir * 4/PI ); // direction
	handle->texture = &sprite_bmp[frame];
	
	// move the camera!
	if( !keystatus[SDLK_p] )
	{
		camx = handle->x;
		camy = handle->y;
		camz = handle->z+bob2+22;
		camang = handle->ang;
	}
}