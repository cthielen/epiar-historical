#include "com_defs.h"
#include "game/scenario.h"
#include "includes.h"
#include "sprite/sprite.h"
#include "sprite/type.h"
#include "system/eaf.h"
#include "system/esf.h"
#include "system/path.h"
#include "system/video/video.h"

int num_types = 0;

static struct _type *create_type(void);
static int free_type(struct _type *type);
static int add_type(struct _type *type);

static int add_type(struct _type *type) {
	if (num_types < MAX_TYPES) {
		int i;
		
		types[num_types] = type;
		assert(type->image);
		for (i = 0; i < MAX_WEAPON_SLOTS; i++) {
			if (!type->default_mounts[i])
				break;
		}
		
		num_types++;
		return (0);
	} else {
#ifndef NDEBUG
		fprintf(stdout, "Too many types. Could not add another.\n");
#endif
	}
	
	return (-1);
}

int load_types_eaf(FILE *eaf, char *filename) {
	parsed_file *types_esf = NULL;
	int i, j, k, l;
	
	if (!eaf)
		return (-1);
	if (!filename)
		return (-1);
	
	if ((types_esf = esf_new_handle()) == NULL) {
		printf("Couldn't allocate memory for parser handle\n");
		return (-1);
	}
	
	if (esf_set_filter(types_esf, "type") != 0) {
		printf("Could not set parsing filter for \"%s\"\n", filename);
		return (-1);
	}
	
	if (esf_parse_file_eaf(eaf, types_esf, filename) != 0) {
		printf("Could not parse \"%s\"\n", filename);
		return (-1);
	} else {
		for (i = 0; i < types_esf->num_items; i++) {
			struct _type *new_type = create_type();
			assert(new_type);
			
			/* run through the parsed values and create ship types */
			for (j = 0; j < types_esf->items[i].num_keys; j++) {
				char *name = types_esf->items[i].keys[j].name;
				assert(name);
				
				if (!strcmp(name, "Name")) {
					char *type_name = types_esf->items[i].keys[j].value.cp;
					
					new_type->name = (char *)malloc(sizeof(char) * (strlen(type_name) + 1));
					memset(new_type->name, 0, sizeof(char) * (strlen(type_name) + 1));
					strcpy(new_type->name, type_name);
				} else if (!strcmp(name, "Image")) {
					char *image = types_esf->items[i].keys[j].value.cp;
					SDL_Surface *temp = NULL;
					
					temp = eaf_load_png(eaf, image);
					if (temp == NULL) {
						temp = eaf_load_png(main_eaf, image);
						if (temp == NULL) {
							fprintf(stdout, "Could not load \"%s\".\n", apply_game_path(image));
						}
					}
					
					if (temp != NULL) {
						SDL_SetColorKey(temp, SDL_RLEACCEL | SDL_SRCCOLORKEY, SDL_MapRGB(temp->format, 0, 0, 0));
						new_type->image = SDL_DisplayFormat(temp);
						SDL_FreeSurface(temp);
					}
				} else if (!strcmp(name, "Wireframe")) {
					char *wireframe = types_esf->items[i].keys[j].value.cp;
					
					new_type->wireframe = (char *)malloc(sizeof(char) * (strlen(wireframe) + 1));
					memset(new_type->wireframe, 0, sizeof(char) * (strlen(wireframe) + 1));
					strcpy(new_type->wireframe, wireframe);
				} else if (!strcmp(name, "Comm Front")) {
					char *comm_front = types_esf->items[i].keys[j].value.cp;
					
					new_type->comm_front = (char *)malloc(sizeof(char) * (strlen(comm_front) + 1));
					memset(new_type->comm_front, 0, sizeof(char) * (strlen(comm_front) + 1));
					strcpy(new_type->comm_front, comm_front);
				} else if (!strcmp(name, "Engine")) {
					char *engine = types_esf->items[i].keys[j].value.cp;
					
					new_type->engine = get_engine_pointer(engine);
				} else if (!strcmp(name, "Shield")) {
					char *shield = types_esf->items[i].keys[j].value.cp;
					
					new_type->shield = get_shield_pointer(shield);
				} else if (!strcmp(name, "Mass")) {
					int mass = types_esf->items[i].keys[j].value.i;
					
					new_type->mass = mass;
				} else if (!strcmp(name, "Fuel")) {
					int fuel = types_esf->items[i].keys[j].value.i;
					
					new_type->total_fuel = fuel;
				} else if (!strcmp(name, "Cargo")) {
					int cargo = types_esf->items[i].keys[j].value.i;
					
					new_type->cargo = cargo;
				} else if (!strcmp(name, "Hull Life")) {
					int hull_life = types_esf->items[i].keys[j].value.i;
					
					new_type->hull_life = hull_life;
				} else if (!strcmp(name, "Price")) {
					int price = types_esf->items[i].keys[j].value.i;
					
					new_type->price = price;
				} else if (!strcmp(name, "Rotate")) {
					int str = types_esf->items[i].keys[j].value.i;
					
					new_type->str = str;
				} else if (!strcmp(name, "Needed Shield Radius")) {
					int radius = types_esf->items[i].keys[j].value.i;
					
					new_type->radius = radius;
				} else if (!strcmp(name, "Class")) {
					char *class_type = types_esf->items[i].keys[j].value.cp;
					assert(class_type);
					
					if (!strcmp(class_type, "Light"))
						new_type->class_type = LIGHT;
					else if (!strcmp(class_type, "Medium"))
						new_type->class_type = MEDIUM;
					else if (!strcmp(class_type, "Heavy"))
						new_type->class_type = HEAVY;
					else if (!strcmp(class_type, "Capitol"))
						new_type->class_type = CAPITOL;
					else if (!strcmp(class_type, "Cargo"))
						new_type->class_type = CARGO;
				}
			}
			/* the subitem's must be the weapon mounts */
			for (k = 0; k < types_esf->items[i].num_subitems; k++) {
				new_type->default_mounts[k] = create_weapon_mount();
				assert(new_type->default_mounts[k]);
				
				for (l = 0; l < types_esf->items[i].subitems[k].num_keys; l++) {
					char *name;
					
					assert(types_esf->items[i].subitems[k].keys);
					name = types_esf->items[i].subitems[k].keys[l].name;
					assert(name);
					
					if (!strcmp(name, "Default")) {
						char *weapon = types_esf->items[i].subitems[k].keys[l].value.cp;
						
						new_type->default_mounts[k]->weapon = get_weapon_pointer(weapon);
					} else if (!strcmp(name, "X")) {
						int x = types_esf->items[i].subitems[k].keys[l].value.i;
						
						new_type->default_mounts[k]->x = x;
					} else if (!strcmp(name, "Y")) {
						int y = types_esf->items[i].subitems[k].keys[l].value.i;
						
						new_type->default_mounts[k]->y = y;
					} else if (!strcmp(name, "Angle")) {
						int angle = types_esf->items[i].subitems[k].keys[l].value.i;
						
						new_type->default_mounts[k]->angle = angle;
					} else if (!strcmp(name, "Range")) {
						int range = types_esf->items[i].subitems[k].keys[l].value.i;
						
						new_type->default_mounts[k]->range = range;
					} else if (!strcmp(name, "Quantity")) {
						int quantity = types_esf->items[i].subitems[k].keys[l].value.i;
						
						new_type->default_mounts[k]->default_quantity = quantity;
					}
				}
			}
			
			if (add_type(new_type) != 0) {
				free_type(new_type);
				break;
			}
		}
	}
	
	if (esf_close_handle(types_esf) != 0) {
		printf("Could not close parser handle\n");
		return (-1);
	}
	
	return (0);
}

