#include "com_defs.h"
#include "game/update.h"
#include "game/scenario.h"
#include "includes.h"
#include "sprite/sprite.h"
#include "sprite/model.h"
#include "system/eaf.h"
#include "system/esf.h"
#include "system/path.h"
#include "system/video/video.h"

int num_models = 0;

static model_t *create_model(void);
static int free_model(model_t *model);
static int add_model(model_t *model);

static int add_model(model_t *model) {
	if (num_models < MAX_MODELS) {
		int i;
		
		models[num_models] = model;
		assert(model->image);
		for (i = 0; i < MAX_WEAPON_SLOTS; i++) {
			if (!model->default_mounts[i])
				break;
		}
		
		num_models++;
		return (0);
	} else {
#ifndef NDEBUG
		fprintf(stdout, "Too many models. Could not add another.\n");
#endif
	}
	
	return (-1);
}

int load_models_eaf(FILE *eaf, char *filename) {
	parsed_file *models_esf = NULL;
	int i, j, k, l, m;
	
	if (!eaf)
		return (-1);
	if (!filename)
		return (-1);
	
	if ((models_esf = esf_new_handle()) == NULL) {
		printf("Couldn't allocate memory for parser handle\n");
		return (-1);
	}
	
	if (esf_set_filter(models_esf, "model") != 0) {
		printf("Could not set parsing filter for \"%s\"\n", filename);
		return (-1);
	}
	
	if (esf_parse_file_eaf(eaf, models_esf, filename) != 0) {
		printf("Could not parse \"%s\"\n", filename);
		return (-1);
	} else {
		for (i = 0; i < models_esf->num_items; i++) {
			model_t *new_model = create_model();
			assert(new_model);
			
			/* run through the parsed values and create ship types */
			for (j = 0; j < models_esf->items[i].num_keys; j++) {
				char *name = models_esf->items[i].keys[j].name;
				assert(name);
				
				if (!strcmp(name, "Name")) {
					char *model_name = models_esf->items[i].keys[j].value.cp;
					
					new_model->name = (char *)malloc(sizeof(char) * (strlen(model_name) + 1));
					memset(new_model->name, 0, sizeof(char) * (strlen(model_name) + 1));
					strcpy(new_model->name, model_name);
				} else if (!strcmp(name, "Image")) {
					char *image = models_esf->items[i].keys[j].value.cp;
					SDL_Surface *temp = NULL;
					
					temp = eaf_load_png(eaf, image);
					if (temp == NULL) {
						temp = eaf_load_png(main_eaf, image);
						if (temp == NULL) {
							fprintf(stdout, "Could not load \"%s\".\n", apply_game_path(image));
						}
					}

					if (temp) {
						SDL_SetColorKey(temp, SDL_RLEACCEL | SDL_SRCCOLORKEY, SDL_MapRGB(temp->format, 0, 0, 0));
						new_model->image = SDL_DisplayFormat(temp);
						SDL_FreeSurface(temp);
					}
				} else if (!strcmp(name, "Wireframe")) {
					char *wireframe = models_esf->items[i].keys[j].value.cp;
					
					new_model->wireframe = (char *)malloc(sizeof(char) * (strlen(wireframe) + 1));
					memset(new_model->wireframe, 0, sizeof(char) * (strlen(wireframe) + 1));
					strcpy(new_model->wireframe, wireframe);
				} else if (!strcmp(name, "Comm Front")) {
					char *comm_front = models_esf->items[i].keys[j].value.cp;
					
					new_model->comm_front = (char *)malloc(sizeof(char) * (strlen(comm_front) + 1));
					memset(new_model->comm_front, 0, sizeof(char) * (strlen(comm_front) + 1));
					strcpy(new_model->comm_front, comm_front);
				} else if (!strcmp(name, "Engine")) {
					char *engine = models_esf->items[i].keys[j].value.cp;
					
					new_model->default_engine = get_engine_pointer(engine);
				} else if (!strcmp(name, "Shield")) {
					char *shield = models_esf->items[i].keys[j].value.cp;
					
					new_model->default_shield = get_shield_pointer(shield);
				} else if (!strcmp(name, "Fuel")) {
					int fuel = models_esf->items[i].keys[j].value.i;
					
					new_model->total_fuel = fuel;
				} else if (!strcmp(name, "Cargo")) {
					int cargo = models_esf->items[i].keys[j].value.i;
					
					new_model->cargo = cargo;
				} else if (!strcmp(name, "Hull Life")) {
					int hull_life = models_esf->items[i].keys[j].value.i;
					
					new_model->hull_life = hull_life;
				} else if (!strcmp(name, "Price")) {
					int price = models_esf->items[i].keys[j].value.i;
					
					new_model->price = price;
				} else if (!strcmp(name, "Rotate")) {
					int str = models_esf->items[i].keys[j].value.i;
					
					new_model->str = str;
				} else if (!strcmp(name, "Needed Shield Radius")) {
					int radius = models_esf->items[i].keys[j].value.i;
					
					new_model->radius = radius;
				} else if (!strcmp(name, "Class")) {
					char *class_type = models_esf->items[i].keys[j].value.cp;
					assert(class_type);
					
					if (!strcmp(class_type, "Light"))
						new_model->class = MODEL_LIGHT;
					else if (!strcmp(class_type, "Medium"))
						new_model->class = MODEL_MEDIUM;
					else if (!strcmp(class_type, "Heavy"))
						new_model->class = MODEL_HEAVY;
					else if (!strcmp(class_type, "Capitol"))
						new_model->class = MODEL_CAPITOL;
					else if (!strcmp(class_type, "Cargo"))
						new_model->class = MODEL_CARGO;
				}
			}
			/* the subitem's must be the weapon mounts */
			for (k = 0; k < models_esf->items[i].num_subitems; k++) {
				new_model->default_mounts[k] = create_weapon_mount();
				assert(new_model->default_mounts[k]);
				
				for (l = 0; l < models_esf->items[i].subitems[k].num_keys; l++) {
					char *name;
					
					assert(models_esf->items[i].subitems[k].keys);
					name = models_esf->items[i].subitems[k].keys[l].name;
					assert(name);
					
					if (!strcmp(name, "Default")) {
						char *weapon = models_esf->items[i].subitems[k].keys[l].value.cp;
						
						new_model->default_mounts[k]->weapon = get_weapon_pointer(weapon);
					} else if (!strcmp(name, "X")) {
						int x = models_esf->items[i].subitems[k].keys[l].value.i;
						
						new_model->default_mounts[k]->x = x;
					} else if (!strcmp(name, "Y")) {
						int y = models_esf->items[i].subitems[k].keys[l].value.i;
						
						new_model->default_mounts[k]->y = y;
					} else if (!strcmp(name, "Angle")) {
						int angle = models_esf->items[i].subitems[k].keys[l].value.i;
						
						new_model->default_mounts[k]->angle = angle;
					} else if (!strcmp(name, "Range")) {
						int range = models_esf->items[i].subitems[k].keys[l].value.i;
						
						new_model->default_mounts[k]->range = range;
					} else if (!strcmp(name, "Quantity")) {
						int quantity = models_esf->items[i].subitems[k].keys[l].value.i;
						
						new_model->default_mounts[k]->max_ammo = quantity;
						new_model->default_mounts[k]->ammo = quantity;
					}
				}
			}

			for (m = 0; m < MAX_CACHED_ROTATIONS; m++)
				new_model->cached[m] = NULL;

			new_model->cache_expiration = 0;

			if (add_model(new_model) != 0) {
				free_model(new_model);
				break;
			}
		}
	}
	
	if (esf_close_handle(models_esf) != 0) {
		printf("Could not close parser handle\n");
		return (-1);
	}
	
	return (0);
}

