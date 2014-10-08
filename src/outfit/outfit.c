#include "gui/gui.h"
#include "navigation/navigation.h"
#include "outfit/outfit.h"
#include "sprite/sprite.h"
#include "sprite/model.h"
#include "sprite/weapon.h"
#include "system/eaf.h"
#include "system/esf.h"

outfit_item *outfits[MAX_OUTFITS];
int num_outfits = 0;

static int add_outfit_item(outfit_item *item);
static outfit_item *create_new_outfit(void);

int load_outfits_eaf(FILE *eaf, char *filename) {
	parsed_file *outfits_esf = NULL;
	int i, j, k, l;
	
	if (!eaf)
		return (-1);
	if (!filename)
		return (-1);
	if (strlen(filename) <= 0)
		return (-1);
	
	outfits_esf = esf_new_handle();
	assert(outfits_esf);
	
	if (esf_set_filter(outfits_esf, "outfit") != 0) {
		printf("Couldn't set parser filter\n");
		esf_close_handle(outfits_esf);
		return (-1);
	}
	
	if (esf_parse_file_eaf(eaf, outfits_esf, filename) != 0) {
		printf("Couldn't parse \"%s\"\n", filename);
		esf_close_handle(outfits_esf);
		return (-1);
	}
	
	for (i = 0; i < outfits_esf->num_items; i++) {
		outfit_item *item = create_new_outfit();
		
		for (j = 0; j < outfits_esf->items[i].num_keys; j++) {
			char *name = outfits_esf->items[i].keys[j].name;
			
			if (!strcmp(name, "Name")) {
				char *text = outfits_esf->items[i].keys[j].value.cp;
				
				item->name = (char *)malloc(sizeof(char) * (strlen(text) + 1));
				memset(item->name, 0, sizeof(char) * (strlen(text) + 1));
				strcpy(item->name, text);
			} else if (!strcmp(name, "Generic Type")) {
				char *text = outfits_esf->items[i].keys[j].value.cp;
				
				if (!strcmp(text, "Additional"))
					item->generic = ADDITIONAL;
				else
					item->generic = MODIFIER;
			} else if (!strcmp(name, "Specific Type")) {
				char *text = outfits_esf->items[i].keys[j].value.cp;
				
				if (!strcmp(text, "Weapon Mod")) {
					item->specific = WEAPON;
					item->data = (weapon_item *)malloc(sizeof(weapon_item));
				} else if (!strcmp(text, "Ammo Mod")) {
					item->specific = AMMO;
					item->data = (ammo_item *)malloc(sizeof(ammo_item));
				} else if (!strcmp(text, "Hull Mod")) {
					item->specific = HULL;
					item->data = (hull_item *)malloc(sizeof(hull_item));
				} else if (!strcmp(text, "Death Mod")) {
					item->specific = DEATH;
					item->data = (death_item *)malloc(sizeof(death_item));
				} else if (!strcmp(text, "Engine Mod")) {
					item->specific = ENGINE;
					item->data = (engine_item *)malloc(sizeof(engine_item));
				} else if (!strcmp(text, "Map Mod")) {
					map_item *mdata;

					item->specific = MAP;
					item->data = (map_item *)malloc(sizeof(map_item));
					mdata = (map_item *)item->data;
					mdata->reveal = NULL;
				} else if (!strcmp(text, "Hud Mod")) {
					hud_item *hdata;

					item->specific = HUD;
					item->data = (hud_item *)malloc(sizeof(hud_item));
					hdata = (hud_item *)item->data;
					hdata->show_target_shield = 0;
					hdata->show_target_hull = 0;
					hdata->show_target_shield_units = 0;
					hdata->show_target_hull_units = 0;
				} else {
					printf("Unknown specific type while loading outfits\n");
				}
			} else if (!strcmp(name, "Required Free Mass")) {
				int mass = outfits_esf->items[i].keys[j].value.i;
				
				item->needed_free_mass = mass;
			} else if (!strcmp(name, "Price")) {
				int price = outfits_esf->items[i].keys[j].value.i;
				
				item->price = price;
			} else if (!strcmp(name, "Description")) {
				char *text = outfits_esf->items[i].keys[j].value.cp;
	
				item->desc = (char *)malloc(sizeof(char) * (strlen(text) + 1));
				memset(item->desc, 0, sizeof(char) * (strlen(text) + 1));
				strcpy(item->desc, text);
			} else if (!strcmp(name, "Stock Image")) {
				char *text = outfits_esf->items[i].keys[j].value.cp;
				
				item->stock = (char *)malloc(sizeof(char) * (strlen(text) + 1));
				memset(item->stock, 0, sizeof(char) * (strlen(text) + 1));
				strcpy(item->stock, text);
			}
		}
    
		/* go through subitem (item->data specific data) */
		for (k = 0; k < outfits_esf->items[i].num_subitems; k++) {
			for (l = 0; l < outfits_esf->items[i].subitems[k].num_keys; l++) {
				char *name = outfits_esf->items[i].subitems[k].keys[l].name;
				
				if (item->specific == WEAPON) {
					if (!strcmp(name, "Name")) {
						char *text = outfits_esf->items[i].subitems[k].keys[l].value.cp;
						weapon_item *wdata = (weapon_item *)item->data;
						
						wdata->weapon = get_weapon_pointer(text);
						assert(wdata->weapon);
					}
				} else if (item->specific == AMMO) {
					if (!strcmp(name, "Name")) {
						char *text = outfits_esf->items[i].subitems[k].keys[l].value.cp;
						ammo_item *adata = (ammo_item *)item->data;
						adata->ammo = get_ammo_pointer(text);
						assert(adata->ammo);
					}
				} else if (item->specific == HULL) {
					if (!strcmp(name, "Name")) {
						char *text = outfits_esf->items[i].subitems[k].keys[l].value.cp;
						hull_item *hdata = (hull_item *)item->data;
						hdata->name = (char *)malloc(sizeof(char) * (strlen(text) + 1));
						memset(hdata->name, 0, sizeof(char) * (strlen(text) + 1));
						strcpy(hdata->name, text);
					} else if (!strcmp(name, "Modifier")) {
						hull_item *hdata = (hull_item *)item->data;
						hdata->multiplier = outfits_esf->items[i].subitems[k].keys[l].value.f;
					} else if (!strcmp(name, "Self-Repair")) {
						hull_item *hdata = (hull_item *)item->data;
						hdata->repairing = (unsigned char)outfits_esf->items[i].subitems[k].keys[l].value.i;
					}
				} else if (item->specific == DEATH) {
					if (!strcmp(name, "New Ship")) {
						death_item *ddata = (death_item *)item->data;
						ddata->escape_ship = get_model_pointer(outfits_esf->items[i].subitems[k].keys[l].value.cp);
						assert(ddata->escape_ship);
					} else if (!strcmp(name, "Auto-Pilot")) {
						death_item *ddata = (death_item *)item->data;
						ddata->auto_pilot = (unsigned char)outfits_esf->items[i].subitems[k].keys[l].value.i;
					}
				} else if (item->specific == ENGINE) {
					if (!strcmp(name, "Is Booster")) {
						engine_item *eitem = (engine_item *)item->data;
						eitem->is_booster = (unsigned char)outfits_esf->items[i].subitems[k].keys[l].value.i;
					} else if (!strcmp(name, "Boost Strength")) {
						engine_item *eitem = (engine_item *)item->data;
						eitem->boost = outfits_esf->items[i].subitems[k].keys[l].value.f;
					}
				} else if (item->specific == MAP) {
					if (!strcmp(name, "Reveal")) {
						map_item *mdata = (map_item *)item->data;
						char *planet = outfits_esf->items[i].subitems[k].keys[l].value.cp;
						
						if (!mdata->reveal) {
							/* no item yet, so just have this one */
							mdata->reveal = (char *)malloc(sizeof(char) * (strlen(planet) + 1));
							memset(mdata->reveal, 0, sizeof(char) * (strlen(planet) + 1));
							strcpy(mdata->reveal, planet);
						} else {
							/* we need to construct a new list, free the old, and set the new as the normal list */
							char *new_list = NULL;
							
							new_list = (char *)malloc(sizeof(char) * (strlen(mdata->reveal) + 1 + strlen(planet) + 1));
							memset(new_list, 0, sizeof(char) * (strlen(mdata->reveal) + 1 + strlen(planet) + 1));
							strcpy(new_list, mdata->reveal);
							strcat(new_list, ",");
							strcat(new_list, planet);
							
							free(mdata->reveal);
							mdata->reveal = new_list;
						}
					}
				} else if (item->specific == HUD) {
					hud_item *hdata = (hud_item *)item->data;
					if (!strcmp(name, "Show Target Shield")) hdata->show_target_shield = 1;
					if (!strcmp(name, "Show Target Hull")) hdata->show_target_hull = 1;
					if (!strcmp(name, "Show Target Shield Units")) hdata->show_target_shield_units = 1;
					if (!strcmp(name, "Show Target Hull Units")) hdata->show_target_hull_units = 1;
				}
			}
		}
		
		add_outfit_item(item);
	}
  
	if (esf_close_handle(outfits_esf) != 0) {
		printf("Couldn't close parser handle\n");
		return (-1);
	}
	
	return (0);
}

