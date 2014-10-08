#include "com_defs.h"
#include "game/scenario.h"
#include "gui/gui.h"
#include "hud/hud.h"
#include "includes.h"
#include "input/input.h"
#include "land/land.h"
#include "land/land_dlg.h"
#include "navigation/navigation.h"
#include "sprite/planet.h"
#include "sprite/sprite.h"
#include "sprite/model.h"
#include "sprite/weapon.h"
#include "system/font.h"
#include "system/path.h"
#include "system/video/video.h"
#include "system/video/zoom.h"

/* ui variables */
static gui_session *session = NULL;
static gui_btab *navbios = NULL;
/* shipyard variables */
static gui_frame *ship_selector_frame, *ship_desc_frame, *ship_view_frame, *ship_stats_frame;
static gui_tab *ship_selector_tab, *ship_stats_tab;
static gui_listbox *light_listbox, *medium_listbox, *heavy_listbox, *capitol_listbox, *cargo_listbox;
static gui_textbox *ship_desc;
static gui_button *buy_btn;
static model_t *light_ships[MAX_MODELS];
static int num_light_ships;
static model_t *medium_ships[MAX_MODELS];
static int num_medium_ships;
static model_t *heavy_ships[MAX_MODELS];
static int num_heavy_ships;
static model_t *capitol_ships[MAX_MODELS];
static int num_capitol_ships;
static model_t *cargo_ships[MAX_MODELS];
static int num_cargo_ships;
/* outfit variables */
static gui_frame *outfit_item_selector_frame, *outfit_desc_frame, *outfit_view_frame;
static gui_listbox *arms_listbox, *engine_listbox, *hull_listbox, *misc_listbox;
static gui_tab *outfit_item_selector_tab = NULL;
static gui_textbox *outfit_desc;
static gui_button *outfit_buy_btn;
static void outfit_view_drawing(int x, int y, int w, int h);
static void outfit_item_selector_tab_cb(int which);
static void outfit_desc_cb(int x, int y, int w, int h);
static outfit_item *outfit_arms[MAX_OUTFITS], *outfit_hulls[MAX_OUTFITS], *outfit_misc[MAX_OUTFITS], *outfit_engine[MAX_OUTFITS];
static int num_outfit_arms, num_outfit_hulls, num_outfit_misc, num_outfit_engine;
/* outfit functions */
int outfit_get_selected(void);
void outfit_buy_btn_cb(void);
/* summary variables */
static gui_image *summary_surface;
static gui_label *summary_planet_name_lbl;
static gui_textbox *summary_desc;
static gui_button *summary_refuel;
static void summary_refuel_cb(void);
/* misc. variables */
static gui_label *not_available;
/* ui callbacks */
static void tmp_btn_cb(void);
static void bt_cb(int which);
static void ships_lb_cb(void *lb, int x, int y, int w, int h, int which);
static void outfit_lb_cb(void *lb, int x, int y, int w, int h, int which);
static void ship_desc_cb(int x, int y, int w, int h);
static void selector_tab_cb(int which);
static void ship_view_drawing(int x, int y, int w, int h);
static void ship_stats_drawing(int x, int y, int w, int h);
static void ftab_cb(int which);
static void buy_btn_cb(void);
/* creation functions */
static void create_shipyard_tab(struct _planet *planet);
static void create_outfit_tab(struct _planet *planet);
static void create_summary_tab(struct _planet *planet);
/* misc. */
static void set_new_ship_desc(model_t *model);
static model_t *shipyard_get_selected_ship(void);

void do_landing_dialog(struct _planet *planet) {
	gui_window *main_wnd;
	gui_button *tmp_btn;
	char temp[40] = {0};

	assert(planet);

	/* reset values */
	num_light_ships = 0;
	num_medium_ships = 0;
	num_heavy_ships = 0;
	num_capitol_ships = 0;
	num_cargo_ships = 0;

	session = gui_create_session();

	/* create the main window */
	main_wnd = gui_create_window(87, 82, 627, 436, session);

	/* temp button used for quitting */
	sprintf(temp, "Leave %s", planet->name);
	tmp_btn = gui_create_button(613, 517, 100, 19, temp, session);
	gui_button_set_callback(tmp_btn, tmp_btn_cb);

	/* create the main btab*/
	navbios = gui_create_btab(98, 91, session);
	gui_btab_set_callback(navbios, bt_cb);
	gui_btab_add_tab_eaf(epiar_eaf, navbios, "land/0.png", NULL);
	gui_btab_add_tab_eaf(epiar_eaf, navbios, "land/1_off.png", "land/1_on.png");
	gui_btab_add_tab_eaf(epiar_eaf, navbios, "land/2_off.png", "land/2_on.png");
	gui_btab_add_tab_eaf(epiar_eaf, navbios, "land/3_off.png", "land/3_on.png");
	gui_btab_add_tab_eaf(epiar_eaf, navbios, "land/4_off.png", "land/4_on.png");
	gui_btab_add_tab_eaf(epiar_eaf, navbios, "land/5_off.png", "land/5_on.png");

	create_summary_tab(planet);
	create_shipyard_tab(planet);
	create_outfit_tab(planet);

	not_available = gui_create_label(335, 290, "Feature not available yet", session);
	not_available->visible = 0;

	gui_session_show_all(session);

	gui_main(session);

	gui_session_destroy_all(session);
	gui_destroy_session(session);

	session = NULL;
}

static void tmp_btn_cb(void) {
	session->active = 0;
}

