#include "ai/ai.h"
#include "alliances/alliances.h"
#include "com_defs.h"
#include "comm/comm.h"
#include "force/force.h"
#include "game/game.h"
#include "game/update.h"
#include "gui/gui.h"
#include "hud/hud.h"
#include "input/input.h"
#include "missions/missions.h"
#include "outfit/outfit.h"
#include "sprite/chunk.h"
#include "sprite/flare.h"
#include "sprite/particle.h"
#include "sprite/player.h"
#include "sprite/r_ships.h"
#include "sprite/sprite.h"
#include "sprite/target.h"
#include "sprite/model.h"
#include "sprite/weapon.h"
#include "system/debug.h"
#include "system/eaf.h"
#include "system/esf.h"
#include "system/font.h"
#include "system/math.h"
#include "system/path.h"
#include "system/rander.h"
#include "system/timer.h"
#include "system/trig.h"
#include "system/video/video.h"

#define MAX_CHUNKS 35 /* maximum # of chunks to fly off the hull right before death (not the explosion #) */

int num_engines = 0;
int num_shields = 0;
int num_ships   = 0;
unsigned char skip_ship_load = 0;
/* for player init */
model_t *player_model = NULL;
int player_x = 0, player_y = 0;

struct _ship *ships[MAX_SHIPS];

enum _ship_type {WEAK, TRADER, MILITARY, PIRATE};

gui_window *board_win;
gui_session *hail_session, *board_session;
gui_button *take_cash, *steal_ship, *abandon_btn;
void hail_close_btn_handle(void);
void take_cash_handle(void);
void steal_ship_handle(void);
void abandon_btn_handle(void);

/* objective based functions */
static void *ship_new_obj_fly_to(char *planet);
static void *ship_new_obj_land_on(char *planet);
static void *ship_new_obj_destroy(char *ship);

static void free_ship(struct _ship *ship);

/* returns a pointer to the created ship, although it can be ignored since the a.i. code will take over all created ships */
struct _ship *add_ship(struct _ship *ship) {
	if (num_ships < MAX_SHIPS) {
		int i;
		int slot = -1;

		for (i = 0; i < MAX_SHIPS; i++) {
			if (ships[i] == NULL) {
				slot = i;
				break;
			}
		}
		if (slot == -1)
			return (NULL);

		ships[slot] = ship;
		/* ensure all coordinates are correct */
		ships[slot]->world_x = ships[slot]->initial_x;
		ships[slot]->world_y = ships[slot]->initial_y;
		ships[slot]->screen_x = get_screen_coords(ships[slot]->world_x, ships[slot]->world_y, 1);
		ships[slot]->screen_y = get_screen_coords(ships[slot]->world_x, ships[slot]->world_y, 0);
		assert(ships[slot]->model);
		assert(ships[slot]->model->default_shield);
		assert(ships[slot]->model->default_engine);
		ships[slot]->shield = ships[slot]->model->default_shield;
		ships[slot]->engine = ships[slot]->model->default_engine;
		ships[slot]->shield_life = ships[slot]->model->default_shield->strength;
		ships[slot]->hull_strength = ships[slot]->model->hull_life;
		ships[slot]->max_velocity = (float)ships[slot]->model->default_engine->top_speed / 2000.0f;
		ships[slot]->fuel = ships[slot]->model->total_fuel;

		/* add inhertied weapon mounts */
		for (i = 0; i < MAX_WEAPON_SLOTS; i++) {
			if (ships[slot]->model->default_mounts[i]) {
				int j, w_slot = -1;

				/* find a free weapon mount */
				for (j = 0; j < MAX_WEAPON_SLOTS; j++) {
					if (!ships[slot]->weapon_mount[j]) {
						w_slot = j;
						break;
					}
				}

				/* no free slot for inheritance */
				if (w_slot == -1)
					break;

				/* add the weapon to 'slot' */
				ships[slot]->weapon_mount[w_slot] = copy_weapon_mount(ships[slot]->model->default_mounts[i]);
			} else {
				break;
			}
		}

		/* setup which weapons are equipped */
		for (i = 0; i < 2; i++) {
			if (ships[slot]->weapon_mount[i])
				ships[slot]->w_slot[i] = i;
			else
				ships[slot]->w_slot[i] = -1;
		}

		/* set the last one (after all objectives) to nothing */
		ship->objectives[ship->num_objectives] = NULL;
		ship->obj_types[ship->num_objectives] = NOTHING;
		
		num_ships++;

		return (ship);
	} else {
		printf("Error: Too many ships, cannot add any more\n");
	}

	return (NULL);
}

static void add_engine(char *name, int acceleration, int top_speed, unsigned char jump) {
	if (num_engines < MAX_ENGINES) {
		strcpy(engines[num_engines].name, name);
		engines[num_engines].acceleration = acceleration;
		engines[num_engines].top_speed = top_speed;
		engines[num_engines].jump = jump;
		num_engines++;
	}
}

int load_engines_eaf(FILE *eaf, char *filename) {
	char *name;
	int acceleration;
	int top_speed;
	unsigned char jump;
	parsed_file *engines_esf = NULL;

	engines_esf = esf_new_handle();
	assert(engines_esf);

	if (esf_set_filter(engines_esf, "engine") != 0) {
		printf("Couldn't set engine filter\n");
	}

	if (esf_parse_file_eaf(eaf, engines_esf, filename) != 0) {
		printf("Coudln't parse \"%s\"\n", filename);
	} else {
		int i, j;

		for (i = 0; i < engines_esf->num_items; i++) {
			for (j = 0; j < engines_esf->items[i].num_keys; j++) {
				char *key_name = engines_esf->items[i].keys[j].name;

				if (!strcmp(key_name, "Name")) {
					name = engines_esf->items[i].keys[j].value.cp;
				} else if (!strcmp(key_name, "Acceleration")) {
					acceleration = engines_esf->items[i].keys[j].value.i;
				} else if (!strcmp(key_name, "Top Speed")) {
					top_speed = engines_esf->items[i].keys[j].value.i;
				} else if (!strcmp(key_name, "Jump")) {
					jump = (unsigned char)engines_esf->items[i].keys[j].value.i;
				}
			}

			add_engine(name, acceleration, top_speed, jump);
		}
	}

	if (esf_close_handle(engines_esf) != 0) {
		printf("Couldn't close parser handle for load_engines_eaf()\n");
	}

	return (0);
}