int unload_types(void) {
	int i, j;
	
	for (i = 0; i < num_types; i++) {
		for (j = 0; j < MAX_WEAPON_SLOTS; j++) {
			if (types[i]->default_mounts[j]) {
				free_weapon_mount(types[i]->default_mounts[j]);
				types[i]->default_mounts[j] = NULL;
			}
		}
		free_type(types[i]);
	}
	
	num_types = 0;
	
	return (0);
}

/* returns type pointer on success, NULL on failure */
struct _type *get_type_pointer(char *name) {
	int i;

	if (!name) {
		printf("bad name during get_type_pointer\n");
		return (NULL);
	}

	for (i = 0; i < num_types; i++) {
		if (!strcmp(name, types[i]->name)) {
			return (types[i]);
		}
	}

	return (NULL);
}

/* returns pointer to created type on success, NULL on failure */
static struct _type *create_type(void) {
	int i;
	struct _type *type = (struct _type *)malloc(sizeof(struct _type));

	if (type == NULL)
		return (NULL);

	type->name = NULL;
	type->image = NULL;
	type->wireframe = NULL;
	type->comm_front = NULL;
	type->engine = NULL;
	type->shield = NULL;
	for (i = 0; i < MAX_WEAPON_SLOTS; i++)
		type->default_mounts[i] = NULL;
	type->mass = 0;
	type->cargo = 0;
	type->hull_life = 0;
	type->str = 0;
	type->radius = 0;
	type->price = 0;