/* navbios callback */
static void bt_cb(int which) {
	/*  set summary parameters */
	if (which == 1) {
		summary_surface->visible = 1;
		summary_planet_name_lbl->visible = 1;
		summary_desc->visible = 1;
		summary_desc->sb->visible = 1;
		if (player.ship->fuel < player.ship->model->total_fuel)
			summary_refuel->visible = 1;
	} else {
		summary_surface->visible = 0;
		summary_planet_name_lbl->visible = 0;
		summary_desc->visible = 0;
		summary_desc->sb->visible = 0;
		summary_refuel->visible = 0;
	}
	/* set outfit parameters */
	if (which == 2) {
		/* show it */
		outfit_item_selector_frame->visible = 1;
		outfit_desc_frame->visible = 1;
		outfit_view_frame->visible = 1;
		outfit_item_selector_tab->visible = 1;
		if (outfit_item_selector_tab->selected == 0) {
			arms_listbox->visible = 1;
			arms_listbox->sb->visible = 1;
		} else if (outfit_item_selector_tab->selected == 1) {
			engine_listbox->visible = 1;
			engine_listbox->sb->visible = 1;
		} else if (outfit_item_selector_tab->selected == 3) {
			hull_listbox->visible = 1;
			hull_listbox->sb->visible = 1;
		} else if (outfit_item_selector_tab->selected == 4) {
			misc_listbox->visible = 1;
			misc_listbox->sb->visible = 1;
		}
		outfit_desc->visible = 1;
		outfit_desc->sb->visible = 1;
		/* set this to default (may be set again before seen) */
		gui_textbox_change_text(outfit_desc, "Select an item to view it's description.");
		outfit_buy_btn->visible = 1;
	} else {
		/* hide it */
		outfit_item_selector_frame->visible = 0;
		outfit_desc_frame->visible = 0;
		outfit_view_frame->visible = 0;
		outfit_item_selector_tab->visible = 0;
		arms_listbox->visible = 0;
		arms_listbox->sb->visible = 0;
		engine_listbox->visible = 0;
		engine_listbox->sb->visible = 0;
		hull_listbox->visible = 0;
		hull_listbox->sb->visible = 0;
		misc_listbox->visible = 0;
		misc_listbox->sb->visible = 0;
		outfit_desc->visible = 0;
		outfit_desc->sb->visible = 0;
		outfit_buy_btn->visible = 0;
	}
	/* set shipyard parameters */
	if (which == 3) {
		ship_selector_frame->visible = 1;
		ship_selector_tab->visible = 1;
		ship_desc_frame->visible = 1;
		ship_view_frame->visible = 1;
		ship_stats_frame->visible = 1;
		ship_stats_tab->visible = 1;
		ship_desc->visible = 1;
		ship_desc->sb->visible = 1;
		/* set the correct listboxes on/off */
		selector_tab_cb(ship_selector_tab->selected);
	} else {
		ship_selector_frame->visible = 0;
		ship_selector_tab->visible = 0;
		ship_desc_frame->visible = 0;
		ship_view_frame->visible = 0;
		ship_stats_frame->visible = 0;
		ship_stats_tab->visible = 0;
		light_listbox->visible = 0;
		light_listbox->sb->visible = 0;
		medium_listbox->visible = 0;
		medium_listbox->sb->visible = 0;
		heavy_listbox->visible = 0;
		heavy_listbox->sb->visible = 0;
		capitol_listbox->visible = 0;
		capitol_listbox->sb->visible = 0;
		cargo_listbox->visible = 0;
		cargo_listbox->sb->visible = 0;
		ship_desc->visible = 0;
		ship_desc->sb->visible = 0;
		buy_btn->visible = 0;
	}
	if ((which == 4) || (which == 5)) {
		not_available->visible = 1;
	} else {
		not_available->visible = 0;
	}
	
	gui_session_show_all(session);
}

/* creates the summary tab and sets it initially visible */
static void create_summary_tab(struct _planet *planet) {
	afont *temp_font = NULL;
	char *desc = NULL;
	char temp[20] = {0};
	
	/* first try and load image from scenario eaf, if that fails, try the main eaf (main.eaf) */
	summary_surface = gui_create_image_eaf(loaded_eaf, 110, 172, planet->surface, session);
	if (!summary_surface) {
		summary_surface = gui_create_image_eaf(main_eaf, 110, 172, planet->surface, session);
		if (!summary_surface) {
			printf("Could not load surface summary image. File \"%s\" not found in scenario EAF or main eaf.\n", planet->surface);
			assert(0);
		}
	}
	summary_surface->visible = 1;
	
	temp_font = epiar_load_font_eaf("fonts/VeraBd-23.af", epiar_eaf);
	assert(temp_font);
	
	desc = get_planet_description(planet->name);
	assert(desc);
	
	/* create the summary planet name */
	summary_planet_name_lbl = gui_create_label_from_font(108, 130, planet->name, temp_font, session);
	
	epiar_free(temp_font);
	
	/* create the description textbox */
	summary_desc = gui_create_textbox(502, 143, 195, 350, desc, session);
	
	free(desc);
	
	/* create the refuel button */
	sprintf(temp, "Refuel (%d credits)", (int)((player.ship->model->total_fuel - player.ship->fuel) * 0.5));
	summary_refuel = gui_create_button(205, 468, 210, 25, temp, session);
	gui_button_set_callback(summary_refuel, summary_refuel_cb);
	
	if ((int)((player.ship->model->total_fuel - player.ship->fuel) * 0.5) == 0)
		summary_refuel->visible = 0; /* dont show the refuel button if they didnt waste any fuel between visits */
}