int unload_outfits(void) {
	int i;
	
	for (i = 0; i < num_outfits; i++) {
		if (outfits[i]->specific == HULL) {
			hull_item *hdata = (hull_item *)outfits[i]->data;
			if (hdata->name)
				free(hdata->name);
		} else if (outfits[i]->specific == MAP) {
			map_item *mdata = (map_item *)outfits[i]->data;
			if (mdata->reveal)
				free(mdata->reveal);
		}
		if (outfits[i]->data) {
			free(outfits[i]->data);
		}
		free(outfits[i]->name);
		free(outfits[i]->desc);
		free(outfits[i]->stock);
		free(outfits[i]);
	}
	
	num_outfits = 0;
	
	return (0);
}

static int add_outfit_item(outfit_item *item) {
	assert(item);
	
	if (num_outfits < MAX_OUTFITS) {
		outfits[num_outfits] = item;
		num_outfits++;
	}
	
	return (0);
}

static outfit_item *create_new_outfit(void) {
	outfit_item *item = (outfit_item *)malloc(sizeof(outfit_item));
	
	assert(item);
	
	item->name = NULL;
	item->needed_free_mass = 0;
	item->price = 0;
	item->desc = NULL;
	item->stock = NULL;
	item->data = NULL;
	
	return (item);
}