	return (type);
}

/* returns 0 on success */
static int free_type(struct _type *type) {
	assert(type);

	if (type->image != NULL)
		SDL_FreeSurface(type->image);

	if (type->name)
		free(type->name);
	if (type->wireframe)
		free(type->wireframe);
	if (type->comm_front)
		free(type->comm_front);

	free(type);

	return (0);
}

struct _weapon_mount *create_weapon_mount(void) {
	struct _weapon_mount *weapon_mount = (struct _weapon_mount *)malloc(sizeof(struct _weapon_mount));

	if (weapon_mount == NULL)
		return (NULL);

	weapon_mount->weapon = NULL;
	weapon_mount->x = 0;
	weapon_mount->y = 0;
	weapon_mount->angle = 0;
	weapon_mount->range = 0;
	weapon_mount->auto_aim = 0;

	return (weapon_mount);
}

void free_weapon_mount(struct _weapon_mount *weapon_mount) {
	assert(weapon_mount);

	free(weapon_mount);
}

/* looks in loaded eaf and main eaf for the type and returns a pointer to description (must free pointer when done) */
char *get_description(struct _type *type) {
	char *desc = NULL;
	parsed_file *types_esf = NULL;
	int i, j;
	unsigned char found_type = 0;

	if (!type)
		return (NULL);

	types_esf = esf_new_handle();
	assert(types_esf);

	if (esf_set_filter(types_esf, "type") != 0) {
		printf("Couldn't set the parser filter type\n");
		esf_close_handle(types_esf);
		return (NULL);
	}

	if (esf_parse_file_eaf(main_eaf, types_esf, "types.esf") != 0) {
		printf("Couldn't parse type.esf file\n");
		esf_close_handle(types_esf);
		return (NULL);
	}

	/* look for our type, and if found, set it */
	for (i = 0; i < types_esf->num_items; i++) {
		found_type = 0;
		for (j = 0; j < types_esf->items[i].num_keys; j++) {
			char *key_name = types_esf->items[i].keys[j].name;

			if (!strcmp(key_name, "Name")) {
				char *text = types_esf->items[i].keys[j].value.cp;

				if (!strcmp(text, type->name))
					found_type = 1;
			} else if (!strcmp(key_name, "Description") && found_type) {
				char *text = types_esf->items[i].keys[j].value.cp;

				desc = (char *)malloc(sizeof(char) * (strlen(text) + 1));
				memset(desc, 0, sizeof(char) * (strlen(text) + 1));
				strcpy(desc, text);

				i = types_esf->num_items; /* escape from loop */
			}
		}
	}

	if (esf_close_handle(types_esf) != 0) {
		printf("Couldn't close types parser handle\n");
	}
	types_esf = NULL;

	/* if it was in main.esf, then return it, else, look through the scenario file now */
	if (desc)
		return (desc);

	/* repeat, looking through the scenario esf since it wasnt in desc */
	types_esf = esf_new_handle();
	assert(types_esf);

	if (esf_set_filter(types_esf, "type") != 0) {
		printf("Couldn't set the parser filter type\n");
		esf_close_handle(types_esf);
		return (NULL);
	}

	if (esf_parse_file_eaf(loaded_eaf, types_esf, "types.esf") != 0) {
		printf("Couldn't parse type.esf file\n");
		esf_close_handle(types_esf);
		return (NULL);
	}

	/* look for our type, and if found, set it */
	for (i = 0; i < types_esf->num_items; i++) {
		found_type = 0;
		for (j = 0; j < types_esf->items[i].num_keys; j++) {
			char *key_name = types_esf->items[i].keys[j].name;

			if (!strcmp(key_name, "Name")) {
				char *text = types_esf->items[i].keys[j].value.cp;

				if (!strcmp(text, type->name)) {
					found_type = 1;
				}
			} else if (!strcmp(key_name, "Description") && found_type) {
				char *text = types_esf->items[i].keys[j].value.cp;

				desc = (char *)malloc(sizeof(char) * (strlen(text) + 1));
				memset(desc, 0, sizeof(char) * (strlen(text) + 1));
				strcpy(desc, text);

				i = types_esf->num_items; /* escape from loop */
			}
		}
	}

	if (esf_close_handle(types_esf) != 0) {
		printf("Couldn't close types parser handle\n");
	}

	/* if we found it in scen_eaf, desc would be set */
	if (desc)
		return (desc);

	return (NULL);
}

