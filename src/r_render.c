/*-------------------------------------------------------------------------------

	BUBBENSTEIN/SDL
	File: r_render.c
	Desc: renders world data and draws it to the video surface.

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

	r_DrawColumns

	Draws walls as columns of pixels.

-------------------------------------------------------------------------------*/

void r_DrawColumns( double ox, double oy, int oz, double angle, double vangle ) {
	int posx, posy;
	double fracx, fracy;
	double cosang, sinang;
	double rx, ry;
	double ix, iy;
	int inx, iny;
	double arx, ary;
	double dincx, dval0, dincy, dval1;
	double d;
	int sx, sy;
	long tx, ty, ttop, tbottom;
	double hei, hei2;
	long index;
	int side;
	float s;
	long lasttop1, lastbottom1, lastheight1;
	long lasttop2, lastbottom2, lastheight2;
	long lasttop3, lastbottom3;
	long top, bottom;
	//long ltexsize;
	double ftexsize;
	int screenindex;
	//long tomove;

	Uint8 r, g, b;
	Uint8 *p;

	bitmap_t *walltex;

	// floor/ceiling rendering vars
	int ftop1, ftop2;
	int inx2, iny2;
	float d2;

	ty=0; // fixes annoying compiler warning
	posx=floor(ox); posy=floor(oy); // integer coordinates
	fracx=ox-posx; fracy=oy-posy; // fraction coordinates
	hx = xres>>1; hy = (yres>>1)+vangle; hz = hx;
	cosang = cos(angle); sinang = sin(angle);
	rx = (cosang + sinang)*2.0;
	ry = (sinang - cosang)*2.0;
	d = 1.0/hz; cosang *= d; sinang *= d; ix=0; iy=0;

	for( sx=0; sx<xres; sx++ ) { // trace for every column of pixels on the screen
		inx=posx; iny=posy;
		inx2=inx;
		iny2=iny;
		arx=0; if (rx) arx = 1.0/fabs(rx);
		ary=0; if (ry) ary = 1.0/fabs(ry);
		dincx=0; dval0=1e32; dincy=0; dval1=1e32;
		if (rx<0) {dincx=-1; dval0=fracx*arx;} else if (rx>0) {dincx=1; dval0=(1.0-fracx)*arx;}
		if (ry<0) {dincy=-1; dval1=fracy*ary;} else if (ry>0) {dincy=1; dval1=(1.0-fracy)*ary;}
		d=.1;
		lasttop1=0;
		lastbottom1=0;
		lastheight1=map.floors[posx+posy*map.width];
		lasttop2=0;
		lastbottom2=0;
		lastheight2=map.ceilings[posx+posy*map.width];
		lasttop3=0;
		lastbottom3=yres;
		//screenindex=(sx<<2) + gdd.f;
		p = (Uint8 *)screen->pixels + sx * screen->format->BytesPerPixel; // calculate the column we are drawing in
		
		// create initial floor rows
		if( dval1>dval0 ) { inx2+=dincx; d2=dval0; }
		else { iny2+=dincy; d2=dval1; }
		hei2 = texsize/d2;
		
		// determine ceiling heights per pixel
		ftop2 = (hy-hei2*(map.ceilings[inx2+iny2*map.width]-oz))+hei2*(map.ceilings[inx2+iny2*map.width]-map.ceilings[inx+iny*map.width])+1;
		ftop1 = 0;
		ftop2 = min(max(ftop2,0),yres);
		for( sy=ftop1; sy<ftop2; sy++ ) {
			index=inx+iny*map.width;
			if( floorbuffer[sy+sx*yres] == 16383 && map.ceilings[index] != map.floors[index] ) {
				floorbuffer[sy+sx*yres] = map.ceilings[index];
				floorbuffer[sy+sx*yres+vidsize] = map.ceilings_tex2[index];
				rowbuffer[sy] = 1;
			}
		}
		
		// determine floor heights per pixel
		ftop2 = hy-hei2*(map.floors[inx+iny*map.width]-oz)-1;
		ftop1 = yres-1;
		ftop2 = min(max(ftop2,0),yres);
		for( sy=ftop2; sy<ftop1; sy++ ) {
			index=inx+iny*map.width;
			if( floorbuffer[sy+sx*yres] == 16383 && map.ceilings[index] != map.floors[index] ) {
				floorbuffer[sy+sx*yres] = map.floors[index];
				floorbuffer[sy+sx*yres+vidsize] = map.floors_tex2[index];
				rowbuffer[sy] = 1;
			}
		}
		
		while( d < 64 ) {
			if( map.floors[inx+iny*map.width] >= map.ceilings[inx+iny*map.width] ) break;
			if( dval1>dval0 ) { inx+=dincx; d=dval0; dval0+=arx; side=0; }
			else { iny+=dincy; d=dval1; dval1+=ary; side=1; }
			
			// ceiling/floor max draw distance
			inx2=inx;
			iny2=iny;
			if( dval1>dval0 ) { inx2+=dincx; d2=dval0; }
			else { iny2+=dincy; d2=dval1; }
			inx2=min(max(inx2,0),map.width-1);
			iny2=min(max(iny2,0),map.height-1);
			
			if( inx < 0 || iny < 0 || inx >= map.width || iny >= map.height ) break;
			
			ix = ox + rx*d;
			iy = oy + ry*d;
			
			ftexsize = d/texsize;
			hei = texsize/d;
			hei2 = texsize/d2;
			//0.015625
			
			///////////////////
			//  CEILING MAP  //
			///////////////////
			
			ttop = hy-hei*(lastheight2-oz); // top of the wall slice
			if( lasttop2 < ttop || lasttop2 <= 0 ) {
				tbottom = ttop+hei*(lastheight2-map.ceilings[inx+iny*map.width]); // bottom of the wall slice
				
				// determine ceiling heights per pixel
				if( tbottom < hy ) {
					ftop2 = hy-hei2*((map.ceilings[inx2+iny2*map.width]-oz))+hei2*(map.ceilings[inx2+iny2*map.width]-map.ceilings[inx+iny*map.width]);
					ftop1 = min(max(tbottom-1,0),yres);
					ftop2 = min(max(ftop2,0),yres);
					for( sy=ftop1; sy<ftop2; sy++ ) {
						index=inx+iny*map.width;
						if( floorbuffer[sy+sx*yres] == 16383 && map.ceilings[index] != map.floors[index] ) {
							floorbuffer[sy+sx*yres] = map.ceilings[index];
							floorbuffer[sy+sx*yres+vidsize] = map.ceilings_tex2[index];
							rowbuffer[sy] = 1;
						}
					}
				} else {
					ftop2 = hy-hei2*((map.ceilings[inx2+iny2*map.width]-oz))+hei2*(map.ceilings[inx2+iny2*map.width]-map.ceilings[inx+iny*map.width]);
					ftop1 = min(max(tbottom,0),yres);
					ftop2 = min(max(ftop2,0),yres);
					for( sy=ftop1; sy<ftop2; sy++ ) {
						index=inx+iny*map.width;
						if( floorbuffer[sy+sx*yres] == 16383 && map.ceilings[index] != map.floors[index] ) {
							floorbuffer[sy+sx*yres] = map.ceilings[index];
							floorbuffer[sy+sx*yres+vidsize] = map.ceilings_tex2[index];
							rowbuffer[sy] = 1;
						}
					}
				}
				
				if( tbottom > lasttop3 || lasttop3 >= yres ) {
					top=lasttop3;
					lasttop3=tbottom;
					
					if( tbottom < lastbottom2 ) {
						if( lastbottom2 != 0 ) tbottom = lasttop2;
						lastbottom2=tbottom;
					}
					if( ttop < lasttop2 ) lasttop2=ttop;
					
					if( map.ceilings_tex[inx+iny*map.width] >= 0 && map.ceilings_tex[inx+iny*map.width] < texture_num ) {
						walltex = &walltex_bmp[map.ceilings_tex[inx+iny*map.width]];
						if( side ) tx = ix*32;
						else tx = iy*32;
						tx = tx &(walltex->width-1);
						
						// draw ceiling wall slice
						s=pow(darkness,-d)/(side+1);
						top=max(ttop,top);
						bottom=min(tbottom,yres);
						screenindex = top*screen->pitch;
						//tomove = walltex->height-1;
						for( sy=top; sy<bottom; sy++ ) {
							/*asm (
								//ty = sy-ttop;
								"mov eax, sy\n"
								"sub eax, ttop\n"
								"mov ty, eax\n"
								
								//ty *= ftexsize;
								"fld	ftexsize\n"
								"fimul ty\n"
								"fistp ty\n"
								
								//ty = ty &(walltex->height-1);
								"mov	eax, tomove\n"
								"and	ty, eax\n"
							);*/
							
							if( zbuffer[sy+sx*yres] == 0 || zbuffer[sy+sx*yres] > d ) { // draw onto the screen and the zbuffer
								ty = ((int)((sy-ttop)*ftexsize))&(walltex->height-1);
								index = (tx+ty*walltex->width)<<2;
								r = walltex->data[index];
								g = walltex->data[index+1];
								b = walltex->data[index+2];
								
								/*asm (
									//shade the RED component
									"fld	s\n\t"
									"fimul r\n\t"
									"fistp r\n\t"
									
									//shade the GREEN component
									"fld	s\n\t"
									"fimul g\n\t"
									"fistp g\n\t"
									
									//shade the BLUE component
									"fld	s\n\t"
									"fimul b\n\t"
									"fistp b\n\t"
								);*/
								r *= s;
								g *= s;
								b *= s;
								
								*(Uint32 *)((Uint8 *)p + screenindex)=SDL_MapRGB( screen->format, r, g, b ); // draw a pixel
								zbuffer[sy+sx*yres] = d; // modify the zbuffer
							}
							screenindex += screen->pitch;
						}
					}
					else
						drawsky=1; // sky is visible in this frame
				}
			}
			lastheight2=map.ceilings[inx+iny*map.width];
			
			///////////////////
			//   FLOOR MAP   //
			///////////////////
			
			//ftemp = hy;
			//ftemp -= hei*(map.floors[inx+iny*map.width]-oz);
			if( map.ceilings[inx+iny*map.width] != map.floors[inx+iny*map.width] )
				ttop = hy-hei*(map.floors[inx+iny*map.width]-oz); // top of the wall slice
			else
				ttop = tbottom;
			
			// determine floor heights per pixel
			if( ttop > hy ) {
				ftop2 = hy-hei2*(map.floors[inx+iny*map.width]-oz)-1;
				ftop1 = min(max(ttop,0),yres);
				ftop2 = min(max(ftop2,0),yres);
				for( sy=ftop2; sy<ftop1; sy++ ) {
					index=inx+iny*map.width;
					if( floorbuffer[sy+sx*yres] == 16383 && map.ceilings[index] != map.floors[index] ) {
						floorbuffer[sy+sx*yres] = map.floors[index];
						floorbuffer[sy+sx*yres+vidsize] = map.floors_tex2[index];
						rowbuffer[sy] = 1;
					}
				}
			} else {
				ftop2 = hy-hei2*(map.floors[inx+iny*map.width]-oz);
				ftop1 = min(max(ttop,0),yres);
				ftop2 = min(max(ftop2,0),yres);
				for( sy=ftop2; sy<ftop1; sy++ ) {
					index=inx+iny*map.width;
					if( floorbuffer[sy+sx*yres] == 16383 && map.ceilings[index] != map.floors[index] ) {
						floorbuffer[sy+sx*yres] = map.floors[index];
						floorbuffer[sy+sx*yres+vidsize] = map.floors_tex2[index];
						rowbuffer[sy] = 1;
					}
				}
			}
			
			if( ttop < lastbottom3 || lastbottom3 <= 0 ) {
				bottom=lastbottom3;
				lastbottom3=ttop;
				if( ttop > lasttop1 && lasttop1 != 0 ) ttop = lastbottom1;
				if( ttop < lasttop1 ) lasttop1=ttop;
				//tbottom = ttop+hei*(map.floors[inx+iny*map.width]-lastheight1); // bottom of the wall slice
				tbottom = hy-hei*(lastheight1-oz);
				
				if( tbottom < lastbottom1 ) {
					if( lastbottom1 != 0 ) tbottom = lasttop1;
					lastbottom1=tbottom;
				}
				
				if( map.floors_tex[inx+iny*map.width] >= 0 && map.floors_tex[inx+iny*map.width] < texture_num ) {
					walltex = &walltex_bmp[map.floors_tex[inx+iny*map.width]];
					if( side ) tx = ix*32;
					else tx = iy*32;
					tx = tx &(walltex->width-1);
					
					// draw floor wall slice
					s=pow(darkness,-d)/(side+1);
					top=max(ttop,0);
					bottom=min(tbottom,bottom);
					screenindex=top*screen->pitch;
					//tomove = walltex->height-1;
					for( sy=top; sy<bottom; sy++ ) {
						/*asm(".intel_syntax noprefix\n");
						__asm
						(
							//ty = sy-ttop;
							"mov eax, sy\n"
							"sub eax, ttop\n"
							"mov ty, eax\n"
							
							//ty *= ftexsize;
							"fld	ftexsize\n"
							"fimul ty\n"
							"fistp ty\n"
							
							//ty = ty &(walltex->height-1);
							"mov	eax, tomove\n"
							"and	ty, eax\n"
						);*/
						if( zbuffer[sy+sx*yres] == 0 || zbuffer[sy+sx*yres] > d ) { // draw onto the screen and the zbuffer
							ty = ((int)((sy-ttop)*ftexsize))&(walltex->height-1);
							index = (tx+ty*walltex->width)<<2;
							r = walltex->data[index];
							g = walltex->data[index+1];
							b = walltex->data[index+2];
							
							/*asm (
								//shade the RED component
								"fld	_s\n\t"
								"fimul r\n\t"
								"fistp r\n\t"
								
								//shade the GREEN component
								"fld	s\n\t"
								"fimul g\n\t"
								"fistp g\n\t"
								
								//shade the BLUE component
								"fld	s\n\t"
								"fimul b\n\t"
								"fistp b"
							);*/
							/*asm ("fld %1\n\t"
								"fimul %0\n\t"
								"fistp %0"
								: "=p" (r)
								: "p" (r), "m" (s)
								: "0");*/
							r *= s;
							g *= s;
							b *= s;
							
							*(Uint32 *)((Uint8 *)p + screenindex)=SDL_MapRGB( screen->format, r, g, b ); // draw a pixel
							zbuffer[sy+sx*yres] = d; // modify the zbuffer
						}
						screenindex += screen->pitch;
					}
				}
				else
					drawsky=1; // sky is visible in this frame
			}
			lastheight1=map.floors[inx+iny*map.width];
			
			if( lasttop3 >= yres-1 || ttop <= 0 ) break;
		}
		
		// adjust ray angle for next column
		rx-=sinang*2; ry+=cosang*2;
	}
	//frametime=klock()-frametime;
}

