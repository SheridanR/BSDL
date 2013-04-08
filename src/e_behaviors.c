/*-------------------------------------------------------------------------------

	BUBBENSTEIN/SDL
	File: e_behaviors.c
	Desc: contains various entity behaviors

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

	e_Cycle

	Computes the behavior functions for every entity in the game world.

-------------------------------------------------------------------------------*/

void e_Cycle() {
	entity_t *entity=firstentity;
	entity_t *nextentity=entity->next;
	
	for( entity=firstentity; entity!=NULL; entity=nextentity) {
		nextentity = entity->next;
		if( entity->behavior != NULL )
			(*entity->behavior)(entity); // execute the entity's behavior function
	}
}

/*-------------------------------------------------------------------------------

	e_ActChunk

	The behavior code of a dirt/mortar chunk particle

-------------------------------------------------------------------------------*/

void e_ActChunk(entity_t* my) {
	my->skill[1] += timesync;
	if( my->skill[1] >= 10 ) {
		my->skill[1] = 0;
		my->z++;
	}
	my->skill[0] += timesync;
	if(my->skill[0] > 150)
		e_DestroyEntity(my);
}

/*-------------------------------------------------------------------------------

	e_ActSplat

	The behavior code of a blood splatter particle

-------------------------------------------------------------------------------*/

void e_ActSplat(entity_t* my) {
	double opx, opy; int opz;

	// calculate movement forces
	if( !my->onground )
		my->SPLAT_AIRTIME += (-1)+min(1-.006*timesync,1);
	else {
		my->SPLAT_AIRTIME = 0;
		my->SPLAT_LIFE += timesync;
	}
	
	// move
	opx=my->x+timesync*.0005*cos(my->ang);
	opy=my->y+timesync*.0005*sin(my->ang);
	opz=my->z+timesync*.2*my->SPLAT_AIRTIME;
	e_MoveTrace( &my->x, &my->y, &my->z, opx, opy, opz, my );
	
	if(my->SPLAT_LIFE > 100)
		e_DestroyEntity(my);
}

/*-------------------------------------------------------------------------------

	e_ActChar

	The behavior code of an NPC.

-------------------------------------------------------------------------------*/

void e_ActChar(entity_t *my) {
	double opx, opy;
	int opz;
	int frame;
	float dir;
	float speed = 0;
	
	if(my->CHAR_HEALTH > 0) {
		if(my->CHAR_PAIN <= 0) {
			// count to ten, choose a destination.
			my->CHAR_MOVEDELAY -= timesync;
			if( my->CHAR_MOVEDELAY <= 0 ) {
				my->CHAR_MOVEDELAY = 3000;
				my->CHAR_DESTX = min(max(my->x,8),map.width-8)-8+(rand()%16)+1;
				my->CHAR_DESTY = min(max(my->y,8),map.height-8)-8+(rand()%16)+1;
			}
			
			// rotate the actor
			dir = my->ang - atan2( my->CHAR_DESTY-my->y, my->CHAR_DESTX-my->x );
			while( dir >= PI )
				dir -= PI*2;
			while( dir < -PI )
				dir += PI*2;
			
			if( dir > .1 )
				my->ang -= timesync*.004;
			if( dir < -.1 )
				my->ang += timesync*.004;
			
			if( my->ang >= PI*2 )
				my->ang -= PI*2;
			if( my->ang < 0 )
				my->ang += PI*2;
			
			// move the actor
			if( sqrt( pow(my->x - my->CHAR_DESTX,2) + pow(my->y - my->CHAR_DESTY,2) ) > 3 )
				speed = 1;
		}
		else
			my->CHAR_PAIN -= timesync;
		
		// change my image based on frame of animation + direction
		dir = my->ang + PI/8 - atan2(camy-my->y, camx-my->x);
		while( dir >= PI*2 )
			dir -= PI*2;
		while( dir < 0 )
			dir += PI*2;
		
		if( my->CHAR_PAIN <= 0 )
			frame = 1 + 8*(int)floor(my->CHAR_DISTANCE); // animation
		else
			frame = 49;
		frame += (int)floor( dir * 4/PI ); // direction
		my->texture = &sprite_bmp[frame];
	}
	if(my->CHAR_HEALTH <= 0) {
		// make me a prop
		if( my->CHAR_ANIM >= 120)
			my->sizez = 16;
	
		// play dying animation
		my->CHAR_ANIM += timesync*.5;
		if(my->CHAR_ANIM > 240)
			my->CHAR_ANIM=240;
		my->texture = &sprite_bmp[57+my->CHAR_ANIM/40];
	}
	
	// calculate movement forces
	my->CHAR_VELOCITY += speed*timesync;
	my->CHAR_VELOCITY *= max(1-.01*timesync,0);
	if( !my->onground )
		my->CHAR_FALL += (-1)+min(1-.006*timesync,1);
	else
		my->CHAR_FALL = 0;
	
	opx=my->x+timesync*.00005*my->CHAR_VELOCITY*cos(my->ang);
	opy=my->y+timesync*.00005*my->CHAR_VELOCITY*sin(my->ang);
	opz=my->z+timesync*.8*my->CHAR_FALL;
	
	// move the actor and record the horizontal distance of the move
	my->CHAR_OLDX = my->x;
	my->CHAR_OLDY = my->y;
	e_MoveTrace( &my->x, &my->y, &my->z, opx, opy, opz, my );
	my->CHAR_DISTANCE += sqrt( pow(my->x - my->CHAR_OLDX,2) + pow(my->y - my->CHAR_OLDY,2) )*2;
	if( my->CHAR_DISTANCE >= 4 ) my->CHAR_DISTANCE = 0;
}

