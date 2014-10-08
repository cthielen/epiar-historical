#include "game/game.h"
#include "game/scenario.h"
#include "gui/gui.h"
#include "includes.h"
#include "missions/missions.h"
#include "system/eaf.h"
#include "system/esf.h"
#include "system/font.h"
#include "system/path.h"
#include "system/video/video.h"

#define MAX_SCENARIOS 25

/* following two variables _only_ for custom list use, nothing else */
ep_scenario *scenarios[MAX_SCENARIOS];
int num_scenarios;
FILE *loaded_eaf = NULL;

/* do_scenario_select() variables */
static gui_session *s_session = NULL, *cs_session = NULL;
static ep_scenario *selected_scen = NULL;
static unsigned char custom_select_cancel = 0;

static void main_btn_callback(void);
static void custom_btn_callback(void);
static void custom_ok_callback(void);
static void custom_cancel_callback(void);
static int load_scen_data(ep_scenario *scen);
static int do_custom_scenario_select(void);
static void lb_draw_scenarios(void *lb, int x, int y, int w, int h, int item);
static void scenario_add_to_lb(gui_listbox *lb, char *path);
static void redraw_info_area(void);
static SDL_Surface *custom_draw_area = NULL;
static int custom_scenario_selected = -1;
static gui_textbox *custom_desc_tb = NULL;

/* loads a scenario - pass the path to an eaf file */
ep_scenario *load_scenario_eaf(char *filename) {
	ep_scenario *scen = (ep_scenario *)malloc(sizeof(struct _ep_scenario));

	if (!filename)
		return (NULL);
	if (strlen(filename) <= 0)
		return (NULL);

	scen->eaf = eaf_open_file(filename);
	if (scen->eaf == NULL) {
		printf("Could not load scenario \"%s\".\n", filename);
		free(scen);
		return (NULL);
	}

	/* set all the initial data to defaults (load_scen_data() will hopefully fill this all in) */
	scen->name = NULL;
	scen->author = NULL;
	scen->image = NULL;
	scen->desc = NULL;
	scen->difficulty = AVERAGE;
	scen->website = NULL;
	scen->intro_msg = NULL;

	/* we know the filename already, so fill that in */
	scen->filename = (char *)malloc(sizeof(char) * (strlen(filename) + 1));
	memset(scen->filename, 0, sizeof(char) * (strlen(filename) + 1));
	strcpy(scen->filename, filename);

	if (load_scen_data(scen) != 0) {
		printf("Error while loading scenario data for file \"%s\".\n", filename);
		fclose(scen->eaf);
		free(scen);
		return (NULL);
	}

	loaded_eaf = scen->eaf;

	return (scen);
}

int close_scenario(ep_scenario *scen) {
	assert(scen);

	fclose(scen->eaf);
	scen->eaf = NULL;

	if (scen->name)
		free(scen->name);
	if (scen->author)
		free(scen->author);
	if (scen->image)
		free(scen->image);
	if (scen->desc)
		free(scen->desc);
	if (scen->website)
		free(scen->website);
	if (scen->intro_msg)
		free(scen->intro_msg);
	if (scen->filename)
		free(scen->filename);

	free(scen);
	loaded_eaf = NULL;

	return (0);
}

