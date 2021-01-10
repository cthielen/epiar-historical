#ifndef H_SPRITE /* prevent conflicts from including this more than once */
#define H_SPRITE

#include "includes.h"
#include "sprite/fire.h"
#include "sprite/gate.h"
#include "sprite/ship.h"
#include "sprite/model.h"
#include "sprite/weapon.h"

#define MAX_ENGINES 20
#define MAX_SHIELDS 20

#define MOMENTUM_CAP 8

#define CYCLE_PRI 0
#define CYCLE_SEC 1
#define CYCLE_FOR 0
#define CYCLE_BAC 1

struct _engine {
	char name[80];
	int acceleration;
	int top_speed;
	unsigned char jump;
};

extern struct _engine engines[MAX_ENGINES];

struct _shield {
	char *name;
	int strength;
};

extern struct _shield shields[MAX_SHIELDS];

/* target structure, used to store information about a target */
struct _target {
	unsigned been_warned;
	int shots_fired;
	struct _ship *offender;
};

extern int num_engines;
extern int num_shields;
extern int num_ships;

extern unsigned char skip_ship_load;

int load_engines_eaf(FILE *eaf, char *filename);
int unload_engines(void);
int load_shields_eaf(FILE *eaf, char *filename);
int unload_shields(void);
int load_ships_eaf(FILE *eaf, char *filename);
int unload_ships(void);
void turn_ship(struct _ship *ship, int right);
int init_player(void);
int uninit_player(void);
void destroy_and_respawn(struct _ship *ship, unsigned char near_player, unsigned char non_destructive);
struct _ship *add_ship(struct _ship *);
void ensure_not_targeted(struct _ship *ship);
struct _engine *get_engine_pointer(char *name);
struct _shield *get_shield_pointer(char *name);
struct _ship *get_ship_pointer(char *name);
void damage_ship(struct _ship *ship, struct _fire *fire); /* called when ship is hurt by weapons fire */
int damage_ship_non_fire(struct _ship *ship, int strength, int angle); /* called when ship is hurt by anything else */
void kill_player(void);
void cloak_ship(struct _ship *ship);
void decloak_ship(struct _ship *ship);
struct _ship *create_ship(void);
void destroy_ship(struct _ship *ship, unsigned char non_destructive);
void init_ships(void);

/* semi-ship related functions */
void hail_ship(struct _ship *ship);
void board_ship(void); /* boards a disabled ship you're on */
void thrust_ship(struct _ship *ship);

/* misc. functions */
unsigned char is_on_screen(struct _ship *ship);

/* player functions */
float get_distance_from_player_sqrd(float x, float y);

void *copy_ship_objective(void *obj, enum _ship_objectives type);

void push_ship_off_radar(struct _ship *ship);

/* use PRI and BAC and those #defines (top of this file) for which and how */
void weapon_cycle(struct _ship *ship, unsigned char which, unsigned char how);

#include "sprite/player.h" /* included here because player.h  */

#endif /* H_SPRITE */