/*-------------------------------------------------------------------------------

	r_DrawFloors

	Draws floor and ceiling spans in the level. Must be called after
	r_DrawColumns, which generates distance data for each section.

-------------------------------------------------------------------------------*/

void r_DrawFloors( double ox, double oy, int oz, double angle, double vangle ) {
	float d;
	float invRes;
	float parallel;
	float vertangle;
	int currentfloor;
	int camHeight;
	int horizon, horizon2;

	float u1, v1, u2, v2;
	float tU, tV, dU, dV;
	float cosmparallel, sinmparallel;
	float cospparallel, sinpparallel;
	//unsigned long flrx, flry;

	float s;
	long index;
	int x, y;
	long tex;
	//int screenindex;
	Uint8 r, g, b;
	Uint8 *p;

	camHeight = yres>>1;
	vertangle = camHeight + vangle;
	horizon = vertangle;
	horizon2 = min(max(horizon,0),yres);
	invRes =  1.0 / xres;
	parallel = 45.0 * PI / 180.0;
	ox *= 32;
	oy *= 32;

	cosmparallel = cos(angle - parallel);
	sinmparallel = sin(angle - parallel);
	cospparallel = cos(angle + parallel);
	sinpparallel = sin(angle + parallel);

	/*                   */
	/*  CEILING DRAWING  */
	/*                   */

	for(y = 0; y < horizon2; y++) {
		if( !rowbuffer[y]) continue; // skip this row, it doesn't need to be drawn
		currentfloor = floorbuffer[y];
		d = (camHeight / (y - vertangle)) * (oz-currentfloor) * 1.875;
		
		// calculate the coordinates at the periphery of the frustum
		u1 = cosmparallel * d;
		v1 = sinmparallel * d;
		u2 = cospparallel * d;
		v2 = sinpparallel * d;
		
		u1 += ox;
		v1 += oy;
		u2 += ox;
		v2 += oy;
		
		// the change in the coordinates across the scanline
		dU = (u2 - u1) * invRes;
		dV = (v2 - v1) * invRes;
		tU = u1;
		tV = v1;
		
		//d = hypot( ox-u1, oy-v1 );
		d *= .011;
		s = pow(darkness,-d)/2;
		//screenindex = (gdd.p*y) + gdd.f;
		p = (Uint8 *)screen->pixels + y * screen->pitch; // calculate the row we are drawing in
		
		for(x = 0; x < xres; x++) {
			if( floorbuffer[y+x*yres] != 16383 ) {
				if( floorbuffer[y+x*yres] != currentfloor || oz-currentfloor>0 ) {
					currentfloor = floorbuffer[y+x*yres];
					d = (camHeight / (y - vertangle)) * (oz-currentfloor) * 1.875;
					if(oz>currentfloor) {
						tU += dU;
						tV += dV;
						continue;
					}
					
					// calculate the coordinates at the periphery of the frustum
					u1 = cosmparallel * d;
					v1 = sinmparallel * d;
					u2 = cospparallel * d;
					v2 = sinpparallel * d;
					
					u1 += ox;
					v1 += oy;
					u2 += ox;
					v2 += oy;
					
					// the change in the coordinates across the scanline
					dU = (u2 - u1) * invRes;
					dV = (v2 - v1) * invRes;
					tU = u1 + dU*(x-1);
					tV = v1 + dV*(x-1);
					
					//d = hypot( ox-u1, oy-v1 );
					d *= .011;
					s = pow(darkness,-d)/2;
				}
				if( d < zbuffer[y+x*yres] || !zbuffer[y+x*yres] ) {
					//flrx=min(max( ((unsigned long)tU)>>5, 0),map.width-1);
					//flry=min(max( ((unsigned long)tV)>>5, 0),map.height-1);
					//tex = map.ceilings_tex2[flrx+flry*map.width];
					tex = floorbuffer[y+x*yres+vidsize];
					if( tex >= 0 && tex < texture_num ) {
						index = ((((unsigned long)tV&(walltex_bmp[tex].height-1))*walltex_bmp[tex].width)+((unsigned long)tU&(walltex_bmp[tex].width-1)))<<2;
						
						r = walltex_bmp[tex].data[index];
						g = walltex_bmp[tex].data[index+1];
						b = walltex_bmp[tex].data[index+2];
						/*asm(".intel_syntax noprefix\n");
						__asm
						(
							//shade the RED component
							"fld	s\n"
							"fimul r\n"
							"fistp r\n"
							
							//shade the GREEN component
							"fld	s\n"
							"fimul g\n"
							"fistp g\n"
							
							//shade the BLUE component
							"fld	s\n"
							"fimul b\n"
							"fistp b\n"
						);*/
						r *= s;
						g *= s;
						b *= s;
						
						*(Uint32 *)((Uint8 *)p)=SDL_MapRGB( screen->format, r, g, b ); // draw a pixel
						zbuffer[y+x*yres] = d;
					}
					else
						drawsky=1;
				}
			}
			tU += dU;
			tV += dV;
			p+=screen->format->BytesPerPixel;
		}
	}
	
	/*                 */
	/*  FLOOR DRAWING  */
	/*                 */
	
	for(y = horizon2; y < yres; y++) {
		if( !rowbuffer[y] ) continue; // skip this row, it doesn't need to be drawn
		currentfloor = floorbuffer[y];
		d = (camHeight / (y - vertangle)) * (oz-currentfloor) * 1.875;
		
		// calculate the coordinates at the periphery of the frustum
		u1 = cosmparallel * d;
		v1 = sinmparallel * d;
		u2 = cospparallel * d;
		v2 = sinpparallel * d;
		
		u1 += ox;
		v1 += oy;
		u2 += ox;
		v2 += oy;
		
		// the change in the coordinates across the scanline
		dU = (u2 - u1) * invRes;
		dV = (v2 - v1) * invRes;
		tU = u1;
		tV = v1;
		
		//d = hypot( ox-u1, oy-v1 );
		d *= .011;
		s = pow(darkness,-d);
		//screenindex = (gdd.p*y) + gdd.f;
		p = (Uint8 *)screen->pixels + y * screen->pitch; // calculate the row we are drawing in
		
		for(x = 0; x < xres; x++) {
			if( floorbuffer[y+x*yres] != 16383 ) {
				if( floorbuffer[y+x*yres] != currentfloor || oz-currentfloor<0 ) {
					currentfloor = floorbuffer[y+x*yres];
					d = (camHeight / (y - vertangle)) * (oz-currentfloor) * 1.875;
					if(oz<currentfloor) {
						tU += dU;
						tV += dV;
						continue;
					}
					
					// calculate the coordinates at the periphery of the frustum
					u1 = cosmparallel * d;
					v1 = sinmparallel * d;
					u2 = cospparallel * d;
					v2 = sinpparallel * d;
					
					u1 += ox;
					v1 += oy;
					u2 += ox;
					v2 += oy;
					
					// the change in the coordinates across the scanline
					dU = (u2 - u1) * invRes;
					dV = (v2 - v1) * invRes;
					tU = u1 + dU*(x-1);
					tV = v1 + dV*(x-1);
					
					//d = hypot( ox-u1, oy-v1 );
					d *= .011;
					s = pow(darkness,-d);
				}
				if( d < zbuffer[y+x*yres] || !zbuffer[y+x*yres] ) {
					//flrx=min(max( ((unsigned long)tU)>>5, 0),map.width-1);
					//flry=min(max( ((unsigned long)tV)>>5, 0),map.height-1);
					//tex = map.floors_tex2[flrx+flry*map.width];
					tex = floorbuffer[y+x*yres+vidsize];
					if( tex >= 0 && tex < texture_num ) {
						index = ((((unsigned long)tV&(walltex_bmp[tex].height-1))*walltex_bmp[tex].width)+((unsigned long)tU&(walltex_bmp[tex].width-1)))<<2;
						
						r = walltex_bmp[tex].data[index];
						g = walltex_bmp[tex].data[index+1];
						b = walltex_bmp[tex].data[index+2];
						/*asm (
							//shade the RED component
							"fld	s\n"
							"fimul r\n"
							"fistp r\n"
							
							//shade the GREEN component
							"fld	s\n"
							"fimul g\n"
							"fistp g\n"
							
							//shade the BLUE component
							"fld	s\n"
							"fimul b\n"
							"fistp b\n"
						);*/
						r *= s;
						g *= s;
						b *= s;
						
						*(Uint32 *)((Uint8 *)p)=SDL_MapRGB( screen->format, r, g, b ); // draw a pixel
						zbuffer[y+x*yres] = d;
					}
					else
						drawsky=1;
				}
			}
			tU += dU;
			tV += dV;
			p += screen->format->BytesPerPixel;
		}
	}
}