int unload_engines(void) {

	int i;

	for (i = 0; i < num_engines; i++) {
		strcpy(engines[i].name, " ");
		engines[i].acceleration = 0;
		engines[i].top_speed = 0;
	}

	num_engines = 0;

	return (0);
}

static void add_shield(char *name, int strength) {
	if (num_shields < MAX_SHIELDS) {
		shields[num_shields].name = (char *)malloc(sizeof(char) * (strlen(name) + 1));
		memset(shields[num_shields].name, 0, sizeof(char) * (strlen(name) + 1));
		strcpy(shields[num_shields].name, name);
		shields[num_shields].strength = strength;
		num_shields++;
	}
}

int load_shields_eaf(FILE *eaf, char *filename) {
	char *name = NULL;
	int strength = 0;
	parsed_file *shields_esf = NULL;

	shields_esf = esf_new_handle();
	assert(shields_esf);

	if (esf_set_filter(shields_esf, "shield") != 0) {
		printf("Couldn't set engine filter\n");
	}

	if (esf_parse_file_eaf(eaf, shields_esf, filename) != 0) {
		printf("Coudln't parse \"%s\"\n", filename);
	} else {
		int i, j;

		for (i = 0; i < shields_esf->num_items; i++) {
			for (j = 0; j < shields_esf->items[i].num_keys; j++) {
				char *key_name = shields_esf->items[i].keys[j].name;

				if (!strcmp(key_name, "Name")) {
					name = shields_esf->items[i].keys[j].value.cp;
				} else if (!strcmp(key_name, "Strength")) {
					strength = shields_esf->items[i].keys[j].value.i;
				}
			}

			add_shield(name, strength);
		}
	}

	if (esf_close_handle(shields_esf) != 0) {
		printf("Couldn't close parser handle for load_shields_eaf()\n");
	}

	return (0);
}

int unload_shields(void) {
	int i;

	for (i = 0; i < num_shields; i++) {
		free(shields[i].name);
		shields[i].name = NULL;
		shields[i].strength = 0;
	}

	num_shields = 0;

	return (0);
}

void init_ships(void) {
	int i;

	for (i = 0; i < MAX_SHIPS; i++)
		ships[i] = NULL;
}

int load_ships_eaf(FILE *eaf, char *filename) {
	parsed_file *ships_esf = NULL;
	struct _ship *ship = NULL;
	unsigned char modified_player = 0; /* if we modified the player, s/he needs to be reinitalized */

	/* we'll do this first because some ship's may have objectives that refer to the player */
	init_player();

	/* first parse for the player information (if any) */
	ships_esf = esf_new_handle();
	assert(ships_esf);
	
	if (esf_set_filter(ships_esf, "player") != 0) {
		printf("Couldn't set parser filter\n");
	}

	if (esf_parse_file_eaf(eaf, ships_esf, filename) != 0) {
		printf("Coudln't parse \"%s\"\n", filename);
	} else {
		int i, j;

		for (i = 0; i < ships_esf->num_items; i++) {
			for (j = 0; j < ships_esf->items[i].num_keys; j++) {
				char *key_name = ships_esf->items[i].keys[j].name;
				union _esf_value value = ships_esf->items[i].keys[j].value;

				if (!strcmp(key_name, "Model")) {
					player_model = get_model_pointer(value.cp);
					assert(player_model);
					modified_player = 1;
				} else if (!strcmp(key_name, "Money")) {
					player.credits = value.i;
					modified_player = 1;
				} else if (!strcmp(key_name, "Initial X")) {
					player_x = value.i;
					modified_player = 1;
				} else if (!strcmp(key_name, "Initial Y")) {
					player_y = value.i;
					modified_player = 1;
				}
			}
		}
	}

	if (esf_close_handle(ships_esf) != 0) {
		printf("Couldn't close parser handle for load_ships_eaf()\n");
	}

	if (modified_player) {
		uninit_player();
		init_player();
	}

	/* and now parse for ships */
	ships_esf = esf_new_handle();
	assert(ships_esf);

	if (esf_set_filter(ships_esf, "ship") != 0) {
		printf("Couldn't set engine filter\n");
	}

	if (esf_parse_file_eaf(eaf, ships_esf, filename) != 0) {
		printf("Coudln't parse \"%s\"\n", filename);
	} else {
		int i, j, k, l;

		for (i = 0; i < ships_esf->num_items; i++) {
			ship = create_ship();
			for (j = 0; j < ships_esf->items[i].num_keys; j++) {
				char *key_name = ships_esf->items[i].keys[j].name;
				union _esf_value value = ships_esf->items[i].keys[j].value;

				if (!strcmp(key_name, "Model")) {
					ship->model = get_model_pointer(value.cp);
					assert(ship->model);
				} else if (!strcmp(key_name, "Alliance")) {
					ship->alliance = get_alliance_pointer(value.cp);
					assert(ship->alliance);
				} else if (!strcmp(key_name, "Name")) {
					ship->name = (char *)malloc(sizeof(char) * (strlen(value.cp) + 1));
					memset(ship->name, 0, sizeof(char) * (strlen(value.cp) + 1));
					strcpy(ship->name, value.cp);
				} else if (!strcmp(key_name, "Class")) {
					ship->class = (char *)malloc(sizeof(char) * (strlen(value.cp) + 1));
					memset(ship->class, 0, sizeof(char) * (strlen(value.cp) + 1));
					strcpy(ship->class, value.cp);
				} else if (!strcmp(key_name, "Initial X")) {
					ship->initial_x = value.i;
					ship->world_x = value.i;
					ship->screen_x = get_screen_coords(ship->initial_x, ship->initial_y, 1);
				} else if (!strcmp(key_name, "Initial Y")) {
					ship->initial_y = value.i;
					ship->world_y = value.i;
					ship->screen_y = get_screen_coords(ship->initial_x, ship->initial_y, 0);
				} else if (!strcmp(key_name, "Respawn")) {
					ship->respawn = (unsigned char)value.i;
				} else if (!strcmp(key_name, "Creation Delay")) ship->creation_delay = get_ticks() + (Uint32)(value.i * 1000);
				else if (!strcmp(key_name, "Cloak")) ship->has_cloak = (unsigned char)value.i;
				else if (!strcmp(key_name, "Disabled")) ship->disabled = (unsigned char)value.i;
			}
			/* parse subitems, for ships, this is the objectives part */
			for (k = 0; k < ships_esf->items[i].num_subitems; k++) {
				for (l = 0; l < ships_esf->items[i].subitems[k].num_keys; l++) {
					char *subkey_name = ships_esf->items[i].subitems[k].keys[l].name;
					union _esf_value subvalue = ships_esf->items[i].subitems[k].keys[l].value;

					if (!strcmp(subkey_name, "Fly To")) {
						ship->objectives[ship->num_objectives] = ship_new_obj_fly_to(subvalue.cp);
						ship->obj_types[ship->num_objectives] = FLY_TO;
						ship->num_objectives++;
					} else if (!strcmp(subkey_name, "Land On")) {
						ship->objectives[ship->num_objectives] = ship_new_obj_land_on(subvalue.cp);
						ship->obj_types[ship->num_objectives] = LAND_ON;
						ship->num_objectives++;
					} else if (!strcmp(subkey_name, "Destroy")) {
						ship->objectives[ship->num_objectives] = ship_new_obj_destroy(subvalue.cp);
						ship->obj_types[ship->num_objectives] = DESTROY;
						ship->num_objectives++;
					}
				}
			}

			add_ship(ship);
		}
	}

	if (esf_close_handle(ships_esf) != 0) {
		printf("Couldn't close parser handle for load_ships_eaf()\n");
	}

	return (0);
}