int unload_models(void) {
	int i, j;
	
	for (i = 0; i < num_models; i++) {
		for (j = 0; j < MAX_WEAPON_SLOTS; j++) {
			if (models[i]->default_mounts[j]) {
				free_weapon_mount(models[i]->default_mounts[j]);
				models[i]->default_mounts[j] = NULL;
			}
		}
		for (j = 0; j < MAX_CACHED_ROTATIONS; j++)
			if (models[i]->cached[j])
				SDL_FreeSurface(models[i]->cached[j]);
		free_model(models[i]);
	}
	
	num_models = 0;
	
	return (0);
}

/* returns type pointer on success, NULL on failure */
model_t *get_model_pointer(char *name) {
	int i;

	if (!name) {
#ifndef NDEBUG
		printf("bad name during get_model_pointer\n");
#endif
		return (NULL);
	}

	for (i = 0; i < num_models; i++) {
		if (!strcmp(name, models[i]->name)) {
			return (models[i]);
		}
	}

	return (NULL);
}

/* returns pointer to created type on success, NULL on failure */
static model_t *create_model(void) {
	int i;
	model_t *model = (model_t *)malloc(sizeof(model_t));

	if (!model)
		return (NULL);

	model->name = NULL;
	model->image = NULL;
	model->wireframe = NULL;
	model->comm_front = NULL;
	model->default_engine = NULL;
	model->default_shield = NULL;
	for (i = 0; i < MAX_WEAPON_SLOTS; i++)
		model->default_mounts[i] = NULL;
	model->cargo = 0;
	model->hull_life = 0;
	model->str = 0;
	model->radius = 0;
	model->price = 0;

	return (model);
}

