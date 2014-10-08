#ifndef H_FIRE /* prevent conflicts from including this more than once */
#define H_FIRE

#include "sprite/ship.h"
#include "sprite/weapon.h"

#define MAX_FIRES 100

struct _fire {
	SDL_Surface *surface;
	struct _weapon *weapon;
	struct _ship *owner;
	float world_x, world_y;
	short int screen_x, screen_y;
	int angle; /* angle of the shot */
	int velocity;
	Uint32 expire_time;
} fires[MAX_FIRES];

extern int num_fires;
extern struct _fire *ordered_fires[MAX_FIRES];

/* pass player struct and set ship to null if the player is firing, otherwise, keep player at null and pass ship */
void fire(struct _ship *ship, short int which_mount);
void init_fire(void);
void uninit_fire(void);
void update_fire(void);

#endif /* H_FIRE */
