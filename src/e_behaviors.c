/*-------------------------------------------------------------------------------

	BUBBENSTEIN/SDL
	File: e_behaviors.c
	Desc: contains various entity behaviors

	Copyright 2013 (c) Sheridan Rathbun, all rights reserved.
	See LICENSE for details.

-------------------------------------------------------------------------------*/

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

void e_ActFountain(entity_t* my) {
	my->skill[0] += timesync;
	if( my->skill[0] > 10 ) {
		my->skill[0]=0;
		e_CreateEntity();
		lastentity->behavior = &e_ActSplat;
		lastentity->texture = &sprite_bmp[64];
		lastentity->flags |= FLAG_PASSABLE;
		
		lastentity->sizex = .25;
		lastentity->sizey = .25;
		lastentity->sizez = 16;
		
		lastentity->x=my->x;
		lastentity->y=my->y;
		lastentity->z=my->z;
		lastentity->ang=rand()*360;
		
		lastentity->SPLAT_AIRTIME = 3;
		lastentity->SPLAT_FORCE = 2;
		lastentity->onground=1;
	}
}

void e_ActSplat(entity_t* my) {
	double opx, opy; int opz;

	// calculate movement forces
	if( !my->onground )
		my->SPLAT_AIRTIME += (-1)+min(1-.006*timesync,1);
	else {
		if(my->SPLAT_AIRTIME<0)
			my->SPLAT_AIRTIME = 0;
		my->SPLAT_LIFE += timesync;
	}
	
	// move
	opx=my->x+timesync*.0005*cos(my->ang)*my->SPLAT_FORCE;
	opy=my->y+timesync*.0005*sin(my->ang)*my->SPLAT_FORCE;
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
	bitmap_t temp_bmp;
	double f, s, v, turn, look;
	double co, si;
	double opx, opy;
	int opz;
	int x;
	int frame;
	float dir;
	float offset;
	
	// toggle thirdperson
	if( i_GetStatus(IN_THIRDPERSON,my->PLAYER_NUM) && in_toggle1 ) {
		in_toggle1 = 0;
		thirdperson=(thirdperson==0);
		if( thirdperson )
			i_Message("Thirdperson camera activated");
		else
			i_Message("Thirdperson camera deactivated");
	} else if( !i_GetStatus(IN_THIRDPERSON,my->PLAYER_NUM) )
		in_toggle1=1;
	
	// toggle noclip
	if( i_GetStatus(IN_NOCLIP,my->PLAYER_NUM) && in_toggle2 ) {
		in_toggle2 = 0;
		noclip=(noclip==0);
		fly=(fly==0);
		if( noclip )
			i_Message("noclip on");
		else
			i_Message("noclip off");
	} else if( !i_GetStatus(IN_NOCLIP,my->PLAYER_NUM) )
		in_toggle2=1;
	
	// report variable status
	if( i_GetStatus(IN_PRINTINFO,my->PLAYER_NUM) && in_toggle3 ) {
		in_toggle3=0;
		i_Message( "cycles=%d\nxres=%d yres=%d\nserver=%d client=%d\naddress=%s\ncamx=%d camy=%d camz=%d\ncamang=%d vang=%d\n",
		            cycles, xres, yres, server, client, address, (int)floor(camx), (int)floor(camy), (int)floor(camz), (int)floor(camang), vang );
	} else if( !i_GetStatus(IN_PRINTINFO,my->PLAYER_NUM) )
		in_toggle3=1;
	
	// play sound effect
	if( i_GetStatus(IN_TESTSOUND,my->PLAYER_NUM) && in_toggle4 ) {
		in_toggle4=0;
		i_Message( "Mix_PlayChannel: %d", Mix_PlayChannel(-1, sounds[0], 0));
	} else if( !i_GetStatus(IN_TESTSOUND,my->PLAYER_NUM) )
		in_toggle4=1;
	
	// toggle music
	if( i_GetStatus(IN_TESTMUSIC,my->PLAYER_NUM) && in_toggle5 ) {
		in_toggle5=0;
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
	} else if( !i_GetStatus(IN_TESTMUSIC,my->PLAYER_NUM) )
		in_toggle5=1;
	
	run = (i_GetStatus(IN_RUN,my->PLAYER_NUM)+1);
	turn = mousex*run*.05;
	look = -mousey*run*7;
	f = (i_GetStatus(IN_FORWARD,my->PLAYER_NUM)-i_GetStatus(IN_BACK,my->PLAYER_NUM))*run; // forward
	s = (i_GetStatus(IN_RIGHT,my->PLAYER_NUM)-i_GetStatus(IN_LEFT,my->PLAYER_NUM))*run; // strafe
	v = (i_GetStatus(IN_UP,my->PLAYER_NUM)-i_GetStatus(IN_DOWN,my->PLAYER_NUM))*run; // up/down
	
	co = cos(my->ang); si = sin(my->ang);
	my->VX += (co*f - si*s)*timesync*.000125;
	my->VY += (si*f + co*s)*timesync*.000125;
	if( fly ) my->VZ = v*.35;
	else {
		if( !my->onground ) my->VZ += (-1)+min(1-.006*timesync,1);
		else {
			if( !i_GetStatus(IN_JUMP,my->PLAYER_NUM) )
				my->VZ = 0;
			else {
				my->VZ = .5;
				my->onground = 0;
			}
		}
	}
	my->VA += turn;
	my->LA += look;
	f = max(1-.01*timesync,0);
	my->VX*=f; my->VY*=f; if(fly) my->VZ*=f; my->VA*=f*.25; my->LA*=f*.25;
	
	if( i_GetStatus(IN_SPAWN,my->PLAYER_NUM) && in_toggle6 ) { // insert a sprite
		in_toggle6=0;
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
		/*e_CreateEntity();
		lastentity->behavior = &e_ActFountain;
		lastentity->texture = NULL;
		lastentity->x=player->x;
		lastentity->y=player->y;
		lastentity->z=player->z-player->sizez+8;*/
	} else if( !i_GetStatus(IN_SPAWN,my->PLAYER_NUM) )
		in_toggle6=1;
	
	// my old position
	my->fskill[0] = my->x;
	my->fskill[1] = my->y;
	
	if( noclip ) { // noclip cheat
		my->x += my->VX*timesync;
		my->y += my->VY*timesync;
		my->z += my->VZ*timesync;
		my->x = min(max(1,my->x),map.width-1);
		my->y = min(max(1,my->y),map.height-1);
		my->z = min(max(-10000,my->z),10000);
	} else {
		opx=my->x+my->VX*timesync;
		opy=my->y+my->VY*timesync;
		opz=my->z+my->VZ*timesync;
		e_MoveTrace(&my->x,&my->y,&my->z,opx,opy,opz,my);
		my->x = min(max(my->x,.36),map.width-.36);
		my->y = min(max(my->y,.36),map.height-.36);
	}
	
	if( my->onground ) {
		if( i_GetStatus(IN_FORWARD,my->PLAYER_NUM) || i_GetStatus(IN_LEFT,my->PLAYER_NUM) || i_GetStatus(IN_BACK,my->PLAYER_NUM) || i_GetStatus(IN_RIGHT,my->PLAYER_NUM) ) {
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
	
	my->ang += my->VA;
	if( my->ang > PI*2 ) my->ang -= PI*2;
	if( my->ang < 0 ) my->ang += PI*2;
	if( my->LA >= 1 || my->LA <= -1 ) 
		vang += my->LA;
	x=(yres/240.0)*114.0;
	if( vang > x ) vang = x;
	if( vang < -x ) vang = -x;
	if( i_GetStatus(IN_CENTERVIEW,my->PLAYER_NUM) ) vang = 0;
	
	// movement code ends
	
	if( darkness < 1.1 ) {
		darkness += timesync/1024;
		if( darkness > 1.1 ) darkness = 1.1;
	}
	if( weap_swap[2] == 0 ) {
		// fire the weapon
		if( i_GetStatus(IN_ATTACK,my->PLAYER_NUM) && weap_skill[2] <= 0 && !(my->flags&FLAG_UNUSED1) ) {
			darkness = 1.02;
			weap_anim = 1;
			my->flags |= FLAG_UNUSED1; // holding the trigger
			if( selected_weapon == 2 ) {
				a_EntitySound(my,sounds[4],64);
				e_CheckHit(e_LineTrace( my, my->x, my->y, my->z+bob2+22, my->ang+(((rand()%3)-1)*PI/180), vang+(rand()%11)-5 ), 25);
			}
			else if( selected_weapon == 3 ) {
				weap_mag[2]--; // reduce ammo in magazine
				a_EntitySound(my,sounds[6],64);
				for( offset=0; offset<4; offset++ )
					e_CheckHit(e_LineTrace( my, my->x, my->y, my->z+bob2+22, my->ang+(((rand()%5)-2)*PI/180), vang+(rand()%21)-10 ), 15);
				
				// animation stuff
				temp_bmp=shotgun_bmp[1];
				shotgun_bmp[1]=shotgun_bmp[11];
				shotgun_bmp[11]=temp_bmp;
			}
		}
		
		// make special animation sounds
		if( weap_anim == 0 )
			weap_sound = 0;
		if( selected_weapon == 3 )
			if( weap_anim >= 7 && weap_sound == 0 ) {
				weap_sound = 1;
				a_EntitySound(my,sounds[5],64);
			}
		
		// refire is available
		if( !i_GetStatus(IN_ATTACK,my->PLAYER_NUM) )
			my->flags &= ~(FLAG_UNUSED1);
		
		if( weap_anim == 0 ) {
			// number keys to change weapons
			if( keystatus[3] && selected_weapon != 2 )
				weap_swap[2] = 2;
			else if( keystatus[4] && selected_weapon != 3 )
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
			if(weap_mag[2] == 0) {
				weap_skill[2] = 60;
				weap_mag[2] = 2; // reload shotgun
			}
			else
				weap_skill[2] = 20;
			weap_skill[0] += timesync;
			if( weap_skill[0] > 30 ) {
				weap_skill[0] = 0;
				weap_anim++;
				if( weap_mag[2] == 1 && weap_anim > 2 ) {
					weap_anim = 0;
				}
				else if( weap_anim > 10 ) {
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
	
	if( weap_anim == 0 ) {
		my->fskill[2] += sqrt( pow(my->x - my->fskill[0],2) + pow(my->y - my->fskill[1],2) )*2;
		if( my->fskill[2] >= 4 ) my->fskill[2] = 0;
	}
	if( weap_anim > 0 ) {
		my->fskill[2] = 4;
		if( selected_weapon == 2 ) {
			if( weap_anim > 1 )
				my->fskill[2] = 5;
		}
		if( selected_weapon == 3 ) {
			if( weap_anim > 1 )
				my->fskill[2] = 5;
			if( weap_anim > 3 )
				my->fskill[2] = 4;
		}
	}
	frame = 1 + 8*(int)floor(my->fskill[2]); // animation
	frame += (int)floor( dir * 4/PI ); // direction
	my->texture = &sprite_bmp[frame];
	
	// move the camera!
	if( !thirdperson && !my->PLAYER_NUM ) {
		camx = my->x;
		camy = my->y;
		camz = my->z+bob2+22;
		camang = my->ang;
	}
	
	// kill troops
	entity_t *entity;
	if( i_GetStatus(IN_KILL,my->PLAYER_NUM) )
		for( entity=firstentity; entity!=NULL; entity=entity->next) {
			if( entity->behavior == &e_ActChar )
				entity->CHAR_HEALTH = 0;
		}
}