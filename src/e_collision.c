/*-------------------------------------------------------------------------------

	BUBBENSTEIN/SDL
	File: e_collision.c
	Desc: handles entity collision detection against the world

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

	e_CheckCells

	Tests all four cells surrounding the position (tx, ty, tz) and returns
	a 1 if the position intersects with a cell.

-------------------------------------------------------------------------------*/

int e_CheckCells( double tx, double ty, int tz, entity_t* me ) {
	double sx, sy;
	float ffx, ffy;
	long fx, fy;
	long fh, ch;
	entity_t* entity;

	for( sy=-me->sizey; sy<=me->sizey; sy+=min(me->sizey,1) ) {
		for( sx=-me->sizex; sx<=me->sizex; sx+=min(me->sizex,1) ) {
			fx = floor(tx+sx); fy = floor(ty+sy); // int coordinates
			ffx = tx+sx; ffy = ty+sy; // float coordinates
			
			fh = map.floors[fx+fy*map.width] + (me->sizez*.5) - 1; // get the initial floor/ceiling heights
			ch = map.ceilings[fx+fy*map.width] - (me->sizez*.5) - 1;
			if( firstentity != NULL ) { // check against entities
				for( entity=firstentity; entity!=NULL; entity=entity->next ) {
					if( !(entity->flags&FLAG_PASSABLE) && entity != me ) { // if the entity is neither passable nor myself
						if( ffx >= entity->x-entity->sizex && ffx <= entity->x+entity->sizex ) { // if this horizontal position intersects with the entity
							if( ffy >= entity->y-entity->sizey && ffy <= entity->y+entity->sizey ) { // if this vertical position intersects with the entity
								if( me->z > entity->z && fh < entity->z+entity->sizez )
									fh=entity->z+entity->sizez; // adjust the floor height to include me!
								if( me->z < entity->z && ch > entity->z-entity->sizez )
									ch=entity->z-entity->sizez; // adjust the ceiling height to include me!
								else if( me->z == entity->z )
									return(1); // you're level with the entity. stop that ship!
							}
						}
					}
				}
			}
			
			if( fh >= ch ) return(1); // the floor is in the ceiling
			if( (fh-tz) > STEPHEI ) { stepclimb=0; return(1); } // this floor is too high
			else sfh = max(fh+1,sfh);
			if( fh+1 >= tz ) me->onground=1; // I am on the ground
			if( (tz-ch) > -1 ) return(1); // the ceiling is too low
			
			gfh = max(fh+1,gfh); // remember the highest position of the floor for this collision pass
			gch = min(ch-1,gch); // remember the lowest position of the ceiling for this collision pass
		}
	}
	return(0);
}

/*-------------------------------------------------------------------------------

	e_ClipVelocity

	Takes a velocity (vx, vy, vz) and clips it against the cells
	surrounding the position (*x, *y, *z)

-------------------------------------------------------------------------------*/

