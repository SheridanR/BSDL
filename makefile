#################################################################################
#	
#	BUBBENSTEIN/SDL
#	File: mingw32 makefile
#	Desc: The BSDL engine makefile, mingw32 version
#	
#	Copyright 2011 (c) Sheridan Rathbun, all rights reserved.
#	See LICENSE.TXT for details.
#	
#################################################################################

all:
	gcc -I/usr/include/SDL -c -Wall src/bsdl.c -o make/bsdl.o

	gcc -I/usr/include/SDL -c -Wall src/e_list.c -o make/e_list.o
	gcc -I/usr/include/SDL -c -Wall src/e_collision.c -o make/e_collision.o
	gcc -I/usr/include/SDL -c -Wall src/e_behaviors.c -o make/e_behaviors.o
	
	gcc -I/usr/include/SDL -c -Wall src/i_draw.c -o make/i_draw.o
	gcc -I/usr/include/SDL -c -Wall src/i_funcs.c -o make/i_funcs.o
	
	gcc -I/usr/include/SDL -c -Wall src/r_data.c -o make/r_data.o
	gcc -I/usr/include/SDL -c -Wall src/r_draw.c -o make/r_draw.o
	gcc -I/usr/include/SDL -c -Wall src/r_render.c -o make/r_render.o
	
	gcc -I/usr/include/SDL -c -Wall src/g_data.c -o make/g_data.o
	gcc -I/usr/include/SDL -c -Wall src/g_main.c -o make/g_main.o
	
	gcc make/bsdl.o make/e_list.o make/e_collision.o make/e_behaviors.o make/i_draw.o make/i_funcs.o make/r_data.o make/r_draw.o make/r_render.o make/g_data.o make/g_main.o -o bsdl.exe -L/usr/lib -lmingw32 -lsprig -L/usr/local/lib -lSDLmain -lSDL -mwindows