int unload_ships(void) {
	int i;

	for (i = 0; i < MAX_SHIPS; i++) {
		if (ships[i]) {
			free_ship(ships[i]);
			ships[i] = NULL;
		}
	}

	num_ships = 0;

	uninit_player();

	return (0);
}

/******************************************************************************      
*
*   Name:
*      void turn_ship(struct _ship *ship, int right);
*
*   Abstract:
*      Sets the angle on a _ship structure, calculating in the str, or seconds
*   to rotate (one full rotation, 360 degrees).
*
*   Context/Scope:	
*      Called by get_input().
*
*   Side Effects:
*      Changes the angle and old_angle variables within the struct passed.
*
*   Return Value:
*      None.
*
*   Assumptions:
*      Assumes the struct _ship passed is a valid structure, fully initialized.
*
******************************************************************************/
void turn_ship(struct _ship *ship, int right) {
	ship->old_angle = ship->angle;

	/* following code is a porportion
	 *
	 * second's it's been since last time   angle_increase (what you want)
	 * ---------------------------------- = ------------------------------
	 *       seconds it should take                   360
	*/
	if (right)
		ship->angle -= (float)((360 * loop_length) / (ship->model->str * 1000));
	else {
		ship->angle += (float)((360 * loop_length) / (ship->model->str * 1000));
	}

	while(ship->angle < 0)
		ship->angle += 360;

	ship->angle %= 360;

#ifndef NDEBUG
	if (ship->angle < 0)
		fprintf(stdout, "for assertion failure: ship->angle = %d\n", ship->angle);
	if (ship->angle > 360)
		fprintf(stdout, "for assertion failure: ship->angle = %d\n", ship->angle);
#endif
	assert(ship->angle >= 0);
	assert(ship->angle < 360);
}

/******************************************************************************      
*
*   Name:
*      int init_player(void);
*
*   Abstract:
*      Initializes player struct (a _ship struct).
*
*   Context/Scope:	
*      Called by init().
*
*   Side Effects:
*      None.
*
*   Return Value:
*      0 on sucess
*      -1 on failure
*
*   Assumptions:
*      None.
*
******************************************************************************/
int init_player(void) {
	extern unsigned char view_mode;
	int i;
	
	if (!view_mode) {
		player.ship = (struct _ship *)malloc(sizeof(struct _ship));
		if (player_model)
			player.ship->model = player_model;
		else
			player.ship->model = models[0];
		assert(player.ship->model);
		player.free_mass = player.ship->model->cargo;
		player.ship->alliance = get_alliance_pointer("Independent");
		player.ship->class = (char *)malloc(sizeof(char) * (strlen("None") + 1));
		memset(player.ship->class, 0, sizeof(char) * (strlen("None") + 1));
		strcpy(player.ship->class, "None");
		player.ship->engine = player.ship->model->default_engine;
		player.ship->shield = player.ship->model->default_shield;
		player.ship->plating.name = NULL;
		player.ship->plating.amt = 1.0f;
		player.ship->plating.repairing = 0;
		player.ship->has_booster = 0;
		player.ship->boost_strength = 1.0f;
		if ((player_x != 0) && (player_y != 0)) {
			player.ship->initial_x = player_x;
			player.ship->initial_y = player_y;
			player.ship->world_x = player_x;
			player.ship->world_y = player_y;
		} else {
			player.ship->initial_x = 0;
			player.ship->initial_y = 0;
			player.ship->world_x = 0;
			player.ship->world_y = 0;
		}
		player.ship->screen_x = 400;
		player.ship->screen_y = 300;
		player.ship->angle = 0;
		player.ship->has_cloak = 0;
		player.ship->old_angle = 0;
		player.ship->shield_life = player.ship->shield->strength;
		player.ship->hull_strength = player.ship->model->hull_life;
		player.ship->accel = 0.0;
		player.ship->name = NULL;
		player.ship->escape_pod = NULL;
		player.ship->max_velocity = (float)player.ship->engine->top_speed / 2000.0;
		player.ship->boost = 0;
		player.ship->cloak = 0;
		player.ship->was_close = 0;
		player.ship->destination = NULL;
		player.ship->target = NULL;
		player.ship->has_shield_flare = 0;
		player.ship->update_interval = 0;
		player.selected_planet = NULL;
		player.credits = 10000;
		player.ship->velocity = 0.0;
		player.ship->accel = 0.0;
		player.ship->status = 0;
		player.ship->momentum_x = 0.0;
		player.ship->momentum_y = 0.0;
		player.ship->disabled = 0;
		player.ship->fuel = player.ship->model->total_fuel;
		player.jump = NULL;
		/* initalize targetting (hud targetting) */
		player.target.target = NULL;
		player.target.type = TARGET_NONE;
		player.target.dist = 0.0f;

		/* initalize upgrades (set them all to null) */
		for (i = 0; i < MAX_OUTFITS; i++) 
			player.upgrades[i] = NULL;
		
		/* initalize weapons */
		for (i = 0; i < MAX_WEAPON_SLOTS; i++)
			player.ship->weapon_mount[i] = NULL;
		
		/* add inhertied weapon mounts */
		for (i = 0; i < MAX_WEAPON_SLOTS; i++) {
			if (player.ship->model->default_mounts[i]) {
				int j, slot = -1;
				
				/* find a free weapon mount */
				for (j = 0; j < MAX_WEAPON_SLOTS; j++) {
					if (!player.ship->weapon_mount[j]) {
						slot = j;
						break;
					}
				}
				
				/* no free slot for inheritance */
				if (slot == -1) {
					printf("no free inheritance slot\n");
					break;
				}
				
				/* add the weapon to 'slot' */
				player.ship->weapon_mount[slot] = copy_weapon_mount(player.ship->model->default_mounts[i]);
			} else {
				break;
			}
		}
		
		/* setup default weapon quantities */
		for (i = 0; i < 2; i++) {
			if (player.ship->weapon_mount[i])
				player.ship->w_slot[i] = i;
			else
				player.ship->w_slot[i] = -1;
		}
	}
	
	player_model = NULL;
	player_x = 0;
	player_y = 0;
	
	return (0);
}