/* creates the shipyard tab and keep it initially invisible */
static void create_shipyard_tab(struct _planet *planet) {
	int i;
	
	assert(planet);

	ship_selector_frame = gui_create_frame(128, 141, 195, 355, session);
	ship_selector_frame->visible = 0;

	ship_selector_tab = gui_create_tab(108, 141, "Light\nMedium\nHeavy\nCargo\nCapitol\nHold\n", 0, session);
	ship_selector_tab->visible = 0;
	gui_tab_set_callback(ship_selector_tab, selector_tab_cb);

	ship_desc_frame = gui_create_frame(342, 365, 345, 130, session);
	ship_desc_frame->visible = 0;
	gui_frame_associate_drawing(ship_desc_frame, ship_desc_cb);

	ship_view_frame = gui_create_frame(342, 141, 210, 220, session);
	ship_view_frame->visible = 0;
	gui_frame_associate_drawing(ship_view_frame, ship_view_drawing);

	ship_stats_frame = gui_create_frame(557, 141, 130, 220, session);
	ship_stats_frame->visible = 0;
	gui_frame_associate_drawing(ship_stats_frame, ship_stats_drawing);

	ship_stats_tab = gui_create_tab(663, 147, "Stats\nEngines\nWeapons\nUtilities\n", 1, session);
	ship_stats_tab->visible = 0;
	gui_tab_set_callback(ship_stats_tab, ftab_cb);

	/* goes on top of the selector frame */
	/* create the light listbox */
	light_listbox = gui_create_listbox(138, 151, 175, 335, 70, session);
	light_listbox->visible = 0;
	light_listbox->sb->visible = 0;
	gui_listbox_set_callback(light_listbox, ships_lb_cb);

	/* run through the planet list and add any light ship types */
	for (i = 0; i < MAX_MODELS; i++) {
		if (planet_features.ships_available[i] == NULL)
			break;
		if (planet_features.ships_available[i]->class == MODEL_LIGHT) {
			gui_listbox_add_item(light_listbox);
			light_ships[num_light_ships] = planet_features.ships_available[i];
			num_light_ships++;
		}
	}

	/* create the medium listbox */
	medium_listbox = gui_create_listbox(138, 151, 175, 335, 70, session);
	medium_listbox->visible = 0;
	medium_listbox->sb->visible = 0;
	gui_listbox_set_callback(medium_listbox, ships_lb_cb);

	/* run through the planet list and add any medium ship types */
	for (i = 0; i < MAX_MODELS; i++) {
		if (planet_features.ships_available[i] == NULL)
			break;
		if (planet_features.ships_available[i]->class == MODEL_MEDIUM) {
			gui_listbox_add_item(medium_listbox);
			medium_ships[num_medium_ships] = planet_features.ships_available[i];
			num_medium_ships++;
		}
	}

	/* create the heavy listbox */
	heavy_listbox = gui_create_listbox(138, 151, 175, 335, 70, session);
	heavy_listbox->visible = 0;
	heavy_listbox->sb->visible = 0;
	gui_listbox_set_callback(heavy_listbox, ships_lb_cb);

	/* run through the planet list and add any heavy ship types */
	for (i = 0; i < MAX_MODELS; i++) {
		if (planet_features.ships_available[i] == NULL)
			break;
		if (planet_features.ships_available[i]->class == MODEL_HEAVY) {
			gui_listbox_add_item(heavy_listbox);
			heavy_ships[num_heavy_ships] = planet_features.ships_available[i];
			num_heavy_ships++;
		}
	}

	/* create the capitol listbox */
	capitol_listbox = gui_create_listbox(138, 151, 175, 335, 70, session);
	capitol_listbox->visible = 0;
	capitol_listbox->sb->visible = 0;
	gui_listbox_set_callback(capitol_listbox, ships_lb_cb);

	/* run through the planet list and add any capitol ship types */
	for (i = 0; i < MAX_MODELS; i++) {
		if (planet_features.ships_available[i] == NULL)
			break;
		if (planet_features.ships_available[i]->class == MODEL_CAPITOL) {
			gui_listbox_add_item(capitol_listbox);
			capitol_ships[num_capitol_ships] = planet_features.ships_available[i];
			num_capitol_ships++;
		}
	}

	/* create the cargo listbox */
	cargo_listbox = gui_create_listbox(138, 151, 175, 335, 70, session);
	cargo_listbox->visible = 0;
	cargo_listbox->sb->visible = 0;
	gui_listbox_set_callback(cargo_listbox, ships_lb_cb);

	/* run through the planet list and add any cargo ship types */
	for (i = 0; i < MAX_MODELS; i++) {
		if (planet_features.ships_available[i] == NULL)
			break;
		if (planet_features.ships_available[i]->class == MODEL_CARGO) {
			gui_listbox_add_item(cargo_listbox);
			cargo_ships[num_cargo_ships] = planet_features.ships_available[i];
			num_cargo_ships++;
		}
	}

	ship_desc = gui_create_textbox(352, 375, 325, 110, "Select a ship to view it's description.", session);
	ship_desc->visible = 0;
	ship_desc->sb->visible = 0;

	buy_btn = gui_create_button(355, 325, 105, 25, "Buy", session);
	buy_btn->visible = 0;
	gui_button_set_callback(buy_btn, buy_btn_cb);
}

