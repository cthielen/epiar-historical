#include "gui/gui.h"
#include "includes.h"
#include "missions/missions.h"
#include "sprite/planet.h"
#include "sprite/sprite.h"
#include "system/eaf.h"
#include "system/esf.h"

#define MAX_OBJECTIVES 10

enum _mission_status {M_NONE, M_ONGOING, M_FAILED, M_SUCCESS} mission_status = M_NONE;

/* main mission variables */
struct {
	void *objectives[MAX_OBJECTIVES];
	enum {SHIP_MUST_LAND, DESTROY_SHIP, BOARD_SHIP, MUST_LAND_ON} type[MAX_OBJECTIVES];
	int num_objectives;
	enum _mission_status status[MAX_OBJECTIVES];
} mission;

/* objective specific structures */
typedef struct {
	struct _ship *who;
	struct _planet *where;
} obj_ship_must_land;

typedef struct {
	struct _ship *who;
} obj_destroy_ship;

typedef struct {
	struct _planet *where;
} obj_land_on;

typedef struct {
	struct _ship *who;
} obj_board_ship;

int load_mission_eaf(FILE *eaf, char *filename) {
	parsed_file *mission_esf = NULL;
	int i, j, k;
	
	mission_status = M_NONE;
	
	mission.num_objectives = 0;
	
	/* parse file inside .eaf to obtain data */
	mission_esf = esf_new_handle();
	assert(mission_esf);
	
	if (esf_set_filter(mission_esf, "scenario") != 0)
		assert(-1);
	
	if (esf_parse_file_eaf(eaf, mission_esf, filename) != 0) {
		printf("Could not parse \"%s\"\n", filename);
		esf_close_handle(mission_esf);
		return (-1);
	}
	
	for (i = 0; i < mission_esf->num_items; i++) {
		for (j = 0; j < mission_esf->items[i].num_subitems; j++) {
			void *obj = NULL;
			for (k = 0; k < mission_esf->items[i].subitems[j].num_keys; k++) {
				char *key = mission_esf->items[i].subitems[j].keys[k].name;
				union _esf_value value = mission_esf->items[i].subitems[j].keys[k].value;
				
				if (!strcmp(key, "Type")) {
					if (!strcmp("Destroy", value.cp)) {
						obj = (obj_destroy_ship *)malloc(sizeof(obj_destroy_ship));
						mission.type[mission.num_objectives] = DESTROY_SHIP;
					} else if (!strcmp("Ship Must Land", value.cp)) {
						obj = (obj_ship_must_land *)malloc(sizeof(obj_ship_must_land));
						mission.type[mission.num_objectives] = SHIP_MUST_LAND;
					} else if (!strcmp("Board", value.cp)) {
						obj = (obj_board_ship *)malloc(sizeof(obj_board_ship));
						mission.type[mission.num_objectives] = BOARD_SHIP;
					} else if (!strcmp("Land On", value.cp)) {
						obj = (obj_land_on *)malloc(sizeof(obj_land_on));
						mission.type[mission.num_objectives] = MUST_LAND_ON;
					}
				} else if (!strcmp(key, "Who")) {
					if (mission.type[mission.num_objectives] == SHIP_MUST_LAND) {
						obj_ship_must_land *land_ship_obj = (obj_ship_must_land *)obj;
						land_ship_obj->who = get_ship_pointer(value.cp);
					} else if (mission.type[mission.num_objectives] == DESTROY_SHIP) {
						obj_destroy_ship *destroy_ship_obj = (obj_destroy_ship *)obj;
						destroy_ship_obj->who = get_ship_pointer(value.cp);
					}
				} else if (!strcmp(key, "Where")) {
					if (mission.type[mission.num_objectives] == SHIP_MUST_LAND) {
						obj_ship_must_land *land_ship_obj = (obj_ship_must_land *)obj;
						land_ship_obj->where = get_planet_pointer(value.cp);
					} else if (mission.type[mission.num_objectives] == MUST_LAND_ON) {
						obj_land_on *land_on = (obj_land_on *)obj;
						land_on->where = get_planet_pointer(value.cp);
					}
				} else if (!strcmp(key, "What")) {
					if (mission.type[mission.num_objectives] == BOARD_SHIP) {
						obj_board_ship *board_ship = (obj_board_ship *)obj;
						board_ship->who = get_ship_pointer(value.cp);
					}
				}
			}
			mission.objectives[mission.num_objectives] = obj;
			mission.status[mission.num_objectives] = M_ONGOING;
			mission.num_objectives++;
		}
	}
	
	if (esf_close_handle(mission_esf) != 0)
		assert(-1);
	
	if (mission.num_objectives != 0)
		mission_status = M_ONGOING;
	
	return (0);
}

