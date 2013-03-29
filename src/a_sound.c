/*-------------------------------------------------------------------------------

	BUBBENSTEIN/SDL
	File: a_sound.c
	Desc: contains functions for game sound effects

	Copyright 2013 (c) Sheridan Rathbun, all rights reserved.
	See LICENSE for details.

-------------------------------------------------------------------------------*/

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <malloc.h>
#include "bsdl.h"

/*-------------------------------------------------------------------------------

	a_EntitySound
	
	Plays a chunk at the given entity's position with the given volume.
	Returns the channel that the sound is being played in.

-------------------------------------------------------------------------------*/

int a_EntitySound(entity_t *entity, Mix_Chunk *snd, int vol) {
	int channel;
	Sint16 angle;
	Uint8 dist;
	
	channel = Mix_PlayChannel(-1, snd, 0);
	Mix_Volume(channel,vol);
	dist = min(sqrt( pow(camx - entity->x,2) + pow(camy - entity->y,2) )*5,255);
	if( dist > 0 ) {
		angle = (int)(atan2( camy-entity->y, camx-entity->x )*(180/PI)+180);
		angle -= (int)(camang*(180/PI));
		if(angle<0) angle += 360;
		else if(angle>=360) angle -= 360;
		Mix_SetPosition(channel, angle, dist);
	}
	return channel;
}