/* loads the data out of a scenario.esf - expects scen has an open FILE pointer to a valid eaf file */
static int load_scen_data(ep_scenario *scen) {
	parsed_file *scen_esf = NULL;
	int i, j;

	if (!scen)
		return (-1);

	scen_esf = esf_new_handle();
	assert(scen_esf);

	if (esf_set_filter(scen_esf, "scenario") != 0)
		assert(-1);

	if (esf_parse_file_eaf(scen->eaf, scen_esf, "scenario.esf") != 0) {
		printf("Error parsing \"scenario.esf\n");
		esf_close_handle(scen_esf);
		return (-1);
	}

	/* read the parsed data and set scenario parameters */
	for (i = 0; i < scen_esf->num_items; i++) {
		for (j = 0; j < scen_esf->items[i].num_keys; j++) {
			char *key = scen_esf->items[i].keys[j].name;
			union _esf_value value = scen_esf->items[i].keys[j].value;

			if (!strcmp(key, "Name")) {
				scen->name = (char *)malloc(sizeof(char) * (strlen(value.cp) + 1));
				memset(scen->name, 0, sizeof(char) * (strlen(value.cp) + 1));
				strcpy(scen->name, value.cp);
			} else if (!strcmp(key, "Author")) {
				scen->author = (char *)malloc(sizeof(char) * (strlen(value.cp) + 1));
				memset(scen->author, 0, sizeof(char) * (strlen(value.cp) + 1));
				strcpy(scen->author, value.cp);
			} else if (!strcmp(key, "Image")) {
				scen->image = (char *)malloc(sizeof(char) * (strlen(value.cp) + 1));
				memset(scen->image, 0, sizeof(char) * (strlen(value.cp) + 1));
				strcpy(scen->image, value.cp);
			} else if (!strcmp(key, "Description")) {
				scen->desc = (char *)malloc(sizeof(char) * (strlen(value.cp) + 1));
				memset(scen->desc, 0, sizeof(char) * (strlen(value.cp) + 1));
				strcpy(scen->desc, value.cp);
			} else if (!strcmp(key, "Difficulty")) {
				char *diff = value.cp;

				if (!strcmp(diff, "Average"))
					scen->difficulty = AVERAGE;
				else if (!strcmp(diff, "Newbie"))
					scen->difficulty = NEWBIE;
				else if (!strcmp(diff, "Easy"))
					scen->difficulty = EASY;
				else if (!strcmp(diff, "Difficult"))
					scen->difficulty = DIFFICULT;
				else if (!strcmp(diff, "Insane"))
					scen->difficulty = INSANE;
			} else if (!strcmp(key, "Website")) {
				scen->website = (char *)malloc(sizeof(char) * (strlen(value.cp) + 1));
				memset(scen->website, 0, sizeof(char) * (strlen(value.cp) + 1));
				strcpy(scen->website, value.cp);
			} else if (!strcmp(key, "Introduction")) {
				scen->intro_msg = (char *)malloc(sizeof(char) * (strlen(value.cp) + 1));
				memset(scen->intro_msg, 0, sizeof(char) * (strlen(value.cp) + 1));
				strcpy(scen->intro_msg, value.cp);
			}
		}
	}

	if (esf_close_handle(scen_esf) != 0)
		assert(-1);

	return (0);
}

/* main scenario launcher - loads and then plays a scenario, used on main menu for main simulation game and arcade mode */
void do_scenario(ep_scenario *scen) {

	assert(scen);

	/* init the new game */
	init_new_game(scen);

	if (load_mission_eaf(scen->eaf, "scenario.esf") != 0) {
		printf("Could not load mission data for scenario. No objectives.\n");
	}

	/* run the game */
	game_loop(scen);

	/* close the scenario and return to the main menu */
	close_scenario(scen);
	close_mission();
}

/* brings up the scenario selection menu (what you see upon selecting new game) */
ep_scenario *do_scenario_select(void) {
	gui_window *wnd;
	gui_frame *frm;
	gui_button *main_btn, *custom_btn;

	selected_scen = NULL;

	s_session = gui_create_session();

	frm = gui_create_frame(290, 255, 225, 90, s_session);

	main_btn = gui_create_button(305, 270, 195, 25, "Main Simulation", s_session);
	gui_button_set_callback(main_btn, main_btn_callback);
	custom_btn = gui_create_button(305, 305, 195, 25, "Custom Mission", s_session);
	gui_button_set_callback(custom_btn, custom_btn_callback);

	gui_session_show_all(s_session);

	gui_main(s_session);

	gui_session_destroy_all(s_session);
	gui_destroy_session(s_session);

	return (selected_scen);
}

static void main_btn_callback(void) {
	selected_scen = load_scenario_eaf(apply_game_path("main.eaf"));
	s_session->active = 0;
}

static void custom_btn_callback(void) {
	if (do_custom_scenario_select() == 0) /* returns zero when they click 'ok', non-zero on cancel */
		s_session->active = 0;
}