/* returns a pointer to the first found outfit item w/ name 'name' */
outfit_item *get_outfit_pointer(char *name) {
	int i;
	
	if(!name) {
#ifndef NDEBUG
		printf("get_outfit_pointer: bad name\n");
#endif
		return (NULL);
	}
  
	for (i = 0; i < num_outfits; i++) {
		if (!strcmp(outfits[i]->name, name)) {
			return (outfits[i]);
		}
	}

#ifndef NDEBUG  
	printf("get_outfit_pointer: \"%s\" not found\n", name);
#endif
  
	return (NULL);
}

/* equips the player with a specific outfit_item */
int player_equip_outfit(outfit_item *item) {
	int i, slot = -1;

	for (i = 0; i < MAX_OUTFITS; i++) {
		if (player.upgrades[i] == NULL) {
			slot = i;
			break;
		}
	}

	if (slot == -1) {
#ifndef NDEBUG
		printf("Couldn't find a slot to equip outfit item \"%s\" for player.\n", item->name);
#endif		
		return (-1);
	}

	/* keep track of the item */
	player.upgrades[slot] = item;
	player.free_mass -= item->needed_free_mass;

	/* item specific equipping code */
	if (item->specific == DEATH) {
		death_item *ditem = (death_item *)item->data;

		player.ship->escape_pod = ditem->escape_ship;
	} else if (item->specific == MAP) {
		map_item *mitem = (map_item *)item->data;
		int i;
		char planet[80] = {0};

		for (i = 0; i < (signed)strlen(mitem->reveal); i++) {
			if (mitem->reveal[i] == ',') {
				nav_reveal_planet(planet);
				memset(planet, 0, sizeof(char) * 80);
			} else {
				int len = strlen(planet);
				planet[len] = mitem->reveal[i];
				planet[len + 1] = 0;
			}
		}

		nav_reveal_planet(planet); /* last planet doesnt have a comma after it */
	} else if (item->specific == ENGINE) {
		engine_item *edata = (engine_item *)item->data;

		if (edata->is_booster) {
			player.ship->has_booster = 1;
			player.ship->boost_strength = edata->boost;
		}
	} else if (item->specific == HULL) {
		hull_item *hdata = (hull_item *)item->data;

		if (player.ship->plating.name)
			free(player.ship->plating.name);

		/* set the hull's name */
		player.ship->plating.name = (char *)malloc(sizeof(char) * (strlen(hdata->name) + 1));
		memset(player.ship->plating.name, 0, sizeof(char) * (strlen(hdata->name) + 1));
		strcpy(player.ship->plating.name, hdata->name);

		player.ship->plating.amt = hdata->multiplier;
		player.ship->plating.repairing = hdata->repairing;
	} else if (item->specific == WEAPON) {
		weapon_item *wdata = (weapon_item *)item->data;

		if (equip_weapon_player(wdata->weapon) != 0)
			gui_alert("No free weapon slots.");
	} else if (item->specific == AMMO) {
		ammo_item *adata = (ammo_item *)item->data;
		struct _weapon *weap = get_weapon_pointer(adata->ammo->adds_to);
		unsigned char completed = 0;
		int i;

		/* in this special case, we dont want to remember we bought ammo b/c it's not something special to sell and doesn't need the tracking */
		player.upgrades[slot] = NULL;

		for (i = 0; i < MAX_WEAPON_SLOTS; i++) {
			if (player.ship->weapon_mount[i]) {
				if (player.ship->weapon_mount[i]->weapon == weap) {
					player.ship->weapon_mount[i]->ammo += adata->ammo->quantity;
					completed = 1;
					break;
				}
			}
		}

		if (!completed) {
			gui_alert("You have no weapon takes this ammo");
			return (-1);
		}
	}

	return (0);
}