/* creates the outfit tab and keep it initially invisible */
static void create_outfit_tab(struct _planet *planet) {
	int i;
	
	assert(planet);
	
	outfit_item_selector_frame = gui_create_frame(128, 141, 195, 285, session);
	outfit_item_selector_frame->visible = 0;
	
	outfit_item_selector_tab = gui_create_tab(108, 141, "Arms\nEngines\nShields\nHull\nMisc\n", 0, session);
	outfit_item_selector_tab->visible = 0;
	gui_tab_set_callback(outfit_item_selector_tab, outfit_item_selector_tab_cb);
	
	outfit_desc_frame = gui_create_frame(342, 365, 345, 130, session);
	outfit_desc_frame->visible = 0;
	gui_frame_associate_drawing(outfit_desc_frame, outfit_desc_cb);
	
	outfit_view_frame = gui_create_frame(342, 141, 345, 220, session);
	outfit_view_frame->visible = 0;
	gui_frame_associate_drawing(outfit_view_frame, outfit_view_drawing);
	
	/* make the various listboxes that appear in the leftmost frame */
	arms_listbox = gui_create_listbox(138, 151, 175, 265, 70, session);
	arms_listbox->visible = 0;
	arms_listbox->sb->visible = 0;
	gui_listbox_set_callback(arms_listbox, outfit_lb_cb);
	
	num_outfit_arms = 0;
	
	engine_listbox = gui_create_listbox(138, 151, 175, 265, 70, session);
	engine_listbox->visible = 0;
	engine_listbox->sb->visible = 0;
	gui_listbox_set_callback(engine_listbox, outfit_lb_cb);
	
	num_outfit_engine = 0;
	
	hull_listbox = gui_create_listbox(138, 151, 175, 265, 70, session);
	hull_listbox->visible = 0;
	hull_listbox->sb->visible = 0;
	gui_listbox_set_callback(hull_listbox, outfit_lb_cb);
	
	num_outfit_hulls = 0;
	
	misc_listbox = gui_create_listbox(138, 151, 175, 265, 70, session);
	misc_listbox->visible = 0;
	misc_listbox->sb->visible = 0;
	gui_listbox_set_callback(misc_listbox, outfit_lb_cb);
	
	num_outfit_misc = 0;
	
	/* create the lists used to draw listboxes */
	for (i = 0; i < planet_features.num_outfits; i++) {
		assert(planet_features.outfits_available[i]);
		if ((planet_features.outfits_available[i]->specific == WEAPON) || (planet_features.outfits_available[i]->specific == AMMO)) {
			gui_listbox_add_item(arms_listbox);
			outfit_arms[num_outfit_arms] = planet_features.outfits_available[i];
			num_outfit_arms++;
		} else if (planet_features.outfits_available[i]->specific == ENGINE) {
			gui_listbox_add_item(engine_listbox);
			outfit_engine[num_outfit_engine] = planet_features.outfits_available[i];
			num_outfit_engine++;
		} else if (planet_features.outfits_available[i]->specific == HULL) {
			gui_listbox_add_item(hull_listbox);
			outfit_hulls[num_outfit_hulls] = planet_features.outfits_available[i];
			num_outfit_hulls++;
		} else if (planet_features.outfits_available[i]->specific == DEATH) {
			/* add to misc. */
			gui_listbox_add_item(misc_listbox);
			outfit_misc[num_outfit_misc] = planet_features.outfits_available[i];
			num_outfit_misc++;
		} else if (planet_features.outfits_available[i]->specific == MAP) {
			/* add to misc. */
			gui_listbox_add_item(misc_listbox);
			outfit_misc[num_outfit_misc] = planet_features.outfits_available[i];
			num_outfit_misc++;
		} else if (planet_features.outfits_available[i]->specific == HUD) {
			/* add to misc. */
			gui_listbox_add_item(misc_listbox);
			outfit_misc[num_outfit_misc] = planet_features.outfits_available[i];
			num_outfit_misc++;
		}
	}
	
	/* make the outfit description textbox */
	outfit_desc = gui_create_textbox(352, 375, 325, 110, "Select an item to view it's description.", session);
	outfit_desc->visible = 0;
	outfit_desc->sb->visible = 0;
	
	/* make the buy button */
	outfit_buy_btn = gui_create_button(128, 435, 195, 25, "Buy", session);
	outfit_buy_btn->visible = 0;
	gui_button_set_callback(outfit_buy_btn, outfit_buy_btn_cb);
}

static void ships_lb_cb(void *lb, int x, int y, int w, int h, int which) {
	model_t *selected = NULL;
	char *name = NULL;
	SDL_Surface *ship_image = NULL;
	SDL_Rect dest;
	int text_w, text_h, base;
	unsigned char zoomed = 0; /* used to know if we need to free a zoomed surface or not */
	
	if (which < 0)
		return;
	
	if ((gui_listbox *)lb == light_listbox) {
		ship_image = light_ships[which]->image;
		
		if (light_listbox->selected == which)
			selected = light_ships[which];
		
		name = light_ships[which]->name;
	} else if ((gui_listbox *)lb == medium_listbox) {
		ship_image = medium_ships[which]->image;
		
		if (medium_listbox->selected == which)
			selected = medium_ships[which];
		
		name = medium_ships[which]->name;
	} else if ((gui_listbox *)lb == heavy_listbox) {
		ship_image = heavy_ships[which]->image;
		
		if (heavy_listbox->selected == which)
			selected = heavy_ships[which];
		
		name = heavy_ships[which]->name;
	} else if ((gui_listbox *)lb == capitol_listbox) {
		ship_image = capitol_ships[which]->image;
		
		if (capitol_listbox->selected == which)
			selected = capitol_ships[which];
		
		name = capitol_ships[which]->name;
	} else if ((gui_listbox *)lb == cargo_listbox) {
		ship_image = cargo_ships[which]->image;
		
		if (cargo_listbox->selected == which)
			selected = cargo_ships[which];
		
		name = cargo_ships[which]->name;
	}
	
	if ((ship_image->h > 65) || (ship_image->w > 90)) {
		/* ship image is too big, we need to zoom */
		float zoom_x, zoom_y;
		
		zoomed = 1; /* flag used to know we need to free the temporary, zoomed surface we're about to create */
		
		zoom_x = 90.0f / (float)ship_image->w;
		zoom_y = 65.0f / (float)ship_image->h;
		
		/* maintain 1:1 aspect ratio */
		if (zoom_x < zoom_y)
			zoom_y = zoom_x;
		else
			zoom_x = zoom_y;
		
		ship_image = zoomSurface(ship_image, zoom_x, zoom_y, 1);
		assert(ship_image);
		SDL_SetColorKey(ship_image, SDL_SRCCOLORKEY | SDL_RLEACCEL, (Uint32)SDL_MapRGB(ship_image->format, 0, 0, 0));
	}
	
	/* do the actual drawing */
	dest.x = x + (w / 4) - (ship_image->w / 2);
	if (dest.x < 140)
		dest.x = 140;
	dest.y = y + (h / 2) - (ship_image->h / 2);
	dest.w = ship_image->w;
	dest.h = ship_image->h;
	
	assert(ship_image);
	
	blit_surface(ship_image, NULL, &dest, 0);
	
	if (zoomed)
		SDL_FreeSurface(ship_image); /* we zoomed so this became a temp. surface used only for this drawing, so free it */
	
	epiar_size_text(gui_font_normal, name, &text_w, &text_h, &base);
	epiar_render_text(gui_font_normal, white, 0, AFONT_RENDER_BLENDED, screen, x + w - text_w - 5, y + 7 + base, name);
	
	/* force redraw of the other three frames and the flat tab (as they're all subject to change) */
	ship_stats_frame->update = 1;
	ship_desc_frame->update = 1;
	ship_view_frame->update = 1;
	ship_stats_tab->update = 1;
	
	/* also, since they selected a new ship, change the description in the textbox */
	if (selected != NULL)
		set_new_ship_desc(selected);
}

