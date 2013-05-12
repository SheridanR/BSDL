/*-------------------------------------------------------------------------------

	BUBBENSTEIN/SDL
	File: e_list.c
	Desc: contains entity list handling functions.

	Copyright 2013 (c) Sheridan Rathbun, all rights reserved.
	See LICENSE for details.

-------------------------------------------------------------------------------*/

#include "bsdl.h"

/*-------------------------------------------------------------------------------

	e_FreeAll

	Frees the entire entity list and all of its contents.

-------------------------------------------------------------------------------*/

void e_FreeAll(void) {
	entity_t *handle;
	entity_t *nexthandle;
	
	if( firstentity == NULL ) return;
	
	handle=firstentity;
	while(handle!=NULL) {
		nexthandle = handle->next;
		free(handle);
		handle=nexthandle;
	}
	firstentity = NULL;
	lastentity = NULL;
}

/*-------------------------------------------------------------------------------

	e_CreateEntity

	Creates a new entity with empty settings and places it in the list.

-------------------------------------------------------------------------------*/

void e_CreateEntity(void) {
	entity_t* handle;
	int x;
	
	// allocate memory
	if( (handle = (entity_t *) malloc(sizeof(entity_t))) == NULL ) {
		g_Close();
		printf( "ERROR: Failed to allocate memory for new entity!\n\n" );
		exit(1);
	}
	
	if( lastentity != NULL ) {
		lastentity->next = handle; // point the NEXT pointer in the last item to the new data
		handle->previous = lastentity; // point the PREVIOUS pointer in the new item to the last data
	} else {
		handle->previous = NULL; // first one
		firstentity = handle;
	}
	lastentity = handle; // now point the LAST pointer to the new data
	handle->next = NULL;
	
	// now set all of my data elements to ZERO or NULL
	handle->onground = 0;
	handle->onground2 = 0;
	handle->x=0;
	handle->y=0;
	handle->z=0;
	handle->ang=0;
	handle->sizex=0;
	handle->sizey=0;
	handle->sizez=0;
	for( x=0; x<9; x++ ) {
		handle->skill[x]=0;
		handle->fskill[x]=0;
	}
	handle->flags=0;
	handle->texture=NULL;
	handle->behavior=NULL;
}

/*-------------------------------------------------------------------------------

	e_DestroyEntity

	Decouples an entity from the list and frees up the memory it was
	occupying.

-------------------------------------------------------------------------------*/

void e_DestroyEntity(entity_t *handle) {
	if( firstentity == NULL || lastentity == NULL ) return;
	
	// if this is the first node...
	if( handle == firstentity ) {
		// is it also the last node?
		if( lastentity == handle ) {
			free( handle );
			firstentity = NULL;
			lastentity = NULL;
		}
		
		// otherwise, the "first" pointer needs to point to the next node
		else {
			handle->next->previous = NULL;
			firstentity = handle->next;
			free( handle );
		}
	}
	
	// if this is the last node, but not the first...
	else if( handle == lastentity ) {
		handle->previous->next = NULL;
		lastentity = handle->previous; // the "last" pointer needs to point to the previous node
		free( handle );
	}
	
	// if the node is neither first nor last, it is in the middle
	else {
		// bridge the previous node and the first node together
		handle->previous->next = handle->next;
		handle->next->previous = handle->previous;
		free( handle );
	}
}