/* returns 0 on select, non-zero on cancel */
static int do_custom_scenario_select(void) {
	gui_window *window;
	gui_frame *frame;
	gui_button *ok_btn, *cancel_btn;
	SDL_Surface *surf_bak = NULL; /* we draw what used to be on the screen to "erase" the window we're gonna draw */
	gui_listbox *lb;
	int i, selected = -1;
	
	custom_select_cancel = 0;
	custom_scenario_selected = -1;

	cs_session = gui_create_session();
	
	/* create the main window and it's frame */
	window = gui_create_window(125, 150, 550, 300, cs_session);
	frame = gui_create_frame(145, 170, 510, 260, cs_session);
	
	/* create the 'select' and 'cancel' buttons */
	ok_btn = gui_create_button(515, 390, 100, 25, "Select", cs_session);
	gui_button_set_callback(ok_btn, custom_ok_callback);
	cancel_btn = gui_create_button(405, 390, 100, 25, "Cancel", cs_session);
	gui_button_set_callback(cancel_btn, custom_cancel_callback);
	
	/* create the main listbox */
	lb = gui_create_listbox(155, 180, 150, 240, 35, cs_session);
	gui_listbox_set_callback(lb, lb_draw_scenarios);
	scenario_add_to_lb(lb, apply_game_path("/missions/"));
	
	/* create the description box */
	custom_desc_tb = gui_create_textbox(314, 313, 325, 68, "Select a scenario", cs_session);
	custom_desc_tb->visible = 0;
	custom_desc_tb->sb->visible = 0;
	
	surf_bak = get_surface(screen, 125, 150, 550, 300);
	assert(surf_bak);

	/* create the custom drawing area */
	/* the frame must be drawn first though (as that's what we're copying) */
	gui_show_frame(frame);
	custom_draw_area = get_surface(screen, 310, 180, 332, 130);
	assert(custom_draw_area);

	gui_session_show_all(cs_session);
	
	gui_main(cs_session);
	
	gui_session_destroy_all(cs_session);
	gui_destroy_session(cs_session);
	
	/* and "erase" the window */
	blit_image(surf_bak, 125, 150);
	SDL_FreeSurface(surf_bak);
	
	/* close all the scenarios we opened when generating the list */
	for (i = 0; i < num_scenarios; i++) {
		if (scenarios[i] != selected_scen)
			close_scenario(scenarios[i]);
		else
			selected = i;
	}
	
	if (selected != -1)
		if (scenarios[selected]->eaf != NULL)
			loaded_eaf = scenarios[selected]->eaf;
	
	/* free the custom drawing area */
	SDL_FreeSurface(custom_draw_area);
	custom_draw_area = NULL;
	
	return (custom_select_cancel);
}

static void custom_ok_callback(void) {
	custom_select_cancel = 0;
	cs_session->active = 0;
}

static void custom_cancel_callback(void) {
	custom_select_cancel = 1;
	cs_session->active = 0;
}

/* adds all scenarios found at 'path' to the listbox */
static void scenario_add_to_lb(gui_listbox *lb, char *path) {
#ifdef LINUX
	struct dirent **namelist = NULL;
	int n;
#endif
#ifdef WIN32
	HANDLE hlist;
	BOOL finished;
	WIN32_FIND_DATA file_data;
	TCHAR sz_dir[MAX_PATH+1];
#endif

	assert(lb);
	assert(path);

	num_scenarios = 0;

#ifdef LINUX
	/* read the directory and look for "*.eaf" files */
	n = scandir(path, &namelist, 0, alphasort);
	if (n < 0)
		perror("scandir");
	else {
		while(n--) {
			char *file = namelist[n]->d_name;
			int length = (signed)strlen(file);

			/* has to be at least ".eaf" */
			if (length > 4) {
				if ((file[length - 1] == 'f') && (file[length - 2] == 'a') && (file[length - 3] == 'e')&& (file[length - 4] == '.')) {
					/* filename ends in ".so", so, assume it's a plugin */
					char plugin_path[120] = {0};
					ep_scenario *scen = NULL;

					sprintf(plugin_path, "%s/%s", path, file);

					/* load the scenario */
					scen = load_scenario_eaf(plugin_path);

					if (scen != NULL) {
						scenarios[num_scenarios] = scen;
						num_scenarios++;
						gui_listbox_add_item(lb);
					}
				}
			}
			free(namelist[n]);
		}
		free(namelist);
	}
#endif
#ifdef WIN32

    /* Get the proper directory path */
    sprintf(sz_dir, "%s\\*", path);

	hlist = FindFirstFile(sz_dir, &file_data);
	if (hlist == INVALID_HANDLE_VALUE) {
		printf("invalid handle \"%s\"\n", path);
		return; /* no files found */
	} else {
		finished = FALSE;
		while(!finished) {
			printf("4\n");
			if (file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				/* ignore directories */
			} else {
				/* entry is a file */
				char *file = file_data.cFileName;
				int length = (signed)strlen(file);

				/* has to be at least ".eaf" */
				if (length > 4) {
					if ((file[length - 1] == 'f') && (file[length - 2] == 'a') && (file[length - 3] == 'e')&& (file[length - 4] == '.')) {
						/* filename ends in ".so", so, assume it's a plugin */
						char plugin_path[120] = {0};
						ep_scenario *scen = NULL;

						sprintf(plugin_path, "%s/%s", path, file);

						/* load the scenario */
						scen = load_scenario_eaf(plugin_path);

						if (scen != NULL) {
							scenarios[num_scenarios] = scen;
							num_scenarios++;
							gui_listbox_add_item(lb);
						}
					}
				}
			}

			/* see if there's another file */
			if (!FindNextFile(hlist, &file_data)) {
				if (GetLastError() == ERROR_NO_MORE_FILES) {
					finished = TRUE;
				}
			}

		}
	}

	FindClose(hlist);
#endif
}

