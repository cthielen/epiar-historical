#include "game/update.h"
#include "includes.h"
#include "sprite/chunk.h"
#include "sprite/particle.h"
#include "system/timer.h"
#include "system/video/video.h"

#define MAX_CHUNKS 20

struct {
  int world_x, world_y; /* world coords */
  int screen_x, screen_y;
  Uint32 expire; /* time to expire */
  SDL_Surface *original; /* the original chunk */
  SDL_Surface *rotated; /* the image to blit (rotated) */
  short int angle; /* angle the chunk should be rotated at */
  float momentum_x, momentum_y;
  int rotate_freq;
  unsigned char on;
} chunks[MAX_CHUNKS];

void init_chunks(void) {
  int i;
  
  for (i = 0; i < MAX_CHUNKS; i++) {
    chunks[i].on = 0;
    chunks[i].momentum_x = chunks[i].momentum_y = chunks[i].angle = chunks[i].world_x = chunks[i].world_y = 0;
    chunks[i].expire = 0;
    chunks[i].original = chunks[i].rotated = NULL;
  }
}

void uninit_chunks(void) {
  int i;
  
  for (i = 0; i < MAX_CHUNKS; i++) {
    chunks[i].on = 0;
    chunks[i].momentum_x = chunks[i].momentum_y = chunks[i].angle = chunks[i].world_x = chunks[i].world_y = 0;
    chunks[i].expire = 0;
    if (chunks[i].original) {
      SDL_FreeSurface(chunks[i].original);
      chunks[i].original = NULL;
    }
    if (chunks[i].rotated) {
      SDL_FreeSurface(chunks[i].rotated);
      chunks[i].rotated = NULL;
    }
  }
}

void create_blast(SDL_Surface *surface, int world_x, int world_y, float given_momentum_x, float given_momentum_y) {
  SDL_Surface *chunk[4];
  SDL_Rect src, dest;
  int i, which = -1, j, a;
  
  assert(surface);
	
  /* construct the four pieces of the surface first */
  /* first chunk */
  src.x = 0;
  src.y = 0;
  src.w = surface->w / 2;
  src.h = surface->h / 2;
  chunk[0] = SDL_CreateRGBSurface(SDL_SWSURFACE, src.w, src.h, surface->format->BitsPerPixel, surface->format->Rmask, surface->format->Gmask, surface->format->Bmask, surface->format->Amask);
  assert(chunk[0]);
  dest.x = 0;
  dest.y = 0;
  dest.w = src.w;
  dest.h = src.h;
  a = SDL_BlitSurface(surface, &src, chunk[0], &dest);
  assert(a == 0);
  /* second chunk */
  src.x = surface->w / 2;
  src.y = 0;
  src.w = surface->w / 2;
  src.h = surface->h / 2;
  chunk[1] = SDL_CreateRGBSurface(SDL_SWSURFACE, src.w, src.h, surface->format->BitsPerPixel, surface->format->Rmask, surface->format->Gmask, surface->format->Bmask, surface->format->Amask);
  assert(chunk[1]);
  dest.x = 0;
  dest.y = 0;
  dest.w = src.w;
  dest.h = src.h;
  a = SDL_BlitSurface(surface, &src, chunk[1], &dest);
  assert(a == 0);
  /* third chunk */
  src.x = 0;
  src.y = surface->h / 2;
  src.w = surface->w / 2;
  src.h = surface->h / 2;
  chunk[2] = SDL_CreateRGBSurface(SDL_SWSURFACE, src.w, src.h, surface->format->BitsPerPixel, surface->format->Rmask, surface->format->Gmask, surface->format->Bmask, surface->format->Amask);
  assert(chunk[2]);
  dest.x = 0;
  dest.y = 0;
  dest.w = src.w;
  dest.h = src.h;
  a = SDL_BlitSurface(surface, &src, chunk[2], &dest);
  assert(a == 0);
  /* fourth chunk */
  src.x = surface->w / 2;
  src.y = surface->h / 2;
  src.w = surface->w / 2;
  src.h = surface->h / 2;
  chunk[3] = SDL_CreateRGBSurface(SDL_SWSURFACE, src.w, src.h, surface->format->BitsPerPixel, surface->format->Rmask, surface->format->Gmask, surface->format->Bmask, surface->format->Amask);
  assert(chunk[3]);
  dest.x = 0;
  dest.y = 0;
  dest.w = src.w;
  dest.h = src.h;
  a = SDL_BlitSurface(surface, &src, chunk[3], &dest);
  assert(a == 0);
  
  for (j = 0; j < 4; j++) {
    which = -1;
    /* find a free chunk for the first one */
    for (i = 0; i < MAX_CHUNKS; i++) {
      if (!chunks[i].on) {
	which = i;
	i = MAX_CHUNKS;
      }
    }
    
    /* no available chunk, so, free image */
    if (which == -1) {
      SDL_FreeSurface(chunk[j]);
      chunk[j] = NULL;
    } else {
      chunks[which].world_x = world_x;
      chunks[which].world_y = world_y;
      chunks[which].screen_x = chunks[which].world_x - camera_x;
      chunks[which].screen_y = chunks[which].world_y - camera_y;
      chunks[which].expire = get_ticks() + 5000; /* five seconds from the time created */
      assert(chunk[j]);
      chunks[which].original = chunk[j];
      chunks[which].angle = rand() % 360; /* not angle of movement, angle for sprite rotation */
      chunks[which].rotate_freq = (rand() % 5) + 1;
      chunks[which].rotated = rotate(chunks[which].original, chunks[which].angle);
      /* assign momentum */
      chunks[which].momentum_x = (rand() % 3) + 2;
      chunks[which].momentum_y = (rand() % 3) + 2;
      /* make sure momentum is logical */
      if (j == 0) {
	if (chunks[which].momentum_x > 0)
	  chunks[which].momentum_x *= -1;
	if (chunks[which].momentum_y > 0)
	  chunks[which].momentum_y *= -1;
      }
      if (j == 1) {
	if (chunks[which].momentum_x < 0)
	  chunks[which].momentum_x *= -1;
	if (chunks[which].momentum_y > 0)
	  chunks[which].momentum_y *= -1;
      }
      if (j == 2) {
	if (chunks[which].momentum_x > 0)
	  chunks[which].momentum_x *= -1;
	if (chunks[which].momentum_y < 0)
	  chunks[which].momentum_y *= -1;
      }
      if (j == 3) {
	if (chunks[which].momentum_x < 0)
	  chunks[which].momentum_x *= -1;
	if (chunks[which].momentum_y < 0)
	  chunks[which].momentum_y *= -1;
      }
      /* add given momentum */
      chunks[which].momentum_x += given_momentum_x;
      chunks[which].momentum_y += given_momentum_y;
      chunks[which].on = 1;
    }
  }
}

