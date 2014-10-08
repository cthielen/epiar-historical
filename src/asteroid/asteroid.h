#ifndef ASTEROID_H
#define ASTEROID_H

#include "includes.h"

#define MAX_ASTEROIDS 70

struct _asteroids {
  int x, y;
  int momentum_x, momentum_y;
  short int which_point;
  unsigned char on;
  short int life;
  short int frame_num;
};

extern struct _asteroids asteroids[MAX_ASTEROIDS];

/* standard functions - system and drawing */
void init_asteroids(void);
void uninit_asteroids(void);
void update_asteroids(void);
void draw_asteroids(void);
void erase_asteroids(void);
void add_asteroid_field(int x, int y);
int load_asteroids_eaf(FILE *eaf, char *filename);
int unload_asteroids(void);

/* common functions */
void damage_asteroid(int which, int amount);

#endif /* ASTEROID_H */
