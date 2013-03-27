/*-------------------------------------------------------------------------------

	BUBBENSTEIN/SDL
	File: r_draw.c
	Desc: contains special drawing functions

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

	r_DrawWeapons

	Draws the player's first person view weapons.

-------------------------------------------------------------------------------*/

int sway, swingmode;
double swing;

void r_DrawWeapons(void) {
	float screenfactor;
	long x, y, index;
	long picx, picy, size;
	long x1, y1, x2, y2, x3, y3, x4, y4;
	Uint8 r, g, b;
	Uint8 *p;
	bitmap_t *gun_bmp;
	
	// move the images differently depending upon the screen size
	screenfactor = ((float)(xres))/320.0;
	
	// select the bitmap and generate the offset
	switch( selected_weapon ) {
		case 2:
			gun_bmp = &pistol_bmp[weap_anim];
			gunx = xres/2;
			guny = yres-(gun_bmp->height)*screenfactor;
			break;
		case 3:
			gun_bmp = &shotgun_bmp[weap_anim];
			gunx = xres/2;
			guny = yres-(gun_bmp->height)*screenfactor;
			break;
	}
	size = gun_bmp->width*gun_bmp->height;
	//gunx += bob2*screenfactor;
	//guny += (24+bob2+weap_swap[0])*screenfactor+vang*.05;
	
	if( keystatus[SDLK_w] || keystatus[SDLK_a] || keystatus[SDLK_s] || keystatus[SDLK_d] ) {
		if( swing < 0 ) {
			swingmode = 0;
			swing = 0;
		}
		if( swing > 180 ) {
			swingmode = -2;
			swing = 180;
		}
		swing += (swingmode+1)*timesync*.5;
		sway += timesync;
	}
	sway *= max(1-.01*timesync,0);
	if(!weap_anim) {
		gunx += sway*cos(swing*PI/180)*.25;
		guny += (weap_swap[0]+4)*screenfactor+sway*sin(swing*PI/180)*.25;
	}
	else {
		gunx += sway*cos(swing*PI/180)*.125;
		guny += (weap_swap[0]+4)*screenfactor+sway*sin(swing*PI/180)*.125;
	}
	
	x1 = gunx-(gun_bmp->width*screenfactor*.5);
	x2 = gunx+(gun_bmp->width*screenfactor*.5);
	y1 = guny;
	y2 = guny+(gun_bmp->height*screenfactor);
	
	// draw the weapon
	x3=max(x1,0);
	x4=min(x2,xres);
	y3=max(y1,0);
	y4=min(y2,yres);
	for( y=y3; y<y4; y++ ) {
		p = (Uint8 *)screen->pixels + y * screen->pitch; // calculate the column we are drawing in
		picy = (y-y1)/screenfactor;
		for( x=x3; x<x4; x++ ) {
			picx = (x-x1)/screenfactor;
			index = (picx+picy*gun_bmp->width);
			if( index >= 0 && index < size ) {
				index *= 4;
				r = gun_bmp->data[index];
				g = gun_bmp->data[index+1];
				b = gun_bmp->data[index+2];
				if( r || g!=255 || b!=255 )
					*(Uint32 *)((Uint8 *)p + x * screen->format->BytesPerPixel)=SDL_MapRGB( screen->format, r, g, b ); // draw a pixel
			}
		}
	}
}

/*-------------------------------------------------------------------------------

	r_DrawSky

	Draws the sky as an image whose position depends upon the given angles.

-------------------------------------------------------------------------------*/

void r_DrawSky( double angle, double vangle ) {
	float screenfactor;
	int skyx, skyy;
	SDL_Rect dest;
	SDL_Rect src;
	
	// move the images differently depending upon the screen size
	screenfactor = ((float)(xres))/320.0;

	// bitmap offsets
	skyx = -angle*((320*screenfactor)/(PI/2.0));
	skyy = (-114*screenfactor+vangle);

	src.x = -skyx;
	src.y = -skyy;
	src.w = (-skyx)+320*screenfactor; // clip to the screen width
	src.h = (-skyy)+240*screenfactor; // clip to the screen height
	dest.x = 0;
	dest.y = 0;
	
	SDL_BlitSurface(sky_bmp, &src, screen, &dest);
	
	// draw the part of the last part of the sky (only appears when angle > 270 deg.)
	if( skyx < -960*screenfactor ) {
		dest.x = 1280*screenfactor+skyx;
		dest.y = 0;
		src.x = 0;
		src.y = -skyy;
		src.w = 320*screenfactor-(-skyx-1280*screenfactor);
		src.h = src.y+240*screenfactor;
		SDL_BlitSurface(sky_bmp, &src, screen, &dest);
	}
}