/* returns 0 on success */
static int free_model(model_t *model) {
	assert(model);

	if (model->image)
		SDL_FreeSurface(model->image);

	if (model->name)
		free(model->name);
	if (model->wireframe)
		free(model->wireframe);
	if (model->comm_front)
		free(model->comm_front);

	free(model);

	return (0);
}

weapon_mount_t *copy_weapon_mount(weapon_mount_t *mount) {
	weapon_mount_t *weapon_mount = (weapon_mount_t *)malloc(sizeof(weapon_mount_t));

	if (!weapon_mount)
		return (NULL);

	weapon_mount->weapon = mount->weapon;
	weapon_mount->x = mount->x;
	weapon_mount->y = mount->y;
	weapon_mount->angle = mount->angle;
	weapon_mount->range = mount->range;
	weapon_mount->time = 0;
	weapon_mount->ammo = mount->ammo;
	weapon_mount->max_ammo = mount->max_ammo;

	return (weapon_mount);
}

weapon_mount_t *create_weapon_mount(void) {
	weapon_mount_t *weapon_mount = (weapon_mount_t *)malloc(sizeof(weapon_mount_t));

	if (!weapon_mount)
		return (NULL);

	weapon_mount->weapon = NULL;
	weapon_mount->x = 0;
	weapon_mount->y = 0;
	weapon_mount->angle = 0;
	weapon_mount->range = 0;
	weapon_mount->time = 0;
	weapon_mount->ammo = 0;
	weapon_mount->max_ammo = 0;

	return (weapon_mount);
}

void free_weapon_mount(struct _weapon_mount *weapon_mount) {
	assert(weapon_mount);

	free(weapon_mount);
}

/* looks in loaded eaf and main eaf for the type and returns a pointer to description (must free pointer when done) */
char *model_get_description(model_t *model) {
	char *desc = NULL;
	parsed_file *models_esf = NULL;
	int i, j;
	unsigned char found_model = 0;

	if (!model)
		return (NULL);

	models_esf = esf_new_handle();
	assert(models_esf);

	if (esf_set_filter(models_esf, "model") != 0) {
		printf("Couldn't set the parser filter type\n");
		esf_close_handle(models_esf);
		return (NULL);
	}

	if (esf_parse_file_eaf(main_eaf, models_esf, "models.esf") != 0) {
		printf("Couldn't parse models.esf file\n");
		esf_close_handle(models_esf);
		return (NULL);
	}

	/* look for our type, and if found, set it */
	for (i = 0; i < models_esf->num_items; i++) {
		found_model = 0;
		for (j = 0; j < models_esf->items[i].num_keys; j++) {
			char *key_name = models_esf->items[i].keys[j].name;

			if (!strcmp(key_name, "Name")) {
				char *text = models_esf->items[i].keys[j].value.cp;

				if (!strcmp(text, model->name))
					found_model = 1;
			} else if (!strcmp(key_name, "Description") && found_model) {
				char *text = models_esf->items[i].keys[j].value.cp;

				desc = (char *)malloc(sizeof(char) * (strlen(text) + 1));
				memset(desc, 0, sizeof(char) * (strlen(text) + 1));
				strcpy(desc, text);

				i = models_esf->num_items; /* escape from loop */
				break;
			}
		}
	}

	if (esf_close_handle(models_esf) != 0) {
		printf("Couldn't close types parser handle\n");
	}
	models_esf = NULL;

	/* if it was in main.esf, then return it, else, look through the scenario file now */
	if (desc)
		return (desc);

	/* repeat, looking through the scenario esf since it wasnt in desc */
	models_esf = esf_new_handle();
	assert(models_esf);

	if (esf_set_filter(models_esf, "model") != 0) {
		printf("Couldn't set the parser filter type\n");
		esf_close_handle(models_esf);
		return (NULL);
	}

	if (esf_parse_file_eaf(loaded_eaf, models_esf, "models.esf") != 0) {
		printf("Couldn't parse models.esf file\n");
		esf_close_handle(models_esf);
		return (NULL);
	}

	/* look for our type, and if found, set it */
	for (i = 0; i < models_esf->num_items; i++) {
		found_model = 0;
		for (j = 0; j < models_esf->items[i].num_keys; j++) {
			char *key_name = models_esf->items[i].keys[j].name;

			if (!strcmp(key_name, "Name")) {
				char *text = models_esf->items[i].keys[j].value.cp;

				if (!strcmp(text, model->name)) {
					found_model = 1;
				}
			} else if (!strcmp(key_name, "Description") && found_model) {
				char *text = models_esf->items[i].keys[j].value.cp;

				desc = (char *)malloc(sizeof(char) * (strlen(text) + 1));
				memset(desc, 0, sizeof(char) * (strlen(text) + 1));
				strcpy(desc, text);

				i = models_esf->num_items; /* escape from loop */
			}
		}
	}

	if (esf_close_handle(models_esf) != 0) {
		printf("Couldn't close types parser handle\n");
	}

	/* if we found it in scen_eaf, desc would be set */
	if (desc)
		return (desc);

	return (NULL);
}