static void selector_tab_cb(int which) {
	/* set the light listbox attribs */
	if (which == 0) {
		light_listbox->visible = 1;
		light_listbox->sb->visible = 1;
		light_listbox->update = 1; /* set update to 1 since it needs to be drawn */
		light_listbox->sb->update = 1;
	} else {
		light_listbox->visible = 0;
		light_listbox->sb->visible = 0;
	}

	/* set the medium listbox attribs */
	if (which == 1) {
		medium_listbox->visible = 1;
		medium_listbox->sb->visible = 1;
		medium_listbox->update = 1; /* set update to 1 since it needs to be drawn */
		medium_listbox->sb->update = 1;
	} else {
		medium_listbox->visible = 0;
		medium_listbox->sb->visible = 0;
	}

	/* set the heavy listbox attribs */
	if (which == 2) {
		heavy_listbox->visible = 1;
		heavy_listbox->sb->visible = 1;
		heavy_listbox->update = 1; /* set update to 1 since it needs to be drawn */
		heavy_listbox->sb->update = 1;
	} else {
		heavy_listbox->visible = 0;
		heavy_listbox->sb->visible = 0;
	}

	/* set the cargo listbox attribs */
	if (which == 3) {
		cargo_listbox->visible = 1;
		cargo_listbox->sb->visible = 1;
		cargo_listbox->update = 1; /* set update to 1 since it needs to be drawn */
		cargo_listbox->sb->update = 1;
	} else {
		cargo_listbox->visible = 0;
		cargo_listbox->sb->visible = 0;
	}

	/* set the capitol listbox attribs */
	if (which == 4) {
		capitol_listbox->visible = 1;
		capitol_listbox->sb->visible = 1;
		capitol_listbox->update = 1; /* set update to 1 since it needs to be drawn */
		capitol_listbox->sb->update = 1;
	} else {
		capitol_listbox->visible = 0;
		capitol_listbox->sb->visible = 0;
	}

	/* force redraw of all four frames and the flat tabs (as they're all subject to change) */
	ship_selector_frame->update = 1;
	ship_stats_frame->update = 1;
	ship_desc_frame->update = 1;
	ship_view_frame->update = 1;
	ship_stats_tab->update = 1;

	/* if there's a selected ship, this'll be changed again */
	gui_textbox_change_text(ship_desc, "Select a ship to view it's description.");
}

static void ship_desc_cb(int x, int y, int w, int h) {
	ship_desc->update = 1;
	ship_desc->sb->update = 1;
}

/* sets the text in the textbox based on what ship type is selected */
static void set_new_ship_desc(model_t *model) {
	char *desc = NULL;

	if (!model)
		return;

	desc = model_get_description(model);
	if (desc) {
		gui_textbox_change_text(ship_desc, desc);
		free(desc);
	}
}

/* called right after ship view frame is drawn, this draws the details (everything but the frame) */
static void ship_view_drawing(int x, int y, int w, int h) {
	model_t *selected = NULL;
	SDL_Rect rect;
	SDL_Surface *wf = NULL;
	char temp[80] = {0};
	int text_w, text_h, base;

	selected = shipyard_get_selected_ship();

	if (selected == NULL)
		return;

	/* get text size to know how much to draw */
	epiar_size_text(gui_font_normal, selected->name, &text_w, &text_h, &base);

	/* draw upper left frame bg and text */
	rect.x = x + 7;
	rect.y = y + 7;
	rect.w = text_w + 3;
	rect.h = 14;
	fill_rect(&rect, map_rgb(0, 93, 92));

	rect.x += rect.w - 1;
	rect.w = 1;

	/* draw the slant */
	for (rect.h = 14; rect.h > 0; rect.h--) {
		rect.x += 1;
		fill_rect(&rect, map_rgb(0, 93, 92));
	}
	/* draw name */
	epiar_render_text(gui_font_normal, white, 0, AFONT_RENDER_BLENDED, screen, x + 9, y + 9 + base, selected->name);

	/* draw price */
	sprintf(temp, "%ld credits", selected->price);
	epiar_size_text(gui_font_normal, temp, &text_w, &text_h, &base);
	epiar_render_text(gui_font_normal, white, 0, AFONT_RENDER_BLENDED, screen, x + 9, y + 22 + base, temp);

	/* draw the ship wireframe */
	wf = load_image_eaf(main_eaf, selected->wireframe, BLACK_COLORKEY);
	if (wf == NULL) {
		wf = load_image_eaf(loaded_eaf, selected->wireframe, BLACK_COLORKEY);
		assert(wf);
	}
	rect.x = x + (w / 2) - (wf->w / 2);
	rect.y = y + (h / 2) - (wf->h / 2);
	rect.w = wf->w;
	rect.h = wf->h;
	blit_surface(wf, NULL, &rect, 0);

	SDL_FreeSurface(wf);

	buy_btn->visible = 1;
	buy_btn->update = 1;
	buy_btn->state |= BTN_REDRAW;
}

static model_t *shipyard_get_selected_ship(void) {

	if (light_listbox->visible) {
		if (light_listbox->selected != -1)
			return (light_ships[light_listbox->selected]);
	} else if (medium_listbox->visible) {
		if (medium_listbox->selected != -1)
			return (medium_ships[medium_listbox->selected]);
	} else if (heavy_listbox->visible) {
		if (heavy_listbox->selected != -1)
			return (heavy_ships[heavy_listbox->selected]);
	} else if (capitol_listbox->visible) {
		if (capitol_listbox->selected != -1)
			return (capitol_ships[capitol_listbox->selected]);
	} else if (cargo_listbox->visible) {
		if (cargo_listbox->selected != -1)
			return (cargo_ships[cargo_listbox->selected]);
	}

	return (NULL);
}