void erase_chunks(void) {
	int i;

	for (i = 0; i < MAX_CHUNKS; i++) {
		if (chunks[i].on) {
			SDL_Rect rect;
			assert(chunks[i].rotated);
			rect.x = chunks[i].screen_x - (chunks[i].rotated->w / 2);
			rect.y = chunks[i].screen_y - (chunks[i].rotated->h / 2);
			rect.w = chunks[i].rotated->w;
			rect.h = chunks[i].rotated->h;
			black_fill(rect.x, rect.y, rect.w, rect.h, 0);
		}
	}
}

void update_chunks(void) {
	int i;
	Uint32 current;

	current = get_ticks();

	for (i = 0; i < MAX_CHUNKS; i++) {
		if (chunks[i].on) {
			/* catch the bugs before they happen */
			assert(chunks[i].rotated);
			assert(chunks[i].original);
			if (chunks[i].original->w <= 0)
				printf("chunks[i].original->w = %d\n", chunks[i].original->w);
			if (chunks[i].original->h <= 0)
				printf("chunks[i].original->h = %d\n", chunks[i].original->h);
			assert(chunks[i].original->w > 0);
			assert(chunks[i].original->h > 0);
			assert(chunks[i].angle >= 0);
			assert(chunks[i].angle < 360);
			assert(chunks[i].original->w != 0);
			assert(chunks[i].original->h != 0);

			/* update it */
			chunks[i].world_x += chunks[i].momentum_x;
			chunks[i].world_y += chunks[i].momentum_y;
			chunks[i].screen_x = chunks[i].world_x - camera_x;
			chunks[i].screen_y = chunks[i].world_y - camera_y;
			chunks[i].angle += chunks[i].rotate_freq;
			chunks[i].angle = (int)chunks[i].angle % 360;

			/* free the old chunk picture and get a new one */
			SDL_FreeSurface(chunks[i].rotated);
			chunks[i].rotated = rotate(chunks[i].original, chunks[i].angle);
			assert(chunks[i].rotated);

			/* make a little particle explosion next to it of very few (2) particles (trails) */
			new_explosion(chunks[i].world_x, chunks[i].world_y, 0, 0, 2, 350);

			if (chunks[i].expire < current) {
				assert(chunks[i].original);
				assert(chunks[i].rotated);

				SDL_FreeSurface(chunks[i].original);
				chunks[i].original = NULL;

				SDL_FreeSurface(chunks[i].rotated);
				chunks[i].rotated = NULL;

				/* make one last explosion as the chunk disappears */
				new_explosion(chunks[i].world_x, chunks[i].world_y, chunks[i].momentum_x, chunks[i].momentum_y, 5, 350);
				chunks[i].on = 0;
			}
		}
	}
}

void draw_chunks(void) {
	int i;

	for (i = 0; i < MAX_CHUNKS; i++) {
		if (chunks[i].on) {
			SDL_Rect src, dest;
			assert(chunks[i].rotated);
			src.x = 0;
			src.y = 0;
			src.w = chunks[i].rotated->w;
			src.h = chunks[i].rotated->h;
			dest.x = chunks[i].screen_x - (chunks[i].rotated->w / 2);
			dest.y = chunks[i].screen_y - (chunks[i].rotated->h / 2);
			dest.w = chunks[i].rotated->w;
			dest.h = chunks[i].rotated->h;
			blit_surface(chunks[i].rotated, &src, &dest, 0);
		}
	}
}