int uninit_player(void) {
	extern unsigned char view_mode;
	int i;

	if (!view_mode) {
		if (player.ship->target)
			loose_target(player.ship->target);
		for (i = 0; i < MAX_WEAPON_SLOTS; i++) {
			if (player.ship->weapon_mount[i])
				free_weapon_mount(player.ship->weapon_mount[i]);
		}
		if (player.ship) {
			free(player.ship);
			player.ship = NULL;
		}
		player.selected_planet = NULL;
		player.credits = 0;
		if (player.jump != NULL) {
			free(player.jump);
			player.jump = NULL;
		}
		player.target.target = NULL;
		player.target.type = TARGET_NONE;
		player.target.dist = 0.0f;
	}

	return (0);
}

/* destroys a ship without respawning it */
void destroy_ship(struct _ship *ship, unsigned char non_destructive) {
	int i;
	
	/* eye candy */
	if (!non_destructive) {
		new_explosion(ship->world_x, ship->world_y, ship->momentum_x, ship->momentum_y, 85, 5000);
		create_blast(ship->model->image, ship->world_x, ship->world_y, ship->momentum_x, ship->momentum_y);
#ifndef WIN32
#warning fix force amount here (make it dependant on type class or something)
#endif
		new_force(RADIATING, ship->world_x, ship->world_y, 65.0f, 70.0f, 1000);
	}
	
	/* respawn */
	/* if this is the player's target, clear player's target */
	if (player.target.type == TARGET_SHIP) {
		struct _ship *targetted = (struct _ship *)player.target.target;
		if (targetted == ship) {
			erase_ship(targetted); /* the player's target will have a recticle that needs erasing */
			player.target.target = NULL;
			player.target.type = TARGET_NONE;
			player.target.dist = 0.0f;
		}
	}
	
	ai_ship_destroyed(ship); /* let all the a.i. ships know (if they have an objective or target, that the ship was destroyed) */
	ensure_not_targeted(ship); /* ensures nobody is still targetting this guy (since he's dead) */
	mission_ship_destroyed(ship, non_destructive); /* let the mission system know this ship is destroyed, in case this ship is part of an objective */
	ship->angle = 0;
	if (ship->target)
		loose_target(ship->target);
	ship->target = NULL;
	
	/* clear offenders list */
	for (i = 0; i < MAX_OFFENDERS; i++)
		ship->offenders[i] = NULL;
	
	ship->hull_strength = ship->model->hull_life;
	ship->shield_life = ship->shield->strength;
	ship->disabled = 0;
	
	/* if it was a r_ship, make sure that gets null'd */
	for (i = 0; i < MAX_R_SHIPS; i++)
		if (r_ships[i] == ship)
			r_ships[i] = NULL;

	/* finally, free the ship */
	for (i = 0; i < MAX_SHIPS; i++) {
		if (ships[i] == ship) {
			free_ship(ships[i]);
			ships[i] = NULL;
			break;
		}
	}

	num_ships--;
}

/* destorys that ship, respawns it, and if near_player is toggled, near the player */
/* NOTE: will always respawn the same model of ship */
/*       if landed is true, the mission objectives function will know that, and you will not fail (this function is used for destroyed ships and by the program to remove ships) */
void destroy_and_respawn(struct _ship *ship, unsigned char near_player, unsigned char non_destructive) {
	/* variables to save when respawning */
	model_t *old_model = NULL;
	struct _alliance *old_alliance = NULL;
	char old_class[80] = {0};
	void *old_objectives[MAX_SHIP_OBJECTIVES];
	int old_num_objectives;
	enum _ship_objectives old_obj_types[MAX_SHIP_OBJECTIVES];
	int i;

#ifndef WIN32
#warning we need to remember the model of ship and weapon mounts or something here or else it isnt a perfect respawn
#endif

	/* back up needed variables */
	old_model = ship->model;
	old_alliance = ship->alliance;
	strcpy(old_class, ship->class);
	for (i = 0; i < MAX_SHIP_OBJECTIVES; i++) {
		old_objectives[i] = copy_ship_objective(ship->objectives[i], ship->obj_types[i]);
		old_obj_types[i] = ship->obj_types[i];
	}
	old_num_objectives = ship->num_objectives;

	destroy_ship(ship, non_destructive);

	ship = create_ship();
	assert(ship);

	/* reassign ship objectives */
	for (i = 0; i < MAX_SHIP_OBJECTIVES; i++) {
		ship->objectives[i] = old_objectives[i];
		ship->obj_types[i] = old_obj_types[i];
	}
	ship->num_objectives = old_num_objectives;

	ship->name = NULL;
	ship->model = old_model; /* random for now i guess, until manufactorer=alliance is implemented */
	ship->alliance = old_alliance;
	ship->class = (char *)malloc(sizeof(char) * (strlen(old_class) + 1));
	memset(ship->class, 0, sizeof(char) * (strlen(old_class) + 1));
	strcpy(ship->class, old_class);

	if (near_player) {
		int offset_x, offset_y;
		offset_x = rander(800.0, 1600.0);
		offset_y = rander(600.0, 1200.0);
		/* random x coord */
		if ((int)rander(0, 1)) {
			ship->world_x = player.ship->world_x - offset_x;
		} else {
			ship->world_x = player.ship->world_x + offset_x;
		}
		/* random y coord */
		if ((int)rander(0, 1)) {
			ship->world_y = player.ship->world_y - offset_y;
		} else {
			ship->world_y = player.ship->world_y + offset_y;
		}
	} else {
		int offset_x, offset_y;
		offset_x = rander(1800.0, 8600.0);
		offset_y = rander(1600.0, 8200.0);
		/* random x coord */
		if ((int)rander(0, 1)) {
			ship->world_x = 0.0f - offset_x;
		} else {
			ship->world_x = 0.0f + offset_x;
		}
		/* random y coord */
		if ((int)rander(0, 1)) {
			ship->world_y = 0.0f - offset_y;
		} else {
			ship->world_y = 0.0f + offset_y;
		}
	}

	if (add_ship(ship) != ship) {
		free_ship(ship);
#ifndef NDEBUG
		printf("Couldn't respawn ship - add_ship() returned NULL\n");
#endif
	} else {
		push_ship_off_radar(ship);
	}
}