static void ship_stats_drawing(int x, int y, int w, int h) {
	char temp[35] = {0};
	model_t *selected = NULL;
	int ftab = ship_stats_tab->selected;

	selected = shipyard_get_selected_ship();

	if (selected == NULL)
		return;

	if (ftab == 0) {
		/* stats tab */
		epiar_render_text(gui_font_normal, white, 0, AFONT_RENDER_BLENDED, screen, x + 8, y + 16, "Manufacturer:");
		sprintf(temp, "%s", model_get_manufacturer(selected));
		epiar_render_text(gui_font_normal, white, 0, AFONT_RENDER_BLENDED, screen, x + 8, y + 28, temp);
		sprintf(temp, "Cargo Space: %d", selected->cargo);
		epiar_render_text(gui_font_normal, white, 0, AFONT_RENDER_BLENDED, screen, x + 8, y + 68, temp);
		epiar_render_text(gui_font_normal, white, 0, AFONT_RENDER_BLENDED, screen, x + 8, y + 84, "Seconds per rotation:");
		sprintf(temp, "%d second(s)", selected->str);
		epiar_render_text(gui_font_normal, white, 0, AFONT_RENDER_BLENDED, screen, x + 8, y + 94, temp);
		sprintf(temp, "Hull Strength: %d", selected->hull_life);
		epiar_render_text(gui_font_normal, white, 0, AFONT_RENDER_BLENDED, screen, x + 8, y + 136, temp);
	} else if (ftab == 1) {
		/* engines tab */
		epiar_render_text(gui_font_normal, white, 0, AFONT_RENDER_BLENDED, screen, x + 8, y + 16, "Engine Creator:");
		sprintf(temp, "%s", (char *)selected->default_engine->name);
		epiar_render_text(gui_font_normal, white, 0, AFONT_RENDER_BLENDED, screen, x + 8, y + 28, temp);
		sprintf(temp, "Acceleration: %d", selected->default_engine->acceleration);
		epiar_render_text(gui_font_normal, white, 0, AFONT_RENDER_BLENDED, screen, x + 8, y + 52, temp);
		sprintf(temp, "Top Speed: %d", selected->default_engine->top_speed);
		epiar_render_text(gui_font_normal, white, 0, AFONT_RENDER_BLENDED, screen, x + 8, y + 68, temp);
		if (selected->default_engine->jump)
			sprintf(temp, "Jump Capable: Yes");
		else
			sprintf(temp, "Jump Capable: No");
		epiar_render_text(gui_font_normal, white, 0, AFONT_RENDER_BLENDED, screen, x + 8, y + 92, temp);
	} else if (ftab == 2) {
		/* weapons tab */
		int cur_y = y + 16, i;

		/* loop through the slots until we either have drawn too much or found the end */
		for (i = 0; (i < MAX_WEAPON_SLOTS) && (cur_y < (y + h)); i++) {
			if (selected->default_mounts[i]) {
				sprintf(temp, "%s\n", selected->default_mounts[i]->weapon->name);
				epiar_render_text(gui_font_normal, white, 0, AFONT_RENDER_BLENDED, screen, x + 8, cur_y, temp);
				cur_y += 12;
				sprintf(temp, "Recharge: %d", selected->default_mounts[i]->weapon->recharge);
				epiar_render_text(gui_font_normal, white, 0, AFONT_RENDER_BLENDED, screen, x + 8, cur_y, temp);
				cur_y += 12;
				sprintf(temp, "Strength: %d", selected->default_mounts[i]->weapon->strength);
				epiar_render_text(gui_font_normal, white, 0, AFONT_RENDER_BLENDED, screen, x + 8, cur_y, temp);
				cur_y += 12;
				sprintf(temp, "Lifetime (ms): %d", selected->default_mounts[i]->weapon->lifetime);
				epiar_render_text(gui_font_normal, white, 0, AFONT_RENDER_BLENDED, screen, x + 8, cur_y, temp);
				cur_y += 24;
			} else {
				break;
			}
		}
	} else if (ftab == 3) {
		/* utils (shields) tab */

		sprintf(temp, "%s\n", selected->default_shield->name);
		epiar_render_text(gui_font_normal, white, 0, AFONT_RENDER_BLENDED, screen, x + 8, y + 16, temp);
		sprintf(temp, "Strength: %d", selected->default_shield->strength);
		epiar_render_text(gui_font_normal, white, 0, AFONT_RENDER_BLENDED, screen, x + 8, y + 28, temp);
	}
}

static void ftab_cb(int which) {
	ship_stats_frame->update = 1;
}

static void buy_btn_cb(void) {
	model_t *selected = NULL;
	int i;

	selected = shipyard_get_selected_ship();
	if (selected == NULL)
		return;

	if (gui_question("Really purchase ship?")) {
		if (player.credits >= selected->price) {
			player.credits -= selected->price;
			erase_hud(1);
			draw_hud(1); /* update credits display */
			player.ship->model = selected;
			player.ship->shield_life = player.ship->model->default_shield->strength;
			player.ship->hull_strength = player.ship->model->hull_life;
			player.ship->shield = player.ship->model->default_shield;
			player.ship->engine = player.ship->model->default_engine;
			player.ship->max_velocity = (float)player.ship->engine->top_speed / 2000.0;
			player.ship->fuel = player.ship->model->total_fuel;
			player.free_mass = player.ship->model->cargo;
			for (i = 0; i < MAX_WEAPON_SLOTS; i++) {
				if (player.ship->model->default_mounts[i]) {
					int j;
					/* find a free slot and equip it there */
					for (j = 0; j < MAX_WEAPON_SLOTS; j++) {
						if (!player.ship->weapon_mount[j]) {
							/* free slot */
							player.ship->weapon_mount[j] = copy_weapon_mount(player.ship->model->default_mounts[i]);
							if (player.ship->w_slot[0] == -1)
								player.ship->w_slot[0] = j;
							else if (player.ship->w_slot[1] == -1)
								player.ship->w_slot[1] = j;
							break;
						}
					}
				}
			}
		} else {
			gui_alert("Not enough credits!");
		}
	}

	gui_session_show_all(session);
}