int e_ClipVelocity( double *x, double *y, int *z, double vx, double vy, double vz, entity_t* me ) {
	double tx, ty; int tz;
	long fx, fy;
	//long fh, ch;
	int exitcode = 1;

	if( *x+vx < .36 ) vx -= *x+vx-.36;
	if( *y+vy < .36 ) vy -= *y+vy-.36;
	if( *x+vx > map.width-.36 ) vx -= *x+vx-map.width+.36;
	if( *y+vy > map.height-.36 ) vy -= *y+vy-map.height+.36;
	
	fx=floor(*x); fy=floor(*y);
	sfh = map.floors[fx+fy*map.width];
	gfh = map.floors[fx+fy*map.width] + (me->sizez*.5);
	gch = map.ceilings[fx+fy*map.width] - (me->sizez*.5);

	stepclimb=1;
	if(me->onground == 0) me->onground2 = 0;
	//else me->onground = 0;
	tx=*x+vx;
	ty=*y+vy;
	tz=*z;
	tz=max(tz,map.floors[fx+fy*map.width]);
	if( !e_CheckCells(tx,ty,tz,me) ) {
		*x=tx;
		*y=ty;
		exitcode=0;
	} else {
		tx=*x+vx;
		ty=*y;
		tz=*z;
		tz=max(tz,map.floors[fx+fy*map.width]);
		if( !e_CheckCells(tx,ty,tz,me) ) {
			*x=tx;
			*y=ty;
		} else {
			tx=*x;
			ty=*y+vy;
			tz=*z;
			tz=max(tz,map.floors[fx+fy*map.width]);
			if( !e_CheckCells(tx,ty,tz,me) ) {
				*x=tx;
				*y=ty;
			}
		}
	}

	tx=*x; ty=*y; tz=*z;
	fx=floor(tx); fy=floor(ty);

	//fh = map.floors[fx+fy*map.width];
	//ch = map.ceilings[fx+fy*map.width];

	*z=min(max((tz+vz),gfh-me->sizez),gch);
	if(*z <= gfh) { // on ground?
		if( !me->onground2 ) { // landing from a fall?
			*z=gfh;
			me->onground2=1;
		} else {
			if(*z < gfh) { // climbing a step?
				if( stepclimb || gfh-sfh < STEPHEI ) {
					*z += timesync*.3;
					if(*z>gfh) *z=gfh;
				}
			}
		}
	}
	if(*z > gfh) // above ground?
		me->onground=0;

	return(exitcode);
}

/*-------------------------------------------------------------------------------

	e_MoveTrace

	Traces from *x1, *y1, *z1 to x2, y2, z2 performing collision detection along the
	way. If an impassable obstacle or the destination is reached, set the
	given pointers *x1, *y1, and *z1 to end position.

-------------------------------------------------------------------------------*/

void e_MoveTrace( double *x1, double *y1, int *z1, double x2, double y2, int z2, entity_t* me ) {
	double tracex, tracey, tracex2, tracey2;
	int tracez, tracez2;
	double dx, dy, dxabs, dyabs, x, y;
	int sdx, sdy, i;

	dx=x2-*x1;		// the horizontal distance of the line
	dy=y2-*y1;		// the vertical distance of the line
	dxabs=abs(dx);
	dyabs=abs(dy);
	sdx=sgn(dx);
	sdy=sgn(dy);
	x=dyabs*.5;
	y=dxabs*.5;
	tracex=*x1;
	tracey=*y1;
	tracez=*z1;
	tracex2=x2;
	tracey2=y2;
	tracez2=z2-tracez;
	
	if( e_ClipVelocity(&tracex,&tracey,&tracez,min(1,tracex2-tracex),min(1,tracey2-tracey),tracez2,me) ) {
		*x1 = tracex;
		*y1 = tracey;
		*z1 = tracez;
		return;
	}
	if( dxabs >= dyabs ) { // the line is more horizontal than vertical
		for( i=0; i<dxabs; i++ ) {
			y+=dyabs;
			if( y >= dxabs ) {
				y -= dxabs;
				tracey += sdy;
			}
			tracex += sdx;
			
			if( e_ClipVelocity(&tracex,&tracey,&tracez,min(1,tracex2-tracex),min(1,tracey2-tracey),tracez2,me) ) {
				*x1 = tracex;
				*y1 = tracey;
				*z1 = tracez;
				return;
			}
		}
	} else { // the line is more vertical than horizontal
		for( i=0; i<dyabs; i++ ) {
			x += dxabs;
			if( x >= dyabs ) {
				x -= dyabs;
				tracex += sdx;
			}
			tracey += sdy;
			
			if( e_ClipVelocity(&tracex,&tracey,&tracez,min(1,tracex2-tracex),min(1,tracey2-tracey),tracez2,me) ) {
				*x1 = tracex;
				*y1 = tracey;
				*z1 = tracez;
				return;
			}
		}
	}
	*x1 = tracex;
	*y1 = tracey;
	*z1 = tracez;
}

void e_LineTrace( double x1, double y1, int z1, double angle, double vangle ) {
	// empty implementation
}