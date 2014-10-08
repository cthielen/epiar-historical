#include "includes.h"

void draw_chunks(void); /* ship chunks, not to be confused w/ chunk_blast() */
void update_chunks(void); /* ship chunks, not to be confused w/ chunk_blast() */
void erase_chunks(void); /* ship chunks, not to be confused w/ chunk_blast() */
void create_blast(SDL_Surface *surface, int world_x, int world_y, float given_momentum_x, float given_momentum_y);
void init_chunks(void);
void uninit_chunks(void);
