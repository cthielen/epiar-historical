#ifndef _H_SHIP_
#define _H_SHIP_

#include "includes.h"
#include "sprite/model.h"
#include "sprite/weapon.h"

#define MAX_SHIPS           35
#define MAX_OFFENDERS        3
#define MAX_RANDOM_SHIPS    12
#define MAX_SHIP_OBJECTIVES  5

/* status indicators */
#define SHIP_CLOAKING   1
#define SHIP_DECLOAKING 2

/* ship objectives work differently than mission objectives. mission objectives can be completed in any order, */
/* while ship objecives _must_ be done in order */
enum _ship_objectives {NOTHING, FLY_TO, LAND_ON, DESTROY};

typedef struct {
	struct _planet *planet;
} ship_obj_fly_to;

typedef struct {
	struct _planet *planet;
} ship_obj_land_on;

typedef struct {
	struct _ship *ship;
	char who[25];
} ship_obj_destroy;

struct _ai {
	int dest_point[4][2];
	short int dest_heading; /* which dest point we're heading for */
	struct _planet *home; /* defenders base to protect, etc. */
	unsigned char stalking;
};

struct _hull_plating {
	char *name;
	float amt;
	unsigned char repairing;
};

struct _ship {
	char *name; /* some ships in scenarios are given names */
	Uint8 status; /* bitfield with status indicators; cloak and probably jump (in the future) */
	model_t *model;
	struct _engine *engine;
	struct _shield *shield;
	struct _alliance *alliance;
	char *class;
	struct _planet *destination;
	struct _gate *gate;
	struct _target *target; /* if player, this is the auto-target ship, else, it's determined by a.i. */
	struct _ship *offenders[MAX_OFFENDERS]; /* list of the last three ships that shot this ship (not necessarily targets) */
	float world_x, world_y;
	int screen_x;
	int screen_y;
	float velocity;
	float max_velocity;
	float accel;
	float momentum_x, momentum_y;
	int initial_x;
	int initial_y;
	short int angle;
	short int old_angle;
	float shield_life;
	float hull_strength;
	short int boost;
	short int fuel;
	short int w_slot[2];
	unsigned char was_close; /* used by audio code to know whether or not to stop/start a source playback */
	unsigned char has_shield_flare; /* used by shield code to know whether to give another flare to that ship yet or not */
	Uint32 update_interval;
	unsigned char disabled;
	struct _ai ai; /* ai information */
	unsigned char used;
	void *objectives[MAX_SHIP_OBJECTIVES]; /* used mostly for missions */
	enum _ship_objectives obj_types[MAX_SHIP_OBJECTIVES];
	int num_objectives;
	int current_objective;
	Uint32 creation_delay;
	unsigned char landed; /* the ship has landed on a planet and should be not be drawn or collision detected */
	struct _planet *landed_on; /* if landed is true, this should be set to the planet where the ship is on */
	/* variables used in cloaking */
	Uint8 cloak;
	Uint32 cloak_start;
	unsigned char has_cloak; /* whether or not the ship has a cloaking device */
	/* weapons for the ship, can be assigned, bought, and inherited */
	struct _weapon_mount *weapon_mount[MAX_WEAPON_SLOTS];
	model_t *escape_pod;
	struct _hull_plating plating;
	unsigned char has_booster;
	float boost_strength;
	unsigned char respawn; /* determines whether of not ship will respawn when killed */
};

extern struct _ship *ships[MAX_SHIPS];

#endif /* H_SHIP_ */