struct _weapon *get_weapon(char *name) {
	int i;

	for (i = 0; i < num_weapons; i++) {
		if (!strcmp(weapons[i]->name, name))
			return (weapons[i]);
	}

	return (NULL);
}

/* returns true or false as to whether the ship is visible on the screen */
unsigned char is_on_screen(struct _ship *ship) {
	if ((ship->screen_x > -300) && (ship->screen_x < (screen_width + 300)) && (ship->screen_y > -300) && (ship->screen_y < (screen_height + 300))) {
		return (1);
	}
	
	return (0);
}

/* hailing ship dialog */
void hail_ship(struct _ship *ship) {
	gui_window *hail_win;
	gui_button *hail_close_btn;
	SDL_Surface *front;
	int x, y, w, h; /* of main window */
	SDL_Rect src, dest;
	
	if (ship == NULL)
		return;
	
	hail_session = gui_create_session();
	
	/* figure out comm_front width and height */
	assert(loaded_eaf);
	front = load_image_eaf(main_eaf, ship->model->comm_front, BLACK_COLORKEY);
	if (!front) {
		front = load_image_eaf(loaded_eaf, ship->model->comm_front, BLACK_COLORKEY);
		if (!front) {
			fprintf(stdout, "Could not load image \"%s\".\n", ship->model->comm_front);
			return;
		}
	}
	
	w = front->w + 20;
	h = front->h + 145;
	x = 400 - (w / 2);
	y = 300 - (h / 2);
	hail_win = gui_create_window(x, y, w, h, hail_session);
	hail_close_btn = gui_create_button(x + w - 95, y + h - 35, 85, 22, "Close channel", hail_session);
	
	gui_button_set_callback(hail_close_btn, hail_close_btn_handle);
	
	gui_session_show_all(hail_session);
	
	/* blit the ship comm_front image */
	src.x = 0;
	src.y = 0;
	src.w = front->w;
	src.h = front->h;
	dest.x = x + 10;
	dest.y = y + 10;
	dest.w = front->w;
	dest.h = front->h;
	blit_surface(front, &src, &dest, 1);
	
	flip();
	
	gui_main(hail_session);
	
	gui_session_destroy_all(hail_session);
	gui_destroy_session(hail_session);
	
	SDL_FreeSurface(front);
	
	draw_frame(1);
}

void hail_close_btn_handle(void) {
	hail_session->active = 0;
}

/* boarding ship code */
/* boards a disabled ship you're on top of */
void board_ship(void) {
	struct _ship *ship = NULL;
	
	update_targets(); /* make sure we update the closest ships */
	
	if ((targets[0].dist > 1000) || (targets[0].type != TARGET_SHIP))
		return; /* not close enough */
	
	ship = (struct _ship *)targets[0].target;
	
	if (ship->disabled) {
		/* if the closest ship is disabled, board it */
		unsigned char success; /* whether or not you could board them */
		
		success = (unsigned char)rander(0, 50);
		if (success < 15) {
			gui_alert("Could not board ship!");
			SDL_ShowCursor(0);
			return;
		} else {
			/* we will be boarding the ship, so see if this affects any mission parameters */
			mission_player_boarded(ship);
		}
		
		board_session = gui_create_session();
		
		board_win = gui_create_window(250, 212, 300, 175, board_session);
		take_cash = gui_create_button(307, 287, 185, 22, "Take their money", board_session);
		steal_ship = gui_create_button(307, 318, 185, 22, "Steal their ship", board_session);
		abandon_btn = gui_create_button(307, 350, 185, 22, "Abandon them", board_session);
		
		gui_button_set_callback(take_cash, take_cash_handle);
		gui_button_set_callback(steal_ship, steal_ship_handle);
		gui_button_set_callback(abandon_btn, abandon_btn_handle);
		
		gui_session_show_all(board_session);
		
		epiar_render_text(gui_font_normal, white, 0, AFONT_RENDER_BLENDED, screen, 400, 232, "The crew surrenders. What will you do?");
		
		gui_main(board_session);
		SDL_ShowCursor(0);
		
		gui_session_destroy_all(board_session);
		gui_destroy_session(board_session);
	}
}

void abandon_btn_handle(void) {
	board_session->active = 0;
}

void take_cash_handle(void) {
	/* interesting way to get a random cash value */
	long int value;
	char msg[80];
	struct _ship *ship = (struct _ship *)targets[0].target;

#ifndef WIN32
#warning fix cash value
#endif	
	value = (rander(25, 50) + 350) * 10;
	
	player.credits += value;
	sprintf(msg, "You stole %ld credits\n", value);
	gui_alert(msg);
	gui_alert("The crew has activated the auto-destruct!");
	destroy_and_respawn(ship, 0, 0);
	board_session->active = 0;
}