/* looks in loaded eaf and main eaf for the type and returns a pointer to manufacturer (must free pointer when done) */
char *model_get_manufacturer(model_t *model) {
	static char mname[35] = {0};
	unsigned char found_model = 0;
	parsed_file *models_esf = NULL;

	if (!model)
		return (NULL);

	if ((models_esf = esf_new_handle()) == NULL) {
		printf("Couldn't find \"models.esf\" in EAF archive\n");
	} else {
		if (esf_set_filter(models_esf, "model") != 0) {
			printf("Couldn't set filter\n");
		} else {
			if (esf_parse_file_eaf(loaded_eaf, models_esf, "models.esf") != 0) {
				printf("Couldn't parse file \"models.esf\"\n");
			} else {
				int i, j;

				for (i = 0; (i < models_esf->num_items) && (!found_model); i++) {
					for (j = 0; j < models_esf->items[i].num_keys; j++) {
						char *name = models_esf->items[i].keys[j].name;

						if (!strcmp(name, "Name")) {
							if (!strcmp(model->name, models_esf->items[i].keys[j].value.cp))
								found_model = 1;
						}
						if (!strcmp(name, "Manufacturer") && found_model) {
							memset(mname, 0, sizeof(char) * 35);
							strcpy(mname, models_esf->items[i].keys[j].value.cp);
							break;
						}
					}
				}
			}
		}
		if (esf_close_handle(models_esf) != 0) {
			printf("Error closing parser handle\n");
		}
	}

	if (found_model)
		return (mname);

	/* couldn't find it in loaded eaf, so repeat the process for the main eaf */
	if ((models_esf = esf_new_handle()) == NULL) {
		printf("Couldn't find \"models.esf\" in EAF archive\n");
	} else {
		if (esf_set_filter(models_esf, "model") != 0) {
			printf("Couldn't set filter\n");
		} else {
			if (esf_parse_file_eaf(main_eaf, models_esf, "models.esf") != 0) {
				printf("Couldn't parse file\n");
			} else {
				int i, j;

				for (i = 0; (i < models_esf->num_items) && (!found_model); i++) {
					for (j = 0; j < models_esf->items[i].num_keys; j++) {
						char *name = models_esf->items[i].keys[j].name;

						if (!strcmp(name, "Name")) {
							if (!strcmp(model->name, models_esf->items[i].keys[j].value.cp))
								found_model = 1;
						}
						if (!strcmp(name, "Manufacturer") && found_model) {
							memset(mname, 0, sizeof(char) * 35);
							strcpy(mname, models_esf->items[i].keys[j].value.cp);
							break;
						}
					}
				}
			}
		}
		if (esf_close_handle(models_esf) != 0) {
			printf("Error closing parser handle\n");
		}
	}

	if (found_model)
		return (mname);

	return (NULL);
}

void clean_models(void) {
	int i, j;

	for (i = 0; i < num_models; i++) {
		if (models[i]->cache_expiration < current_time) {
			for (j = 0; j < MAX_CACHED_ROTATIONS; j++) {
				SDL_FreeSurface(models[i]->cached[j]);
				models[i]->cached[j] = NULL;
			}
		}
	}
}
