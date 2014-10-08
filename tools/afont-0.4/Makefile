# afont Makefile

# Define one of the following:
SYSTEM=LINUX
#SYSTEM=MINGW32

#USE_OPTIMIZATION=TRUE
#USE_PROFILING=TRUE

# compiler and library flags for the various libraries
ifeq ($(SYSTEM),LINUX)
	FT_CFLAGS=`freetype-config --cflags`
	FT_LIBS=`freetype-config --libs` 
	SDL_CFLAGS=`sdl-config --cflags`
	SDL_LIBS=`sdl-config --libs`
	SDL_TTF_CFLAGS=
	SDL_TTF_LIBS=-lSDL_ttf
	GL_CFLAGS=
	GL_LIBS=-lGL -lGLU

	PROGS=buildfont testafont testglfont benchmark
endif
ifeq ($(SYSTEM),MINGW32)
	FT_CFLAGS=
	FT_LIBS=-lfreetype
	SDL_CFLAGS=-I/mingw/include/SDL/
	SDL_LIBS=-lmingw32 -lSDLmain -lSDL -mwindows
	SDL_TTF_CFLAGS=
	SDL_TTF_LIBS=-lSDL_ttf
	GL_CFLAGS=
	GL_LIBS=-lOPENGL32 -lglu32

	CFLAGS+=-DSYS_IS_BRAINDEAD
	PROGS=buildfont testafont testglfont test1bit
endif

LIB=afont_sdl.a afont_gl.a

ifdef USE_OPTIMIZATION
	CFLAGS+=-O3 -funroll-loops -fomit-frame-pointer
else
	CFLAGS+=-g
endif

ifdef USE_PROFILING
	CFLAGS+=-pg
endif

CC=gcc
AR=ar

all: $(PROGS)

lib: $(LIB)

afont_sdl.a: afont_base.o afont_sdl.o 
	$(AR) rs $@ afont_base.o afont_sdl.o

afont_gl.a: afont_base.o afont_gl.o 
	$(AR) rs $@ afont_base.o afont_gl.o

afont_base.o: afont_base.c
	$(CC) -c $(CFLAGS) afont_base.c

afont_gl.o: afont_gl.c
	$(CC) -c $(CFLAGS) $(GL_CFLAGS) afont_gl.c

afont_sdl.o: afont_sdl.c
	$(CC) -c $(CFLAGS) $(SDL_CFLAGS) afont_sdl.c

benchmark.o: benchmark.c
	$(CC) -c $(CFLAGS) $(SDL_CFLAGS) $(SDL_TTF_CFLAGS) benchmark.c

buildfont.o: buildfont.c
	$(CC) -c $(CFLAGS) $(FT_CFLAGS) buildfont.c

testafont.o: testafont.c
	$(CC) -c $(CFLAGS) $(SDL_CFLAGS) testafont.c

testglfont.o: testglfont.c
	$(CC) -c $(CFLAGS) $(SDL_CFLAGS) $(GL_CFLAGS) testglfont.c

test1bit.o: test1bit.c
	$(CC) -c $(CFLAGS) test1bit.c

testafont: testafont.o afont_base.o afont_sdl.o
	$(CC) $(CFLAGS) -o testafont testafont.o afont_base.o afont_sdl.o \
	$(SDL_LIBS)

testglfont: testglfont.o afont_base.o afont_gl.o
	$(CC) $(CFLAGS) -o testglfont testglfont.o afont_base.o afont_gl.o \
	$(GL_LIBS) $(SDL_LIBS)
	
benchmark: benchmark.o afont_base.o afont_sdl.o
	$(CC) $(CFLAGS) -o benchmark benchmark.o afont_base.o afont_sdl.o \
	$(SDL_LIBS) $(SDL_TTF_LIBS)

buildfont: buildfont.o
	$(CC) $(CFLAGS) -o buildfont buildfont.o $(FT_LIBS) 

test1bit: test1bit.o afont_base.o afont_gl.o
	$(CC) $(CFLAGS) -o test1bit test1bit.o afont_base.o afont_gl.o $(GL_LIBS)

clean:
	rm -f *.o
	rm -f benchmark buildfont testafont testglfont
	rm -f buildfont.exe testafont.exe testglfont.exe test1bit.exe
	rm -f $(LIB)