/* draws the main view window in the outfit tab */
static void outfit_view_drawing(int x, int y, int w, int h) {
	int selected = outfit_get_selected();
	int tw, text_h, base;
	SDL_Surface *temp;
	SDL_Rect rect;
	char text[120] = {0};
	outfit_item *item = NULL;

	if (selected == -1)
		return;

	if (arms_listbox->visible) {
		if (arms_listbox->selected > (arms_listbox->num_items - 1))
			return;
		item = outfit_arms[arms_listbox->selected];
	} else if (engine_listbox->visible) {
		if (engine_listbox->selected > (engine_listbox->num_items - 1))
			return;
		item = outfit_engine[engine_listbox->selected];
	} else if (hull_listbox->visible) {
		if (hull_listbox->selected > (hull_listbox->num_items - 1))
			return;
		item = outfit_hulls[hull_listbox->selected];
	} else if (misc_listbox->visible) {
		if (misc_listbox->selected > (misc_listbox->num_items - 1))
			return;
		item = outfit_misc[misc_listbox->selected];
	}
	assert(item);
	
	afont_size_text(gui_font_normal, item->name, &tw, &text_h, &base);
	
	/* draw upper left frame bg and text */
	rect.x = x + 7;
	rect.y = y + 7;
	rect.w = tw + 3;
	rect.h = 14;
	fill_rect(&rect, map_rgb(0, 93, 92));

	rect.x += rect.w - 1;
	rect.w = 1;

	/* draw the slant */
	for (rect.h = 14; rect.h > 0; rect.h--) {
		rect.x += 1;
		fill_rect(&rect, map_rgb(0, 93, 92));
	}

	/* draw the item's name in the upper left */
	epiar_render_text(gui_font_normal, white, 0, AFONT_RENDER_BLENDED, screen, x + 9, y + 9 + base, item->name);

	/* draw the item */
	temp = load_image_eaf(loaded_eaf, item->stock, ALPHA);
	if (!temp) {
		temp = load_image_eaf(main_eaf, item->stock, ALPHA);
		if (!temp) {
			printf("Couldn't load \"%s\"\n", item->stock);
			return;
		}
	}

	blit_image(temp, x + (w / 2) - (temp->w / 2), y + (h / 2) - (temp->h / 2));

	SDL_FreeSurface(temp);

	/* draw the price information in the lower right of the window */
	memset(text, 0, sizeof(text));
	sprintf(text, "Credits Required: %d", item->price);
	epiar_size_text(gui_font_normal, text, &tw, &text_h, &base);
	epiar_render_text(gui_font_normal, white, 0, AFONT_RENDER_BLENDED, screen, x + w - tw - 12, y + h - 35, text);
	memset(text, 0, sizeof(text));
	sprintf(text, "Credits Available: %d", player.credits);
	epiar_size_text(gui_font_normal, text, &tw, &text_h, &base);
	epiar_render_text(gui_font_normal, white, 0, AFONT_RENDER_BLENDED, screen, x + w - tw - 12, y + h - 25, text);
	memset(text, 0, sizeof(text));
	sprintf(text, "Projected Balance: %d", player.credits - item->price);
	epiar_size_text(gui_font_normal, text, &tw, &text_h, &base);
	epiar_render_text(gui_font_normal, white, 0, AFONT_RENDER_BLENDED, screen, x + w - tw - 12, y + h - 15, text);

	/* NOTE: following four fill_rect()s are commented out because they're ugly */

	assert(player.ship);
	assert(player.ship->model);
	assert(item);

	/* draw free mass and projected mass */
	/* draw the frame representing all available mass */
	rect.x = x + w - 210;
	rect.y = y + 10;
	rect.w = 200;
	rect.h = 8;
	/* fill_rect(&rect, dark_grey); */
	rect.x += 1;
	rect.y += 1;
	rect.w -= 2;
	rect.h -= 2;
	/* fill_rect(&rect, black); */
	/* now that we drew that, draw the free mass */
	/* ensure the cargo space is not zero */
	if(player.ship->model->cargo)
		rect.w = (198 * player.free_mass) / player.ship->model->cargo;
	else
		rect.w = 0;
	/* fill_rect(&rect, grey); */
	/* now show how much mass the selected outfit will take */
	rect.x += rect.w;
	/* ensure the cargo space is not zero */
	if(player.ship->model->cargo)
		rect.w = (item->needed_free_mass * 198) / player.ship->model->cargo;
	else
		rect.w = 0;
	rect.x -= rect.w;
	if (rect.x < (x + w - 210))
		rect.x = x + w - 210;
	/* fill_rect(&rect, red); */

	gui_textbox_change_text(outfit_desc, item->desc);
	outfit_desc->update = 1;
}

/* outfit tab - called whenever a new item is selected on the left */
static void outfit_item_selector_tab_cb(int which) {
	if (which == 0) {
		arms_listbox->visible = 1;
		arms_listbox->sb->visible = 1;
	} else {
		arms_listbox->visible = 0;
		arms_listbox->sb->visible = 0;
	}
	if (which == 1) {
		engine_listbox->visible = 1;
		engine_listbox->sb->visible = 1;
	} else {
		engine_listbox->visible = 0;
		engine_listbox->sb->visible = 0;
	}
	if (which == 3) {
		hull_listbox->visible = 1;
		hull_listbox->sb->visible = 1;
	} else {
		hull_listbox->visible = 0;
		hull_listbox->sb->visible = 0;
	}
	if (which == 4) {
		misc_listbox->visible = 1;
		misc_listbox->sb->visible = 1;
	} else {
		misc_listbox->visible = 0;
		misc_listbox->sb->visible = 0;
	}
	
	gui_textbox_change_text(outfit_desc, "Select an item to view it's description.");
	outfit_desc->update = 1;
	
	outfit_item_selector_frame->update = 1;
	arms_listbox->update = 1;
	arms_listbox->sb->update = 1;
	engine_listbox->update = 1;
	engine_listbox->sb->update = 1;
	hull_listbox->update = 1;
	hull_listbox->sb->update = 1;
	misc_listbox->update = 1;
	misc_listbox->sb->update = 1;
	outfit_view_frame->update = 1;
}