void steal_ship_handle(void) {
	model_t *old_player_model = player.ship->model;
	struct _ship *ship = (struct _ship *)targets[0].target;

	/* much like the buying a ship in the shipyard code, this code changes the player's ship */
	player.ship->model = ship->model;
	player.ship->engine = ship->engine;
	player.ship->shield = ship->shield;
	player.ship->hull_strength = player.ship->model->hull_life;
	player.ship->shield_life = 0;
	player.free_mass = player.ship->model->cargo;
	player.ship->fuel += ship->fuel;
	if (player.ship->fuel > player.ship->model->total_fuel)
		player.ship->fuel = player.ship->model->total_fuel;
	gui_alert("You have abandoned your ship and taken theirs.");
	board_session->active = 0;
	ship->model = old_player_model;
	destroy_and_respawn(ship, 0, 0);
}

void ensure_not_targeted(struct _ship *ship) {
	int i, j;

	for (i = 0; i < MAX_SHIPS; i++) {
		/* ensure not targetted */
		if (ships[i]) {
			if (ships[i]->target) {
				if (ships[i]->target->offender == ship) {
					loose_target(ships[i]->target);
					ships[i]->target = NULL;
				}
			}
			/* clear it if its on the offenders list */
			for (j = 0; j < MAX_OFFENDERS; j++) {
				if (ships[i]->offenders[j] == ship)
					ships[i]->offenders[j] = NULL;
			}
		}
	}
}

float get_distance_from_player_sqrd(float x, float y) {
	return (((x - player.ship->world_x) * (x - player.ship->world_x)) + ((y - player.ship->world_y) * (y - player.ship->world_y)));
}

struct _engine *get_engine_pointer(char *name) {
	int i;

	for (i = 0; i < num_engines; i++) {
		if (!strcmp(engines[i].name, name))
			return (&engines[i]);
	}

	return (NULL);
}

struct _shield *get_shield_pointer(char *name) {
	int i;

	for (i = 0; i < num_shields; i++) {
		if (!strcmp(shields[i].name, name))
			return (&shields[i]);
	}

	return (NULL);
}

void damage_ship(struct _ship *ship, struct _fire *fire) {
	assert(ship && fire && ship->alliance && fire->weapon && fire->owner);
	
	add_offender(ship, fire->owner);

	alliance_ship_damaged(ship->alliance);

	damage_ship_non_fire(ship, fire->weapon->strength, fire->angle);
}

/* returns true if ship was/is destroyed */
int damage_ship_non_fire(struct _ship *ship, int strength, int angle) {
	int destroyed = 0;
	assert(ship);
	
	if (ship->hull_strength <= 0)
		return (1); /* already dead */
	
	if (ship->shield_life > 0) {
		ship->shield_life -= strength;
		new_flare(ship, angle);
	} else {
		int dam_x, dam_y;
		int opp_angle;
		
		/* figure out where on the shield circle the thing hit, based on angle */
		opp_angle = (angle + 180) % 360;
		dam_x = get_cos(opp_angle) * ship->model->radius;
		dam_y = get_sin(opp_angle) * ship->model->radius;
		
		ship->hull_strength -= (strength * ship->plating.amt);
		chunk_blast(ship->world_x + dam_x, ship->world_y - dam_y, 35 - ((MAX_CHUNKS * ship->hull_strength) / ship->model->hull_life), 2000, angle, -1);
	}
	
	/* take the extra damage and put it on the hull */
	if (ship->shield_life < 0) {
		int dam_x, dam_y;
		int opp_angle;
		int extra = 0 - ship->shield_life;
		
		/* figure out where on the shield circle the thing hit, based on angle */
		opp_angle = (angle + 180) % 360;
		dam_x = cos(opp_angle) * ship->model->radius;
		dam_y = sin(opp_angle) * ship->model->radius;
		
		ship->shield_life = 0;
		/* apply the left over damage to the hull */
		ship->hull_strength -= (extra * ship->plating.amt);
		chunk_blast(ship->world_x + dam_x, ship->world_y - dam_y, 35 - ((MAX_CHUNKS * ship->hull_strength) / ship->model->hull_life), 2000, angle, -1);
	}
	
	/* ship is gone */
	if (ship->hull_strength <= 0) {
		if (ship->escape_pod) {
			int i;
			
			/* change ship to the escape pod */
			ship->model = ship->escape_pod;
			ship->hull_strength = ship->model->hull_life;
			ship->shield_life = ship->model->default_shield->strength;
			ship->engine = ship->model->default_engine;
			ship->shield = ship->model->default_shield;
			ship->escape_pod = NULL;
			/* free weapon mounts (escape pods dont have weapons I guess) */
			for (i = 0; i < MAX_WEAPON_SLOTS; i++) {
				if (ship->weapon_mount[i]) {
					free_weapon_mount(ship->weapon_mount[i]);
					ship->weapon_mount[i] = NULL;
				}
			}

			/* nobody should recognize the pod as the player */
			ensure_not_targeted(ship);

			/* give them a little boost away */
			ship->momentum_x += cos(ship->angle) * 10.0f;
			ship->momentum_y -= sin(ship->angle) * 10.0f; /* y axis is flipped */
			cap_momentum(ship, MOMENTUM_CAP);
		} else {
			if (ship == player.ship) {
				kill_player();
				destroyed = 1;
			} else {
				if (ship->respawn) {
					destroy_and_respawn(ship, 0, 0);
					destroyed = 1;
				} else {
					destroy_ship(ship, 0);
					destroyed = 1;
				}
			}
		}
	} else if (((float)ship->hull_strength / (float)ship->model->hull_life) < 0.25f) {
		/* ship is disabled */
		ship->disabled = 1;
		if (ship->has_cloak) {
			/* ensure the ship is decloaked */
			decloak_ship(ship);
		}
	}

	return(destroyed);
}

void kill_player(void) {
	player_dead = current_time + 2500;

	/* eye candy */
#ifndef WIN32
#warning the 400 should be some number dependant upon mass
#endif
	new_explosion(player.ship->world_x, player.ship->world_y, player.ship->momentum_x, player.ship->momentum_y, 350, 5000);
	create_blast(player.ship->model->image, player.ship->world_x, player.ship->world_y, player.ship->momentum_x, player.ship->momentum_y);

	/* "gameplay candy" */
	new_force(RADIATING, player.ship->world_x, player.ship->world_y, 3, 100, 1500);

	ensure_not_targeted(player.ship);

	reset_input(); /* don't want to keep firing or anything if they're dead */
}