/* callback to draw the custom scenario listbox */
static void lb_draw_scenarios(void *lb, int x, int y, int w, int h, int item) {
	int text_w, text_h, base;
	gui_listbox *listbox = (gui_listbox *)lb;

	custom_scenario_selected = listbox->selected;

	epiar_size_text(gui_font_bold, scenarios[item]->name, &text_w, &text_h, &base);
	epiar_render_text(gui_font_bold, white, 0, AFONT_RENDER_BLENDED, screen, x + 5, y + 6 + base, scenarios[item]->name);
	epiar_size_text(gui_font_normal, scenarios[item]->author, &text_w, &text_h, &base);
	epiar_render_text(gui_font_normal, white, 0, AFONT_RENDER_BLENDED, screen, x + 5, y + 19 + base, scenarios[item]->author);

	redraw_info_area(); /* does the main drawing of information about this scenario */
}

static void redraw_info_area(void) {
	SDL_Surface *temp;
	int selected = custom_scenario_selected;

	if (custom_scenario_selected == -1)
		return;

	/* "erase" what was there */
	blit_image(custom_draw_area, 310, 180);

	assert(custom_draw_area);

	/* draw the picture that the scenario.esf file specified */
	temp = eaf_load_png(scenarios[selected]->eaf, scenarios[selected]->image);
	if (temp) {
		blit_image(temp, 440, 185);

		SDL_FreeSurface(temp);
	}

	/* draw the rest of the information about the scenario */
	/* draw the name */
	epiar_render_text(gui_font_bold, white, 0, AFONT_RENDER_BLENDED, screen, 312, 190, "Name:");
	epiar_render_text(gui_font_normal, white, 0, AFONT_RENDER_BLENDED, screen, 312, 203, scenarios[selected]->name);

	/* draw the author */
	epiar_render_text(gui_font_bold, white, 0, AFONT_RENDER_BLENDED, screen, 312, 222, "Author:");
	epiar_render_text(gui_font_normal, white, 0, AFONT_RENDER_BLENDED, screen, 312, 235, scenarios[selected]->author);

	/* draw their website */
	epiar_render_text(gui_font_bold, white, 0, AFONT_RENDER_BLENDED, screen, 312, 254, "Website:");
	epiar_render_text(gui_font_normal, white, 0, AFONT_RENDER_BLENDED, screen, 312, 267, scenarios[selected]->website);

	/* draw the difficulty rating */
	epiar_render_text(gui_font_bold, white, 0, AFONT_RENDER_BLENDED, screen, 312, 284, "Difficulty");
	if (scenarios[selected]->difficulty == NEWBIE) {
		epiar_render_text(gui_font_normal, white, 0, AFONT_RENDER_BLENDED, screen, 312, 299, "Newbie");
	} else if (scenarios[selected]->difficulty == EASY) {
		epiar_render_text(gui_font_normal, white, 0, AFONT_RENDER_BLENDED, screen, 312, 299, "Easy");
	} else if (scenarios[selected]->difficulty == AVERAGE) {
		epiar_render_text(gui_font_normal, white, 0, AFONT_RENDER_BLENDED, screen, 312, 299, "Average");
	} else if (scenarios[selected]->difficulty == DIFFICULT) {
		epiar_render_text(gui_font_normal, white, 0, AFONT_RENDER_BLENDED, screen, 312, 299, "Difficult");
	} else if (scenarios[selected]->difficulty == INSANE) {
		epiar_render_text(gui_font_normal, white, 0, AFONT_RENDER_BLENDED, screen, 312, 299, "Insane");
	}

	/* draw the description */
	custom_desc_tb->visible = 1;
	custom_desc_tb->update = 1;
	custom_desc_tb->sb->visible = 1;
	custom_desc_tb->sb->update = 1;

	gui_textbox_change_text(custom_desc_tb, scenarios[selected]->desc);

	selected_scen = scenarios[selected];
}
