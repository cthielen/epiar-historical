#include "includes.h"
#include "system/video/backbuffer.h"

/* used to know what video area to update */
unsigned char dirty_tiles[TILES_ACROSS + 1][TILES_DOWN + 1];

void initialize_tiles(void) {
	memset(dirty_tiles, 0, sizeof(dirty_tiles));
}

void dirty_rectangle(int x, int y, int w, int h) {
	int i, j;
	int inital_x, inital_y;
	int final_x, final_y;

	assert(x >= 0);
	assert(y >= 0);
	assert(x <= 799);
	assert(y <= 599);

	inital_x = (x / TILE_W);
	inital_y = (y / TILE_H);

	final_x = (x + w - 1) / TILE_W;
	final_y = (y + h - 1) / TILE_H;

	for (j = inital_y; j <= final_y; j++) {
		for (i = inital_x; i <= final_x; i++) {
			assert(i <= (TILES_ACROSS + 1));
			assert(j <= (TILES_DOWN + 1));
			dirty_tiles[i][j] = 1;
		}
	}
}

void dirty_pixel(int x, int y) {
	assert((x / TILE_W) <= (TILES_ACROSS + 1));
	assert((y / TILE_H) <= (TILES_DOWN + 1));
	dirty_tiles[x / TILE_W][y / TILE_H] = 1;
}