void close_mission(void) {
	int i;
	
	for (i = 0; i < mission.num_objectives; i++) {
		free(mission.objectives[i]);
	}
	
	mission.num_objectives = 0;
}

/* checks to see if any objectives are complete and set a flag if all objectives are complete */
int update_mission(void) {
	int i;
	unsigned char mission_completed = 1; /* this is quickly set to zero if any objectives are "ONGOING" */
	unsigned char something_failed = 0;
	
	if (mission_status == M_NONE)
		return (-1);
	
	for (i = 0; i < mission.num_objectives; i++) {
		if (mission.status[i] == M_ONGOING) {
			/* check through ongoing missions to see if their status has changed (note: most objectives are checked and set elsewhere) */
			
			/* checks for new scenario objectives (check to see if they were completed) go here */
			
			mission_completed = 0;
		} else if (mission.status[i] == M_FAILED) {
			something_failed = 1;
		}
	}
	
	if (mission_completed) {
		close_mission();
		mission_status = M_NONE;
	}
	
	if (something_failed)
		return (2);
	else
		return (mission_completed);
}

/* checks and sees if this ship will affect mission (like ... failing the mission or something) */
/* if "landed", the ship wasnt destroyed (although it will be gone from the game unniverse), but it landed somewhere forever */
void mission_ship_destroyed(struct _ship *ship, unsigned char landed) {
	int i, j;
	
	assert(ship);
	
	if (mission.status == M_NONE)
		return; /* no mission going on right now */
	
	/* check against all mission objectives so we don't fail (or suceed) something, and if we do, fail the mission */
	for (i = 0; i < mission.num_objectives; i++) {
		if (mission.type[i] == SHIP_MUST_LAND) {
			obj_ship_must_land *obj = mission.objectives[i];
			
			if (obj->who == ship) {
				if (landed)
					mission.status[i] = M_SUCCESS;
				else
					mission.status[i] = M_FAILED; /* a ship that must land somewhere has been destroyed */
			}
		} else if (mission.type[i] == DESTROY_SHIP) {
			obj_destroy_ship *obj = mission.objectives[i];
			
			if (obj->who == ship)
				mission.status[i] = M_SUCCESS;
		}
		/* boarding a ship is destructive to that ship so this causes the vargeson scenario to fail every time - needs fixing
		   if (mission.type[i] == BOARD_SHIP) {
		   obj_board_ship *obj = mission.objectives[i];
		   
		   if (obj->who == ship)
		   mission.status[i] = M_FAILED;
		   }
		*/
	}
	
	/* check all a.i. to see if the destruction of this ship affects any of their objectives */
	for (i = 0; i < MAX_SHIPS; i++) {
		if (ships[i]) {
			for (j = 0; j < ships[i]->num_objectives; j++) {
				if (ships[i]->obj_types[j] == DESTROY_SHIP) {
					obj_destroy_ship *obj = ships[i]->objectives[j];
					assert(obj);
					
					/* see if they completed this objective */
					if (obj->who == ship)
						ships[i]->current_objective++;
				}
			}
		}
	}
}

/* called when the player lands somewhere - this function checks to see how it affects mission parameters (if you have to land on a planet, for example) and reacts */
void mission_player_landed(struct _planet *planet) {
	int i;
	
	for (i = 0; i < mission.num_objectives; i++) {
		if (mission.type[i] == MUST_LAND_ON) {
			obj_land_on *data = (obj_land_on *)mission.objectives[i];
			
			if (data->where == planet)
				mission.status[i] = M_SUCCESS;
		}
	}
}

void mission_player_boarded(struct _ship *ship) {
	int i;
	
	for (i = 0; i < mission.num_objectives; i++) {
		if (mission.type[i] == BOARD_SHIP) {
			obj_board_ship *data = (obj_board_ship *)mission.objectives[i];
			
			if (data->who == ship)
				mission.status[i] = M_SUCCESS;
		}
	}
}
