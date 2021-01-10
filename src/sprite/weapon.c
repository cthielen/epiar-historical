#include "audio/audio.h"
#include "com_defs.h"
#include "includes.h"
#include "sprite/player.h"
#include "sprite/ship.h"
#include "sprite/weapon.h"
#include "system/eaf.h"
#include "system/esf.h"
#include "system/path.h"
#include "system/video/video.h"

int num_weapons = 0;
int num_ammo = 0;

struct _weapon *weapons[MAX_WEAPONS];
struct _ammo *ammos[MAX_AMMO];

/* adds weapon to the main weapon list */
static int add_weapon(struct _weapon *weapon);
static int add_ammo(struct _ammo *ammo);
/* creates a weapon struct */
static struct _weapon *create_weapon(void);
static struct _ammo *create_ammo(void);
/* frees a weapon struct */
static int free_weapon(struct _weapon *weapon);
static int free_ammo(struct _ammo *ammo);

static int add_weapon(struct _weapon *weapon) {
	if (num_weapons < MAX_WEAPONS) {
		weapons[num_weapons] = weapon;
		num_weapons++;

		return (0);
	} else {
		fprintf(stdout, "Too many weapons added already.\n");
	}

	return (-1);
}

static int add_ammo(struct _ammo *ammo) {
	if (num_ammo < MAX_AMMO) {
		ammos[num_ammo] = ammo;
		num_ammo++;
		
		return (0);
	} else {
		fprintf(stdout, "Too many weapons added already.\n");
	}
	
	return (-1);
}

/* parse the esf and get weapon/ammo information */
int load_weapons_eaf(FILE *eaf, char *filename) {
	parsed_file *weapons_esf = NULL;
	
	/* get a parser handle */
	weapons_esf = esf_new_handle();
	assert(weapons_esf);
	
	/* set the filter to look for weapons */
	if (esf_set_filter(weapons_esf, "weapon") != 0)
		assert(0);
	
	if (esf_parse_file_eaf(eaf, weapons_esf, filename) != 0) {
		printf("Error: Could not parse \"weapons.esf\" in EAF archive.\n");
	} else {
		/* parsed successfully, so get the data */
		int i, j;
		
		for (i = 0; i < weapons_esf->num_items; i++) {
			struct _weapon *new_weapon = create_weapon();
			
			for (j = 0; j < weapons_esf->items[i].num_keys; j++) {
				char *name = weapons_esf->items[i].keys[j].name;
				if (!strcmp(name, "Name")) {
					/* set the weapon name */
					char *weap_name = weapons_esf->items[i].keys[j].value.cp;
					new_weapon->name = (char *)malloc(sizeof(char) * (strlen(weap_name) + 1));
					strcpy(new_weapon->name, weap_name);
					new_weapon->name[strlen(new_weapon->name)] = 0;
				} else if (!strcmp(name, "Ammo Image")) {
					char *img = weapons_esf->items[i].keys[j].value.cp;
					new_weapon->ammo_image = load_image_eaf(eaf, img, BLUE_COLORKEY);
					if (new_weapon->ammo_image == NULL) {
						new_weapon->ammo_image = load_image_eaf(main_eaf, img, BLUE_COLORKEY);
						if (new_weapon->ammo_image == NULL) {
							printf("Could not load weapon ammo image from scenario or main EAF\n");
						}
					}
				} else if (!strcmp(name, "Lifetime")) {
					new_weapon->lifetime = weapons_esf->items[i].keys[j].value.i;
				} else if (!strcmp(name, "Recharge")) {
					new_weapon->recharge = weapons_esf->items[i].keys[j].value.i;
				} else if (!strcmp(name, "Velocity")) {
					new_weapon->velocity = weapons_esf->items[i].keys[j].value.i;
				} else if (!strcmp(name, "Strength")) {
					new_weapon->strength = weapons_esf->items[i].keys[j].value.i;
				} else if (!strcmp(name, "Uses Ammo")) {
					new_weapon->uses_ammo = (unsigned char)weapons_esf->items[i].keys[j].value.i;
				} else if (!strcmp(name, "Initial Ammo")) {
					new_weapon->initial_ammo = weapons_esf->items[i].keys[j].value.i;
				} else if (!strcmp(name, "Homing")) {
					new_weapon->homing = (unsigned char)weapons_esf->items[i].keys[j].value.i;
				} else if (!strcmp(name, "Accepted Ammo")) {
					/* store the accepted ammo */
					char *ammo = weapons_esf->items[i].keys[j].value.cp;
					new_weapon->accepted_ammo = (char *)malloc(sizeof(char) * (strlen(ammo) + 1));
					strcpy(new_weapon->accepted_ammo, ammo);
					new_weapon->accepted_ammo[strlen(new_weapon->accepted_ammo)] = 0;
				}
			}
			
			if (add_weapon(new_weapon) != 0) {
				free_weapon(new_weapon);
				break;
			}
		}
	}
	
	/* free the parser handle */
	if ((esf_close_handle(weapons_esf)) != 0)
		assert(0);
	
	/* now repeat the process above but do it for ammo (simpler) */
	/* get a parser handle */
	weapons_esf = esf_new_handle();
	assert(weapons_esf);
	
	/* set the filter to look for weapons */
	if (esf_set_filter(weapons_esf, "ammo") != 0)
		assert(0);
	
	if (esf_parse_file_eaf(eaf, weapons_esf, filename) != 0) {
		printf("Error: Could not parse \"weapons.esf\" in EAF archive.\n");
	} else {
		/* parsed successfully, so get the data */
		int i, j;
		
		for (i = 0; i < weapons_esf->num_items; i++) {
			struct _ammo *new_ammo = create_ammo();
			
			for (j = 0; j < weapons_esf->items[i].num_keys; j++) {
				char *name = weapons_esf->items[i].keys[j].name;
				
				if (!strcmp(name, "Name")) {
					/* set the weapon name */
					char *ammo_name = weapons_esf->items[i].keys[j].value.cp;
					new_ammo->name = (char *)malloc(sizeof(char) * (strlen(ammo_name) + 1));
					strcpy(new_ammo->name, ammo_name);
					new_ammo->name[strlen(new_ammo->name)] = 0;
				} else if (!strcmp(name, "Adds to")) {
					/* set the adds to parameter */
					char *adds_to = weapons_esf->items[i].keys[j].value.cp;
					new_ammo->adds_to = (char *)malloc(sizeof(char) * (strlen(adds_to) + 1));
					strcpy(new_ammo->adds_to, adds_to);
					new_ammo->adds_to[strlen(new_ammo->adds_to)] = 0;
				} else if (!strcmp(name, "Quantity")) {
					new_ammo->quantity = weapons_esf->items[i].keys[j].value.i;
				}
			}
			
			if (add_ammo(new_ammo) != 0) {
				free_ammo(new_ammo);
				break;
			}
		}
	}
	
	/* free the parser handle */
	if ((esf_close_handle(weapons_esf)) != 0)
		assert(0);
	
	return (0);
}

