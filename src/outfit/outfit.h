#ifndef H_OUTFIT
#define H_OUTFIT

#include "includes.h"
#include "sprite/model.h"

#define MAX_OUTFITS 35

typedef struct _weapon_item {
	struct _weapon *weapon;
} weapon_item;

typedef struct _ammo_item {
	struct _ammo *ammo;
} ammo_item;

typedef struct _hull_item {
	float multiplier;
	char *name;
	unsigned char repairing;
} hull_item;

/* death item is an escape pod that launches upon death, if to_nearest_planet is set, the pod goes to the nearest planet; if not, you must pilot it yourself */
typedef struct _death_item {
	model_t *escape_ship;
	unsigned char auto_pilot;
} death_item;

typedef struct _engine_item {
	unsigned char is_booster;
	float boost;
} engine_item;

typedef struct _map_item {
	char *reveal; /* planets to reveal, comma separated, no spaces */
} map_item;

typedef struct _hud_item {
	unsigned char show_target_shield;
	unsigned char show_target_hull;
	unsigned char show_target_shield_units;
	unsigned char show_target_hull_units;
} hud_item;

typedef struct _outfit_item {
	char *name;
	enum _generic_type {ADDITIONAL, MODIFIER} generic;
	enum _specific_type {WEAPON, AMMO, HULL, DEATH, ENGINE, MAP, HUD} specific;
	int needed_free_mass;
	int price;
	char *desc;
	char *stock;
	void *data;	
} outfit_item;

extern outfit_item *outfits[MAX_OUTFITS];
extern int num_outfits;

int load_outfits_eaf(FILE *eaf, char *filename);
int unload_outfits(void);
outfit_item *get_outfit_pointer(char *name);
int player_equip_outfit(outfit_item *item);

#endif /* H_OUTFIT */