void thrust_ship(struct _ship *ship) {

	assert(ship);

	if ((ship->fuel) || (ship->fuel == -1)) {
		ship->accel += ship->engine->acceleration;
		if (ship->fuel != -1) {
			ship->fuel -= 1;
			if (ship->fuel < 0)
				ship->fuel = 0;
		}
	}

}

/* parses ships that have been given names and returns the correct one if it exists */
struct _ship *get_ship_pointer(char *name) {
	int i;

	assert(name);

	if (!strcmp(name, "Player"))
		return (player.ship);

	for (i = 0; i < MAX_SHIPS; i++) {
		if (ships[i]) {
			if (ships[i]->name) {
				if (!strcmp(name, ships[i]->name))
					return (ships[i]);
				else
					printf("Ship \"%s\" does not match with passed \"%s\"\n", ships[i]->name, name);
			} else {
				printf("Ship #%d has no name\n", i);
			}
		}
	}

#ifndef NDEBUG
	printf("get_ship_pointer() returning NULL\n");
#endif

	return (NULL);
}

/* objective based functions */
static void *ship_new_obj_fly_to(char *planet) {
	ship_obj_fly_to *obj = (ship_obj_fly_to *)malloc(sizeof(ship_obj_fly_to));

	assert(obj);

	obj->planet = get_planet_pointer(planet);
	assert(obj->planet);

	return (obj);
}

static void *ship_new_obj_land_on(char *planet) {
	ship_obj_land_on *obj = (ship_obj_land_on *)malloc(sizeof(ship_obj_land_on));

	assert(obj);

	obj->planet = get_planet_pointer(planet);
	assert(obj->planet);

	return (obj);
}

static void *ship_new_obj_destroy(char *ship) {
	ship_obj_destroy *obj = (ship_obj_destroy *)malloc(sizeof(ship_obj_destroy));

	assert(obj);

	obj->ship = get_ship_pointer(ship);
	if (!obj->ship) {
		printf("Couldn't find ship \"%s\" for a ship's objectives. This will cause problems. Perhaps\n", ship);
		printf("in the ships.esf file in the .EAF archive, you are referring to destroy a ship that isn't\n");
		printf("defined yet. That would produce this. Please only refer to ships that have already been\n");
		printf("created.\n");
	}
	strcpy(obj->who, ship);

	return (obj);
}

/* call this if you want a ship to cloak - sets up the cloaking process */
void cloak_ship(struct _ship *ship) {
	printf("ship is cloaking\n");
	if (ship->has_cloak && (!ship->disabled)) {
		ship->cloak = 255;
		ship->status |= SHIP_CLOAKING;
		if (ship->status & SHIP_DECLOAKING)
			ship->status ^= SHIP_DECLOAKING;
		ship->cloak_start = get_ticks();
	}
}

/* call this if you want a ship to decloak - sets up the decloaking process */
void decloak_ship(struct _ship *ship) {
	printf("ship is decloaking\n");
	if (ship->has_cloak) {
		ship->cloak = 0;
		if (ship->status & SHIP_CLOAKING)
			ship->status ^= SHIP_CLOAKING;
		ship->status |= SHIP_DECLOAKING;
		ship->cloak_start = get_ticks();
	}
}

/* creates a blank ship struct - use add_ship() if you want to add a ship to the galaxy, not this */
struct _ship *create_ship(void) {
	struct _ship *ship = (struct _ship *)malloc(sizeof(struct _ship));
	int i;

	if (!ship)
		return (NULL);

	ship->name = NULL;
	ship->status = 0;
	ship->respawn = 1;
	ship->model = NULL;
	ship->alliance = NULL;
  	ship->class = NULL;
	ship->destination = NULL;
	ship->gate = NULL;
  	ship->target = NULL;
	for (i = 0; i < MAX_OFFENDERS; i++)
		ship->offenders[i] = NULL;
	ship->world_x = 0.0f;
	ship->world_y = 0.0f;
	ship->screen_x = 0;
	ship->screen_y = 0;
	ship->velocity = 0.0f;
	ship->max_velocity = 0.0f;
  	ship->accel = 0.0f;
	ship->momentum_x = 0.0f;
	ship->momentum_y = 0.0f;
  	ship->angle = 0;
	ship->old_angle = 0;  	
  	ship->shield_life = 0.0f;
  	ship->hull_strength = 0;
	ship->boost = 0;
	ship->fuel = 0;
	ship->w_slot[0] = ship->w_slot[1] = -1;
	ship->has_shield_flare = 0;
  	ship->update_interval = 0;
	ship->disabled = 0;
	for (i = 0; i < 4; i++) {
		ship->ai.dest_point[i][0] = 0;
		ship->ai.dest_point[i][1] = 0;
	}
	ship->ai.dest_heading = 0;
	ship->ai.home = NULL;
	ship->ai.stalking = 0;
	for (i = 0; i < MAX_SHIP_OBJECTIVES; i++) {
		ship->objectives[i] = NULL;
		ship->obj_types[i] = NOTHING;
	}
  	ship->num_objectives = 0;
  	ship->current_objective = 0;
  	ship->creation_delay = 0;
  	ship->landed = 0;
	ship->landed_on = NULL;
	ship->cloak = 0;
	ship->cloak_start = 0;
  	ship->has_cloak = 0;
  	for (i = 0; i < MAX_WEAPON_SLOTS; i++)
		ship->weapon_mount[i] = NULL;
  	ship->escape_pod = NULL;
	ship->plating.name = NULL;
	ship->plating.amt = 1.0f;
	ship->plating.repairing = 0;
	ship->has_booster = 0;
	ship->boost_strength = 1.0f;

	return (ship);
}

