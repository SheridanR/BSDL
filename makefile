#################################################################################
#	
#	BUBBENSTEIN/SDL
#	File: mingw32 makefile
#	Desc: The BSDL engine makefile, mingw32 version
#	
#	Copyright 2013 (c) Sheridan Rathbun, all rights reserved.
#	See LICENSE for details.
#	
#################################################################################

all:
	gcc -I/usr/include/SDL -c -Wall src/bsdl.c -o bsdl.o

	gcc -I/usr/include/SDL -c -Wall src/e_list.c -o e_list.o
	gcc -I/usr/include/SDL -c -Wall src/e_collision.c -o e_collision.o
	gcc -I/usr/include/SDL -c -Wall src/e_behaviors.c -o e_behaviors.o
	
	gcc -I/usr/include/SDL -c -Wall src/i_draw.c -o i_draw.o
	gcc -I/usr/include/SDL -c -Wall src/i_funcs.c -o i_funcs.o
	
	gcc -I/usr/include/SDL -c -Wall src/r_data.c -o r_data.o
	gcc -I/usr/include/SDL -c -Wall src/r_draw.c -o r_draw.o
	gcc -I/usr/include/SDL -c -Wall src/r_render.c -o r_render.o
	
	gcc -I/usr/include/SDL -c -Wall src/a_sound.c -o a_sound.o
	
	gcc -I/usr/include/SDL -c -Wall src/g_data.c -o g_data.o
	gcc -I/usr/include/SDL -c -Wall src/g_main.c -o g_main.o
	
	gcc bsdl.o e_list.o e_collision.o e_behaviors.o i_draw.o i_funcs.o r_data.o r_draw.o r_render.o a_sound.o g_data.o g_main.o -o bsdl.exe -L/usr/lib -lmingw32 -lsprig -L/usr/local/lib -lSDLmain -lSDL -lSDL_mixer -mwindows
	rm *.o