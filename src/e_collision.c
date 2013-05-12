/*-------------------------------------------------------------------------------

	BUBBENSTEIN/SDL
	File: e_collision.c
	Desc: handles entity collision detection against the world

	Copyright 2013 (c) Sheridan Rathbun, all rights reserved.
	See LICENSE for details.

-------------------------------------------------------------------------------*/

#include "bsdl.h"

/*-------------------------------------------------------------------------------

	e_CheckCells

	Tests all four cells surrounding the position (tx, ty, tz) and returns
	a 1 if the position intersects with a cell.

-------------------------------------------------------------------------------*/

int e_CheckCells( double tx, double ty, double tz, entity_t* me ) {
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
			if( firstentity != NULL && !(me->flags&FLAG_PASSABLE) ) { // check against entities
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

int e_ClipVelocity( double *x, double *y, double *z, double vx, double vy, double vz, entity_t* me ) {
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

	*z=min(max((tz+vz),gfh-STEPHEI),gch);
	if(*z <= gfh) { // on ground?
		if( !me->onground2 ) { // landing from a fall?
			*z=gfh;
			me->onground2=1;
		} else {
			if(*z < gfh) { // climbing a step?
				if( stepclimb || gfh-sfh < STEPHEI ) {
					*z += timesync*.5;
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

void e_MoveTrace( double *x1, double *y1, double *z1, double x2, double y2, double z2, entity_t* me ) {
	double tracex, tracey, tracez, tracex2, tracey2, tracez2;
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

/*-------------------------------------------------------------------------------

	e_LineTrace

	Trace a line from x1, y1, z1 along the provided heading and report
	information on the first obstacle in a hit_t struct

-------------------------------------------------------------------------------*/

hit_t e_LineTrace( entity_t *me, double x1, double y1, double z1, double angle, double vangle ) {
	entity_t *entity;
	hit_t hit;
	int posx, posy;
	double fracx, fracy;
	double cosang, sinang;
	double rx, ry;
	double ix, iy;
	int inx, iny, inz;
	int inx2, iny2;
	double arx, ary;
	double dincx, dval0, dincy, dval1;
	double d;
	double d2;
	double vertfactor;
	
	posx=floor(x1); posy=floor(y1); // integer coordinates
	fracx=x1-posx; fracy=y1-posy; // fraction coordinates
	hx = xres>>1; hy = (yres>>1)+vangle; hz = hx;
	cosang = cos(angle); sinang = sin(angle);
	rx = cosang*2.0;
	ry = sinang*2.0;
	d = 1.0/hz; cosang *= d; sinang *= d; ix=0; iy=0;
	vertfactor = 480.0/yres;

	inx=posx; iny=posy; inz=z1;
	inx2=inx; iny2=iny;
	arx=0; if (rx) arx = 1.0/fabs(rx);
	ary=0; if (ry) ary = 1.0/fabs(ry);
	dincx=0; dval0=1e32; dincy=0; dval1=1e32;
	if (rx<0) {dincx=-1; dval0=fracx*arx;} else if (rx>0) {dincx=1; dval0=(1.0-fracx)*arx;}
	if (ry<0) {dincy=-1; dval1=fracy*ary;} else if (ry>0) {dincy=1; dval1=(1.0-fracy)*ary;}
	d=.1;
	hit.entity = NULL;
	
	// trace the line
	while( d < 64 ) {
		inx2=inx; iny2=iny;
		d2=d;
		if( dval1>dval0 ) { inx+=dincx; d=dval0; dval0+=arx; }
		else { iny+=dincy; d=dval1; dval1+=ary; }
		if( inx < 0 || iny < 0 || inx >= map.width || iny >= map.height ) break;
		
		ix = x1 + rx*d;
		iy = y1 + ry*d;
		inz += (vangle/4.75*vertfactor)*(d-d2);
		
		// check against the map
		if( map.floors[inx+iny*map.width] >= map.ceilings[inx+iny*map.width] ) break;
		if( map.ceilings[inx+iny*map.width] <= inz ) {
			inz = min(map.ceilings[inx2+iny2*map.width],inz);
			break;
		}
		else if( map.floors[inx+iny*map.width] >= inz ) {
			inz = max(map.floors[inx2+iny2*map.width],inz);
			break;
		}
		
		// check against entities
		// to be fixed: line sometimes passes through entities whose x and y sizes are < .5
		if( firstentity != NULL ) {
			for( entity=firstentity; entity!=NULL; entity=entity->next ) {
				if( !(entity->flags&FLAG_PASSABLE) && entity != me ) {
					if( ix >= entity->x-entity->sizex && ix <= entity->x+entity->sizex ) {
						if( iy >= entity->y-entity->sizey && iy <= entity->y+entity->sizey ) {
							if( inz >= entity->z-entity->sizez && inz <= entity->z+entity->sizez ) {
								ix = entity->x;
								iy = entity->y;
								inz = entity->z;
								hit.entity = entity;
								d=64;
							}
						}
					}
				}
			}
		}
	}
	hit.x = ix;
	hit.y = iy;
	hit.z = inz;
	return hit;
}
/*-------------------------------------------------------------------------------

	e_CheckHit
	
	Takes a hit_t struct and spawns a different particle depending upon
	the nature of what was hit

-------------------------------------------------------------------------------*/

void e_CheckHit(hit_t hitspot, int power) {
	if(hitspot.entity != NULL) {
		if(hitspot.entity->behavior == &e_ActChar) { // shot an enemy
			// hurt him
			hitspot.entity->CHAR_HEALTH -= power;
			
			// check that he hasn't yet been shot this frame (to prevent simultaneous pain sounds)
			if(hitspot.entity->CHAR_PAIN <= 0)
				a_EntitySound(hitspot.entity,sounds[3],64);
			hitspot.entity->CHAR_PAIN = 100; // put him in pain
			
			// make a splat
			e_CreateEntity();
			lastentity->behavior = &e_ActSplat;
			lastentity->texture = &sprite_bmp[64];
			lastentity->flags |= FLAG_PASSABLE;
			
			lastentity->sizex = .25;
			lastentity->sizey = .25;
			lastentity->sizez = 16;
			
			lastentity->x=hitspot.x;
			lastentity->y=hitspot.y;
			lastentity->z=hitspot.z;
			lastentity->ang=rand()*360;
			
			lastentity->SPLAT_AIRTIME = 1;
			lastentity->SPLAT_FORCE = 1;
			lastentity->onground=1;
		}	
	}
	else {
		// make a chunk
		e_CreateEntity();
		lastentity->behavior = &e_ActChunk;
		lastentity->texture = &sprite_bmp[65];
		lastentity->flags |= FLAG_PASSABLE;
		
		lastentity->sizex = .25;
		lastentity->sizey = .25;
		lastentity->sizez = 16;
		
		lastentity->x=hitspot.x;
		lastentity->y=hitspot.y;
		lastentity->z=hitspot.z;
	}
}