/* frees ship and removes from ship list */
static void free_ship(struct _ship *ship) {
	if (ship) {
		int j;

		free(ship->class);
		if (ship->target) {
			loose_target(ship->target);
			ship->target = NULL;
		}
		for (j = 0; j < MAX_WEAPON_SLOTS; j++) {
			if (ship->weapon_mount[j])
				free_weapon_mount(ship->weapon_mount[j]);
			else
				break;
		}
		ship->angle = 0;
		ship->hull_strength = 0;
		ship->shield_life = 0;
		ship->model = NULL;
		ship->world_x = 0;
		ship->world_y = 0;
		ship->destination = NULL;
		ship->gate = NULL;
		ship->target = NULL;
		ship->ai.home = NULL;
		if (ship->plating.name)
			free(ship->plating.name);
		
		/* clear offenders list */
		for (j = 0; j < MAX_OFFENDERS; j++) {
			ship->offenders[j] = NULL;
		}
		
		/* clear objective list (if any) */
		for (j = 0; j < ship->num_objectives; j++) {
			if (ship->obj_types[j] == FLY_TO) {
				ship_obj_fly_to *obj = (ship_obj_fly_to *)ship->objectives[j];
				
				assert(obj);
				
				free(obj);
				obj = NULL;
			} else if (ship->obj_types[j] == DESTROY) {
				ship_obj_destroy *obj = (ship_obj_destroy *)ship->objectives[j];
				
				assert(obj);
				
				free(obj);
				obj = NULL;
			} else if (ship->obj_types[j] == LAND_ON) {
				ship_obj_land_on *obj = (ship_obj_land_on *)ship->objectives[j];
				
				assert(obj);
				
				free(obj);
				obj = NULL;
			}
		}
		
		ship->alliance = NULL;

		for (j = 0; j < MAX_SHIPS; j++) {
			if (ships[j] == ship) {
				ships[j] = NULL;
				break;
			}
		}
#ifndef NDEBUG
		if (j == MAX_SHIPS) {
			printf("couldnt find ship free_ship() is freeing in ships list\n");
		}
#endif

		free(ship);
		ship = NULL;
	}
}

void *copy_ship_objective(void *obj, enum _ship_objectives type) {
	void *new_obj = NULL;

	if (type == NOTHING) {
		return (NULL);
	} else if (type == FLY_TO) {
		ship_obj_fly_to *new, *old;
		new_obj = (ship_obj_fly_to *)malloc(sizeof(ship_obj_fly_to));
		new = (ship_obj_fly_to *)new_obj;
		old = (ship_obj_fly_to *)obj;

		new->planet = old->planet;
	} else if (type == LAND_ON) {
		ship_obj_land_on *old, *new;
		new_obj = (ship_obj_land_on *)malloc(sizeof(ship_obj_land_on));
		new = (ship_obj_land_on *)new_obj;
		old = (ship_obj_land_on *)obj;

		new->planet = old->planet;
	} else if (type == DESTROY) {
		ship_obj_destroy *old, *new;
		new_obj = (ship_obj_destroy *)malloc(sizeof(ship_obj_destroy));
		new = (ship_obj_destroy *)new_obj;
		old = (ship_obj_destroy *)obj;

		new->ship = old->ship;
		strcpy(new->who, old->who);
	}

	return (new_obj);
}

void push_ship_off_radar(struct _ship *ship) {
	if (!ship)
		return;

	if (get_distance_sqrd(player.ship->world_x, player.ship->world_y, ship->world_x, ship->world_y) > COMM_DIST_SQRD)
		return; /* we are off radar */

	printf("player   at %f,%f\n", player.ship->world_x, player.ship->world_y);
	printf("ship was at %f,%f\n", ship->world_x, ship->world_y);

	/* they're to the player's right */
	if ((ship->world_x - player.ship->world_x) > 0) {
		/* find out exactly how far they are from being off radar */
		int far_x = abs((int)ship->world_x - ((int)player.ship->world_x + COMM_DIST));

		printf("ship is %d units too close x-wise on the right\n", far_x);
		ship->world_x += far_x;
	} else {
		/* they're on the player's left */

		/* find out exactly how far they are from being off radar */
                int far_x = abs(((int)player.ship->world_x - COMM_DIST) - (int)ship->world_x);

                printf("ship is %d units too close x-wise on the left\n", far_x);
                ship->world_x -= far_x;
	}

	/* they're below us (epiar has a reversed y axis unit system) */
	if ((ship->world_y - player.ship->world_y) < 0) {
		int far_y = abs((int)ship->world_y - ((int)player.ship->world_y - COMM_DIST));

		printf("ship is %d units too close y-wise on bottom\n", far_y);
		ship->world_y -= far_y;
	} else {
		int far_y = abs(((int)player.ship->world_y + COMM_DIST) - (int)ship->world_y);

		printf("ship is %d units too close y-wise on top\n", far_y);

		ship->world_y += far_y;
	}

	printf("ship is now at %f,%f\n", ship->world_x, ship->world_y);
}

void weapon_cycle(struct _ship *ship, unsigned char which, unsigned char how) {
	if (how == CYCLE_FOR) {
		ship->w_slot[which]++;
		if ((ship->w_slot[which] >= MAX_WEAPON_SLOTS) || (!ship->weapon_mount[ship->w_slot[which]]))
			ship->w_slot[which] = 0;
		if (ship->w_slot[which] == ship->w_slot[1]) {
			ship->w_slot[which]++;
			if ((ship->w_slot[which] >= MAX_WEAPON_SLOTS) || (!ship->weapon_mount[ship->w_slot[which]]))
				ship->w_slot[which] = 0;
		}
		if (!ship->weapon_mount[ship->w_slot[which]])
			ship->w_slot[which] = -1;
	} else if (how == CYCLE_BAC) {
		ship->w_slot[which]--;
		if (ship->w_slot[which] < 0) {
			int i;
			
			for (i = (MAX_WEAPON_SLOTS - 1); i >= 0; i--) {
				if (ship->weapon_mount[i]) {
					ship->w_slot[which] = i;
					break;
				}
			}
		}
		if (ship->w_slot[which] == ship->w_slot[1]) {
			ship->w_slot[which]--;
			
			if (ship->w_slot[which] < 0) {
				int i;
				
				for (i = (MAX_WEAPON_SLOTS - 1); i >= 0; i--) {
					if (ship->weapon_mount[i]) {
						ship->w_slot[which] = i;
						break;
					}
				}
			}
		}
		if ((!ship->weapon_mount[ship->w_slot[which]]) || (ship->w_slot[which] < 0))
			ship->w_slot[which] = -1;
	}
}