/*-------------------------------------------------------------------------------

	e_ActPlayer

	The behavior code of the player entity.

-------------------------------------------------------------------------------*/

void e_ActPlayer(entity_t *my) {
	double f, s, v, turn, look;
	double co, si;
	double opx, opy;
	int opz;
	int x;
	int frame;
	float dir;
	float offset;
	
	// toggle thirdperson
	if( keystatus[SDLK_p] ) {
		keystatus[SDLK_p] = 0;
		thirdperson=(thirdperson==0);
		if( thirdperson )
			i_Message("Thirdperson camera activated");
		else
			i_Message("Thirdperson camera deactivated");
	}
	
	// toggle noclip
	if( keystatus[SDLK_n] ) {
		keystatus[SDLK_n] = 0;
		noclip=(noclip==0);
		fly=(fly==0);
		if( noclip )
			i_Message("noclip on");
		else
			i_Message("noclip off");
	}
	
	// report position
	if( keystatus[SDLK_k] ) {
		keystatus[SDLK_k] = 0;
		i_Message( "X=%d Y=%d Z=%d\nAng=%f\nVang=%d\nxres=%d\nyres=%d", (int)floor(my->x), (int)floor(my->y), my->z, my->ang, vang, xres, yres );
	}
	
	// play sound effect
	if( keystatus[SDLK_b] ) {
		keystatus[SDLK_b] = 0;
		i_Message( "Mix_PlayChannel: %d", Mix_PlayChannel(-1, sounds[0], 0));
	}
	
	// toggle music
	if( keystatus[SDLK_m] ) {
		keystatus[SDLK_m] = 0;
		if(musicplaying) {
			Mix_HaltMusic();
			musicplaying=0;
		}
		else {
			x = Mix_PlayMusic(music, -1);
			i_Message( "Mix_PlayMusic: %d", x);
			if( x == 0 )
				musicplaying=1;
		}
	}
	
	run = (keystatus[SDLK_LSHIFT]+1);
	turn = mousex*run*.05;
	look = -mousey*run*7;
	f = (keystatus[SDLK_w]-keystatus[SDLK_s])*run; // forward
	s = (keystatus[SDLK_d]-keystatus[SDLK_a])*run; // strafe
	v = (keystatus[SDLK_e]-keystatus[SDLK_q])*run; // up/down
	
	co = cos(my->ang); si = sin(my->ang);
	vx += (co*f - si*s)*timesync*.000125;
	vy += (si*f + co*s)*timesync*.000125;
	if( fly ) vz = v*timesync/25;
	else {
		if( !my->onground ) vz += (-1)+min(1-.006*timesync,1);
		else {
			if( !keystatus[SDLK_SPACE] )
				vz = 0;
			else {
				vz = .5;
				my->onground = 0;
			}
		}
	}
	va += turn;
	la += look;
	f = max(1-.01*timesync,0);
	vx*=f; vy*=f; va*=f*.25; la*=f*.25;
	
	if( keystatus[SDLK_i] ) { // insert a sprite
		keystatus[SDLK_i] = 0;
		e_CreateEntity();
		lastentity->behavior = &e_ActChar;
		lastentity->texture = &sprite_bmp[1];
		
		lastentity->sizex = .5;
		lastentity->sizey = .5;
		lastentity->sizez = 52;
	
		lastentity->x=player->x+3;
		lastentity->y=player->y;
		lastentity->z=player->z;
		lastentity->ang=player->ang;
		
		lastentity->CHAR_HEALTH = 100;
	}
	
	// my old position
	my->fskill[0] = my->x;
	my->fskill[1] = my->y;
	
	if( noclip ) { // noclip cheat
		my->x += vx*timesync;
		my->y += vy*timesync;
		my->z += vz*timesync;
		my->x = min(max(1,my->x),map.width-1);
		my->y = min(max(1,my->y),map.height-1);
		my->z = min(max(-10000,my->z),10000);
	} else {
		opx=my->x+vx*timesync;
		opy=my->y+vy*timesync;
		opz=my->z+vz*timesync;
		e_MoveTrace(&my->x,&my->y,&my->z,opx,opy,opz,my);
		my->x = min(max(my->x,.36),map.width-.36);
		my->y = min(max(my->y,.36),map.height-.36);
	}
	
	if( my->onground ) {
		if( keystatus[SDLK_w] || keystatus[SDLK_s] || keystatus[SDLK_a] || keystatus[SDLK_d] ) {
			bob1 += bob3*timesync/48;
			bob1 *= f;
			bob2 += bob1*timesync/12;
			if( bob2 >= 3 && bob3 == 1 ) bob3 = -1;
			if( bob2 <= -3 && bob3 == -1 ) {
				bob1 = 0;
				bob2 = -3;
				bob3 = 1;
			}
		} else {
			bob1 = 0;
			bob3 = 1;
			if( bob2 > 0 ) {
				bob2 -= timesync/12;
				if( bob2 < 0 ) bob2 = 0;
			}
			if( bob2 < 0 ) {
				bob2 += timesync/12;
				if( bob2 > 0 ) bob2 = 0;
			}
		}
	}
	
	my->ang += va;
	if( my->ang > PI*2 ) my->ang -= PI*2;
	if( my->ang < 0 ) my->ang += PI*2;
	if( la >= 1 || la <= -1 ) 
		vang += la;
	x=(yres/240.0)*114.0;
	if( vang > x ) vang = x;
	if( vang < -x ) vang = -x;
	if( keystatus[SDLK_END] ) vang = 0;
	
	// movement code ends
	
	if( darkness < 1.1 ) {
		darkness += timesync/1024;
		if( darkness > 1.1 ) darkness = 1.1;
	}
	if( weap_swap[2] == 0 ) {
		// fire the weapon
		if( mousestatus[SDL_BUTTON_LEFT] && weap_skill[2] <= 0 && !(my->flags&FLAG_UNUSED1) ) {
			darkness = 1.02;
			weap_anim = 1;
			my->flags |= FLAG_UNUSED1; // holding the trigger
			if( selected_weapon == 2 ) {
				a_EntitySound(my,sounds[4],64);
				e_CheckHit(e_LineTrace( my, my->x, my->y, my->z+bob2+22, my->ang, vang ));
			}
			else if( selected_weapon == 3 ) {
				a_EntitySound(my,sounds[6],64);
				for( offset=-8; offset<=8; offset+=4 )
					e_CheckHit(e_LineTrace( my, my->x, my->y, my->z+bob2+22, my->ang+(offset*PI/180), vang ));
			}
		}
		
		// refire is available
		if( !mousestatus[SDL_BUTTON_LEFT] )
			my->flags &= ~(FLAG_UNUSED1);
		
		if( weap_anim == 0 ) {
			// number keys to change weapons
			if( keystatus[SDLK_2] && selected_weapon != 2 )
				weap_swap[2] = 2;
			else if( keystatus[SDLK_3] && selected_weapon != 3 )
				weap_swap[2] = 3;
		}
	}
	if( weap_swap[2] ) { // swapping weapons
		if( !weap_swap[1] ) {
			weap_swap[0] += timesync;
			if( weap_swap[0] >= 128 ) {
				weap_swap[0] = 128;
				weap_swap[1] = 1;
				selected_weapon = weap_swap[2];
			}
		} else {
			weap_swap[0] -= timesync;
			if( weap_swap[0] < 0 ) {
				weap_swap[0] = 0;
				weap_swap[1] = 0;
				weap_swap[2] = 0;
			}
		}
	}
	if( weap_skill[2] > 0 ) weap_skill[2] -= timesync;
	if( weap_anim ) { // firing weapons
		if( selected_weapon == 2 ) {
			weap_skill[2] = 10;
			weap_skill[0] += timesync;
			if( weap_skill[0] > 30 ) {
				weap_skill[0] = 0;
				weap_anim++;
				if( weap_anim > 4 ) {
					weap_anim = 0;
				}
			}
		}
		if( selected_weapon == 3 ) {
			weap_skill[2] = 60;
			weap_skill[0] += timesync;
			if( weap_skill[0] > 30 ) {
				weap_skill[0] = 0;
				weap_anim++;
				if( weap_anim > 8 ) {
					weap_anim = 0;
				}
			}
		}
	}
	
	// change my image based on frame of animation + direction
	dir = my->ang + PI/8 - atan2(camy-my->y, camx-my->x);
	while( dir >= PI*2 )
		dir -= PI*2;
	while( dir < 0 )
		dir += PI*2;
	
	my->fskill[2] += sqrt( pow(my->x - my->fskill[0],2) + pow(my->y - my->fskill[1],2) )*2;
	if( my->fskill[2] >= 4 ) my->fskill[2] = 0;
	frame = 1 + 8*(int)floor(my->fskill[2]); // animation
	frame += (int)floor( dir * 4/PI ); // direction
	my->texture = &sprite_bmp[frame];
	
	// move the camera!
	if( !thirdperson ) {
		camx = my->x;
		camy = my->y;
		camz = my->z+bob2+22;
		camang = my->ang;
	}
	
	// kill troops
	entity_t *entity;
	if( keystatus[SDLK_g] )
		for( entity=firstentity; entity!=NULL; entity=entity->next) {
			if( entity->behavior == &e_ActChar )
				entity->CHAR_HEALTH = 0;
		}
}