/*-------------------------------------------------------------------------------

	r_DrawSprites

	Draws all entities in the level.

-------------------------------------------------------------------------------*/

void r_DrawSprites( double ox, double oy, int oz, double angle, double vangle ) {
	float d, dx, dy; // distance variables
	long sx, sy; // screen position variables
	float ax, ay; // angular variables
	float cosang, sinang;

	entity_t* sprites = firstentity;

	// drawing variables
	int x, y, x1, x2, y1, y2;
	int x3, x4, y3, y4;
	int picx, picy;
	long index;
	int screenindex;
	int width, height, size;
	float s, sizefactor;
	Uint8 r, g, b;
	Uint8 *p;
	
	// don't run the function if there are no entities.
	if( firstentity == NULL )
		return;

	index=0; // fixes compiler warning
	cosang = cos( angle );
	sinang = sin( angle );
	hx = xres>>1; hy = (yres>>1)+vangle; hz = hx;

	for( sprites=firstentity; sprites!=NULL; sprites=sprites->next) {
		// if the entity has no texture, skip to the next one.
		if( sprites->texture == NULL )
			continue;
		
		// first, calculate the distance between the player and the sprite
		dx = sprites->x-ox;
		dy = oy-sprites->y;
		d = sqrt(dx*dx + dy*dy)*.5;
		
		// get the onscreen direction to the sprite
		ax = (dx*(sinang*.25))+(dy*(cosang*.25));
		ay = (dx*(cosang*.25))-(dy*(sinang*.25));
		if( ay <= 0 ) continue;
		d *= cos(ax/(d*.5)); // correct the "fishbowl" effect
		if( d < .05 ) continue; // sprite is too close
		s = pow(darkness,-d);
		
		// get the onscreen position of the sprite
		sx = (ax*(hx/ay)*-1)+hx; // onscreen position x
		sy = hy+((((oz-sprites->z)*.66667)*0.015625)/d)*yres; // onscreen position y
		
		width = sprites->texture->width;
		height = sprites->texture->height;
		x1 = sx-ceil(width*.5)/d*sprsize;
		x2 = sx+floor(width*.5)/d*sprsize;
		y1 = sy-ceil(height*.5)/d*sprsize;
		y2 = sy+floor(height*.5)/d*sprsize;
		//i_PrintText( 4, 4, "%d", floor(width*.5)
		
		size = width*height;
		sizefactor = d/sprsize;
		
		// draw the sprite
		x3=max(x1,0);
		x4=min(x2,xres);
		y3=max(y1,0);
		y4=min(y2,yres);
		for( y=y3; y<y4; y++ ) {
			p = (Uint8 *)screen->pixels + y * screen->pitch; // calculate the row we are drawing in
			picy = (y-y1)*sizefactor;
			screenindex=x3*screen->format->BytesPerPixel;
			for( x=x3; x<x4; x++ ) {
				if( zbuffer[y+x*yres] >= d || !zbuffer[y+x*yres] ) {
					picx = (x-x1)*sizefactor;
					/*asm (
					"fld	$0,$d\n"
					"fimul $0,$picx\n"
					"fdiv  $0,$sprsize\n"
					"fistp $0,$picx\n"
					
					"fld	$0,$d\n"
					"fimul $0,$picy\n"
					"fdiv  $0,$sprsize\n"
					"fistp $0,$picy\n"
					
					"mov $picx, %eax\n"
					"mul $width\n"
					"add $picy, %eax\n"
					"mov %eax, index\n"
					);*/
					index = picx+picy*width;
					
					if( index >= 0 && index < size ) {
						index *= 4;
						r = sprites->texture->data[index];
						g = sprites->texture->data[index+1];
						b = sprites->texture->data[index+2];
						if( r || g<255 || b<255 ) {
							//*(long *)(screenindex+(x<<2)) = colors;
							r *= s;
							g *= s;
							b *= s;
							/*asm (
								//shade the RED component
								"fld	s\n"
								"fimul r\n"
								"fistp r\n"
								
								//shade the GREEN component
								"fld	s\n"
								"fimul g\n"
								"fistp g\n"
								
								//shade the BLUE component
								"fld	s\n"
								"fimul b\n"
								"fistp b\n"
							);*/
							zbuffer[y+x*yres] = d;
							//SDL_PutPixel( screen, x, y, SDL_MapRGB( screen->format, r, g, b ) );
							*(Uint32 *)((Uint8 *)p + screenindex)=SDL_MapRGB( screen->format, r, g, b ); // draw a pixel
						}
					}
				}
				screenindex+=screen->format->BytesPerPixel;
			}
		}
	}
}