int unload_weapons(void) {
	int i;

	for (i = 0; i < num_weapons; i++)
		free_weapon(weapons[i]);

	num_weapons = 0;

	for (i = 0; i < num_ammo; i++)
	  free_ammo(ammos[i]);

	num_ammo = 0;

	return (0);
}

/* creates a weapon struct - returns pointer to struct on success, NULL on failure */
static struct _weapon *create_weapon(void) {
	struct _weapon *weapon = (struct _weapon *)malloc(sizeof(struct _weapon));

	if (weapon == NULL)
		return (NULL);

	weapon->name = NULL;
	weapon->ammo_image = NULL;
	weapon->accepted_ammo = NULL;
	weapon->lifetime = 0;
	weapon->velocity = 0.0f;
	weapon->strength = 0;

	return (weapon);
}

static struct _ammo *create_ammo(void) {
	struct _ammo *ammo = (struct _ammo *)malloc(sizeof(struct _ammo));
	
	if (!ammo)
		return (NULL);
	
	ammo->name = NULL;
	ammo->adds_to = NULL;
	ammo->quantity = 0;
	
	return (ammo);
}

/* frees a weapon struct, returns 0 on success */
static int free_weapon(struct _weapon *weapon) {
	assert(weapon != NULL);

	if (weapon->ammo_image) {
		SDL_FreeSurface(weapon->ammo_image);
		weapon->ammo_image = NULL;
	}

	if (weapon->name) {
		free(weapon->name);
		weapon->name = NULL;
	}

	if (weapon->accepted_ammo) {
		free(weapon->accepted_ammo);
		weapon->accepted_ammo = NULL;
	}

	free(weapon);
	weapon = NULL;

	return (0);
}

static int free_ammo(struct _ammo *ammo) {
	assert(ammo);
	
	if (ammo->name)
		free(ammo->name);
	if (ammo->adds_to)
		free(ammo->adds_to);
	
	free(ammo);
	ammo = NULL;
	
	return (0);
}

struct _weapon *get_weapon_pointer(char *name) {
	int i;
	int length = strlen(name) - 1;

	for (i = 0; i < num_weapons; i++) {
		if (!strncmp(weapons[i]->name, name, length)) {
			return (weapons[i]);
		}
	}

	return (NULL);
}

struct _ammo *get_ammo_pointer(char *name) {
	int i;
	int length = strlen(name) - 1;
	
	for (i = 0; i < num_ammo; i++) {
		if (!strncmp(ammos[i]->name, name, length))
			return (ammos[i]);
	}
	
	return (NULL);
}

/* pass a ship pointer and weapon pointer to equip that weapon to that ship */
int equip_weapon_player(struct _weapon *weap) {
	int i, slot = -1;

	if (!weap)
		return (-1);

	/* find a free weapon slot */
	for (i = 0; i < MAX_WEAPON_SLOTS; i++) {
		if (!player.ship->weapon_mount[i]) {
			slot = i;
			break;
		}
	}

	if (slot == -1)
		return (-1);

	player.ship->weapon_mount[slot] = create_weapon_mount();
	player.ship->weapon_mount[slot]->weapon = weap;
	printf("THIS IS WHERE THE PLAYER SHOULD BE ABLE TO CONFIGURE THE X,Y AND ANGLE\n");
	player.ship->weapon_mount[slot]->weapon = weap;
	player.ship->weapon_mount[slot]->x = 0;
	player.ship->weapon_mount[slot]->y = 0;
	player.ship->weapon_mount[slot]->angle = 0;
	player.ship->weapon_mount[slot]->range = 0;
	player.ship->weapon_mount[slot]->ammo = player.ship->weapon_mount[slot]->weapon->initial_ammo;
	player.ship->weapon_mount[slot]->max_ammo = player.ship->weapon_mount[slot]->weapon->initial_ammo;

	player.ship->weapon_mount[slot]->time = 0;

	if (player.ship->w_slot[0] == -1)
		player.ship->w_slot[0] = slot;
	else if (player.ship->w_slot[1] == -1)
		player.ship->w_slot[1] = slot;

	return (0);
}
