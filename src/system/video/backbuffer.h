#include "includes.h"

#define RESOLUTION_X 800
#define RESOLUTION_Y 600
#define TILE_W       32
#define TILE_H       40
#define TILES_ACROSS (RESOLUTION_X / TILE_W)
#define TILES_DOWN   (RESOLUTION_Y / TILE_H)

void initialize_tiles(void);
void dirty_rectangle(int x, int y, int w, int h);
void dirty_pixel(int x, int y);

extern unsigned char dirty_tiles[TILES_ACROSS + 1][TILES_DOWN + 1];