/* looks in loaded eaf and main eaf for the type and returns a pointer to manufacturer (must free pointer when done) */
char *get_manufacturer(struct _type *type) {
	static char mname[35] = {0};
	unsigned char found_type = 0;
	parsed_file *types_esf = NULL;

	if (type == NULL)
		return (NULL);

	if ((types_esf = esf_new_handle()) == NULL) {
		printf("Couldn't find \"types.esf\" in EAF archive\n");
	} else {
		if (esf_set_filter(types_esf, "type") != 0) {
			printf("Couldn't set filter\n");
		} else {
			if (esf_parse_file_eaf(loaded_eaf, types_esf, "types.esf") != 0) {
				printf("Couldn't parse file\n");
			} else {
				int i, j;

				for (i = 0; (i < types_esf->num_items) && (!found_type); i++) {
					for (j = 0; j < types_esf->items[i].num_keys; j++) {
						char *name = types_esf->items[i].keys[j].name;

						if (!strcmp(name, "Name")) {
							if (!strcmp(type->name, types_esf->items[i].keys[j].value.cp))
								found_type = 1;
						}
						if (!strcmp(name, "Manufacturer") && found_type) {
							memset(mname, 0, sizeof(char) * 35);
							strcpy(mname, types_esf->items[i].keys[j].value.cp);
							break;
						}
					}
				}
			}
		}
		if (esf_close_handle(types_esf) != 0) {
			printf("Error closing parser handle\n");
		}
	}

	if (found_type)
		return (mname);

	/* couldn't find it in loaded eaf, so repeat the process for the main eaf */
	if ((types_esf = esf_new_handle()) == NULL) {
		printf("Couldn't find \"types.esf\" in EAF archive\n");
	} else {
		if (esf_set_filter(types_esf, "type") != 0) {
			printf("Couldn't set filter\n");
		} else {
			if (esf_parse_file_eaf(main_eaf, types_esf, "types.esf") != 0) {
				printf("Couldn't parse file\n");
			} else {
				int i, j;

				for (i = 0; (i < types_esf->num_items) && (!found_type); i++) {
					for (j = 0; j < types_esf->items[i].num_keys; j++) {
						char *name = types_esf->items[i].keys[j].name;

						if (!strcmp(name, "Name")) {
							if (!strcmp(type->name, types_esf->items[i].keys[j].value.cp))
								found_type = 1;
						}
						if (!strcmp(name, "Manufacturer") && found_type) {
							memset(mname, 0, sizeof(char) * 35);
							strcpy(mname, types_esf->items[i].keys[j].value.cp);
							break;
						}
					}
				}
			}
		}
		if (esf_close_handle(types_esf) != 0) {
			printf("Error closing parser handle\n");
		}
	}

	if (found_type)
		return (mname);

	return (NULL);
}