/* called whenever the description frame for outfit is outdated */
static void outfit_desc_cb(int x, int y, int w, int h) {

}

/* callback when something occurs in any listbox on the outfit tab */
static void outfit_lb_cb(void *lb, int x, int y, int w, int h, int which) {
	gui_listbox *lbox = (gui_listbox *)lb;
	outfit_item *item = NULL;
	SDL_Surface *temp;
	char file[120] = {0};
	int cur_y, text_w, text_h, base;
	char text[120] = {0}; /* array used for resizing when text is too long */
	
	if (arms_listbox->visible) {
		if (num_outfit_arms == 0)
			return;
		item = outfit_arms[which];
	} else if (engine_listbox->visible) {
		if (num_outfit_engine == 0)
			return;
		item = outfit_engine[which];
	} else if (hull_listbox->visible) {
		if (num_outfit_hulls == 0)
			return;
		item = outfit_hulls[which];
	} else if (misc_listbox->visible) {
		if (num_outfit_misc == 0)
			return;
		item = outfit_misc[which];
	}
	assert(item);
	
	/* draw the generic information */
	/* draw the bitmap of the item */
	strncpy(file, item->stock, 113);
	sprintf(&file[strlen(file) - 4], "_s.png");
	
	temp = load_image_eaf(loaded_eaf, file, ALPHA);
	if (!temp) {
		temp = load_image_eaf(main_eaf, file, ALPHA);
		if (!temp) {
			printf("Couldn't load \"%s\"\n", file);
			return;
		}
	}
	
	blit_image(temp, x + 3, y + (h / 2) - (temp->h / 2));
	SDL_FreeSurface(temp);
	
	/* write the item's info */
	/* render the item's name */
	cur_y = y + 13;
	strncpy(text, item->name, 119);
	epiar_size_text(gui_font_bold, text, &text_w, &text_h, &base);
	while(text_w > 150) {
		text[strlen(text) - 2] = '.';
		text[strlen(text) - 3] = '.';
		text[strlen(text) - 4] = '.';
		text[strlen(text) - 1] = 0;
		epiar_size_text(gui_font_bold, text, &text_w, &text_h, &base);
	}
	epiar_render_text(gui_font_bold, white, 0, AFONT_RENDER_BLENDED, screen, x + w - text_w - 3, cur_y, text);
	cur_y += 10;
	/* render the item's price */
	memset(file, 0, sizeof(file));
	sprintf(file, "%d credits", item->price);
	epiar_size_text(gui_font_normal, file, &text_w, &text_h, &base);
	epiar_render_text(gui_font_normal, white, 0, AFONT_RENDER_BLENDED, screen, x + w - text_w - 3, cur_y, file);
	cur_y += 10;
	/* render the required mass */
	memset(file, 0, sizeof(file));
	sprintf(file, "%d EMU", item->needed_free_mass);
	epiar_size_text(gui_font_normal, file, &text_w, &text_h, &base);
	epiar_render_text(gui_font_normal, white, 0, AFONT_RENDER_BLENDED, screen, x + w - text_w - 3, cur_y, file);
	cur_y += 10;
	
	outfit_view_frame->update = 1;
}

/* returns slot # on success, -1 on failure */
int outfit_get_selected(void) {
	if (arms_listbox->visible)
		return (arms_listbox->selected);
	if (engine_listbox->visible)
		return (engine_listbox->selected);
	if (hull_listbox->visible)
		return (hull_listbox->selected);
	if (misc_listbox->visible)
		return (misc_listbox->selected);
	
	return (-1);
}

/* called whenever the buy button is clicked */
void outfit_buy_btn_cb(void) {
	outfit_item *item = NULL;

	/* figure out which item they're buying */
	if ((hull_listbox->visible) && (hull_listbox->selected != -1))
		item = outfit_hulls[hull_listbox->selected];
	else if ((engine_listbox->visible) && (engine_listbox->selected != -1))
		item = outfit_engine[engine_listbox->selected];
	else if ((arms_listbox->visible) && (arms_listbox->selected != -1))
		item = outfit_arms[arms_listbox->selected];
	else if ((misc_listbox->visible) && (misc_listbox->selected != -1))
		item = outfit_misc[misc_listbox->selected];

	if (!item)
		return; /* they didn't select anything */

	/* ensure they meet the requirements */
	if (player.credits < item->price) {
		gui_alert("Not enough credits!");
		gui_session_show_all(session);
		return;
	}
	if (player.free_mass < item->needed_free_mass) {
		gui_alert("Not enough free mass!");
		gui_session_show_all(session);
		return;
	}

	/* outfit.c has this function */
	if (player_equip_outfit(item) == 0) {
		erase_hud(1);
		player.credits -= item->price;
		draw_hud(1);
	}

	/* update the display */
	gui_session_show_all(session);
}

static void summary_refuel_cb(void) {
	int needed = (player.ship->model->total_fuel - player.ship->fuel) * 0.5;
	
	if (player.credits < needed) {
		gui_alert("Not enough credits!");
	} else {
		player.credits -= needed;
		player.ship->fuel = player.ship->model->total_fuel;
		erase_hud(1);
		draw_hud(1);
		summary_refuel->visible = 0;
	}

	/* redraw session */
	gui_session_show_all(session);
}
