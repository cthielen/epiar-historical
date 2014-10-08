#ifndef GATE_H
#define GATE_H

#include "includes.h"
#include "sprite/planet.h"

#define MAX_GATES 25

struct _gate {
	char *name;
	char *top_image, *bottom_image;
	int world_x, world_y;
	int screen_x, screen_y;
	int num_defenders;
	int angle;
	struct _gate *destination;
	char *dest_name;
	SDL_Surface *top_surface, *bottom_surface;
};

struct _jump {
	struct _gate *gate;
	int angle;
	int x, y;
	int gate_x, gate_y;
	int old_dist;
	int old_momentum_x, old_momentum_y; /* ship's momentum is restored to these values after jump */
	int stage;
	Uint32 time;
};

extern struct _gate *gates[MAX_GATES];
extern int num_gates;

int init_gates_eaf(FILE *eaf, char *filename);
void unload_gates(void);
struct _jump *create_jump(struct _gate *gate);
int hail_gate(struct _gate *gate);

#endif
