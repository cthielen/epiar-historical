#include "audio/audio.h"
#include "comm/comm.h"
#include "includes.h"
#include "input/input.h"
#include "land/land.h"
#include "menu/options.h"
#include "menu/status.h"
#include "navigation/navigation.h"
#include "sprite/fire.h"
#include "sprite/sprite.h"
#include "system/debug.h"
#include "system/video/video.h"

#define CAMERA_SPEED 64
#define REPEAT_DELAY 150 /* in milliseconds */

static SDLKey get_assigned_key(char *line);

struct {
	unsigned char esc,
		toggle_audio,
		screenshot,
		land,
		board,
		target,
		near_target,
		pri_next,
		pri_last,
		sec_next,
		sec_last,
		hail,
		nav,
		turn_left,
		turn_right,
		thrust,
		fire_pri,
		fire_sec;
} tracked_keys;

struct _post_draw post_draw;
struct _key_locks key_locks;
struct _keys keys;

static char *time_stamp(void);
static void set_default_keybindings(void);

/******************************************************************************      
*
*   Name:
*      int get_input(unsigned char take_action);
*
*   Abstract:
*      Checks the status of various input devices and fills the input buffer.
*   It also parses the input buffer, doing anything required (rotating the
*   ship, taking a screenshot, etc.) then clears the input buffer.
*
*   Context/Scope:	
*      Called throughout Epiar.
*
*   Side Effects:
*      None.
*
*   Return Value:
*      1 on a normal keystroke, 0 if the keystroke means to quit
*
*   Assumptions:
*      keys array must be filled to work correctly (load_input_cfg() does this)
*
******************************************************************************/
int get_input(void) {
	SDL_Event event;
	extern Uint32 player_dead;
	extern unsigned char view_mode;
	extern int ship_to_follow;

	while(SDL_PollEvent(&event)) {
		switch(event.type) {
		case SDL_KEYDOWN:
			{
				SDLKey key = event.key.keysym.sym;
				
				if ((key == keys.quit) && !key_locks.quit)
					tracked_keys.esc = 1;
				else if ((key == keys.nav) && !key_locks.nav)
					tracked_keys.nav = 1;
				else if ((key == keys.options) && !key_locks.options)
					post_draw.options = 1;
				else if ((key == keys.screenshot) && !key_locks.screenshot)
					tracked_keys.screenshot = 1;
				else if ((key == keys.toggle_audio) && !key_locks.toggle_audio)
					tracked_keys.toggle_audio = 1;
				else if ((key == keys.land) && !key_locks.land && !view_mode)
					tracked_keys.land = 1;
				else if ((key == keys.rotate_left) && !key_locks.rotate_left)
					tracked_keys.turn_left = 1;
				else if ((key == keys.rotate_right) && !key_locks.rotate_right)
					tracked_keys.turn_right = 1;
				if (!player_dead) {
					if ((key == keys.thrust) && !key_locks.thrust)
						tracked_keys.thrust = 1;
					else if ((key == keys.booster) && !key_locks.booster)
						player.ship->boost = 1;
					else if ((key == keys.target) && !key_locks.target)
						tracked_keys.target = 1;
					else if ((key == keys.deselect_target) && !key_locks.deselect_target) {
						player.target.target = NULL;
						player.target.type = TARGET_NONE;
						player.target.dist = 0.0f;
					} else if ((key == keys.pri_next) && !key_locks.pri_next)
						tracked_keys.pri_next = 1;
					else if ((key == keys.pri_last) && !key_locks.pri_last)
						tracked_keys.pri_last = 1;
					else if ((key == keys.sec_next) && !key_locks.sec_next)
						tracked_keys.sec_next = 1;
					else if ((key == keys.sec_last) && !key_locks.sec_last)
						tracked_keys.sec_last = 1;
					else if ((key == keys.board) && !key_locks.board)
						tracked_keys.board = 1;
					else if ((key == keys.hail) && !key_locks.hail)
						tracked_keys.hail = 1;
					else if ((key == keys.status) && !key_locks.status)
						ship_status();
#ifndef WIN32
#warning ship_status() should be finished and should show information
#endif
					else if ((key == keys.near_target) && !key_locks.near_target)
						tracked_keys.near_target = 1;
					else if ((key == keys.fire) && !key_locks.fire)
						tracked_keys.fire_pri = 1;
					else if ((key == keys.alt_fire) && !key_locks.alt_fire)
						tracked_keys.fire_sec = 1;
				}

				break;
			}
		case SDL_KEYUP:
			{
				SDLKey key = event.key.keysym.sym;

				if ((key == keys.quit) && tracked_keys.esc) {
					post_draw.options = 1;
					tracked_keys.esc = 0;
				} else if ((key == keys.nav) && tracked_keys.nav) {
					post_draw.nav = 1;
					tracked_keys.nav = 0;
				} else if ((key == keys.screenshot) && tracked_keys.screenshot) {
					post_draw.screenshot = 1;
					tracked_keys.screenshot = 0;
				} else if ((key == keys.toggle_audio) && tracked_keys.toggle_audio) {
					toggle_mute();
					tracked_keys.toggle_audio = 0;
				}
				if (!player_dead) {
					if ((key == keys.land) && tracked_keys.land) {
						post_draw.land = 1;
						tracked_keys.land = 0;
					} else if ((key == keys.rotate_left) && tracked_keys.turn_left)
						tracked_keys.turn_left = 0;
					else if ((key == keys.rotate_right) && tracked_keys.turn_right)
						tracked_keys.turn_right = 0;
					else if ((key == keys.thrust) && tracked_keys.thrust)
						tracked_keys.thrust = 0;
					else if ((key == keys.booster) && !key_locks.booster)
						player.ship->boost = 0;
					else if ((key == keys.target) && !key_locks.target) {
						cycle_targets();
						tracked_keys.target = 0;
#ifndef WIN32
#warning cycle_targets() and set_near_target() should not set a target that is cloaked, fix me
#endif
					} else if ((key == keys.pri_next) && !key_locks.pri_next) {
						weapon_cycle(player.ship, CYCLE_PRI, CYCLE_FOR);
						tracked_keys.pri_next = 0;
					} else if ((key == keys.pri_last) && !key_locks.pri_last) {
						weapon_cycle(player.ship, CYCLE_PRI, CYCLE_BAC);
						tracked_keys.pri_last = 0;
					} else if ((key == keys.sec_next) && !key_locks.sec_next) {
						weapon_cycle(player.ship, CYCLE_SEC, CYCLE_FOR);
						tracked_keys.sec_next = 0;
					} else if ((key == keys.sec_last) && !key_locks.sec_last) {
						weapon_cycle(player.ship, CYCLE_SEC, CYCLE_BAC);
						tracked_keys.sec_last = 0;
					} else if ((key == keys.board) && !key_locks.board) {
						post_draw.board = 1;
						tracked_keys.board = 0;
					} else if ((key == keys.hail) && !key_locks.hail) {
						post_draw.hail = 1;
						tracked_keys.hail = 0;
					} else if ((key == keys.near_target) && !key_locks.near_target) {
						set_target_nearest();
						tracked_keys.near_target = 0;
					} else if ((key == keys.fire) && !key_locks.fire)
						tracked_keys.fire_pri = 0;
					else if ((key == keys.alt_fire) && !key_locks.alt_fire)
						tracked_keys.fire_sec = 0;
				}

				break;
			}
		default:
			break;
		}
	}

	/* do any operations for pressed keys */
	if (tracked_keys.turn_left)
		turn_ship(player.ship, 0);
	else if (tracked_keys.turn_right)
		turn_ship(player.ship, 1);

	if (tracked_keys.thrust && !player_dead)
		thrust_ship(player.ship);
	if (tracked_keys.fire_pri)
		fire(player.ship, player.ship->w_slot[0]);
	if (tracked_keys.fire_sec)
		fire(player.ship, player.ship->w_slot[1]);
	
	return (1);
}

/******************************************************************************      
*
*   Name:
*      int load_input_cfg(void);
*
*   Abstract:
*      Parses the .epiar-input.ecf for lines like "rotate left = SDLK_LEFT" and
*   assigns the SDL key value of SDLK_LEFT to the keys array element for rotate
*   left, so that the get_input() function knows whether SDLK_LEFT is a relevent
*   key, and if so, what to do with it (in this case, rotate the ship left).
*
*   Context/Scope:
*      Called from init().
*
*   Side Effects:
*      None.
*
*   Return Value:
*      0 on success
*      -1 on error (no file found _and_ could not create it)
*
*   Assumptions:
*      None
*
******************************************************************************/
int load_input_cfg(void) {
	FILE *fp = fopen("./.epiar-input.ecf", "rb");

	/* make sure all keys are enabled */
	unlock_keys();

	/* clean the post_draw items (these are flagged to be done _after_ drawing, not during get_input(), which is after erasing) */
	post_draw.screenshot = 0;
	post_draw.hail = 0;
	post_draw.land = 0;
	post_draw.options = 0;
	post_draw.nav = 0;

	/* clear event flags */
	tracked_keys.esc = 0;
	tracked_keys.toggle_audio = 0;
	tracked_keys.screenshot = 0;
	tracked_keys.land = 0;
	tracked_keys.board = 0;
	tracked_keys.target = 0;
	tracked_keys.near_target = 0;
	tracked_keys.pri_next = 0;
	tracked_keys.pri_last = 0;
	tracked_keys.sec_next = 0;
	tracked_keys.sec_last = 0;
	tracked_keys.hail = 0;
	tracked_keys.nav = 0;
	tracked_keys.turn_left = 0;
	tracked_keys.turn_right = 0;
	tracked_keys.thrust = 0;
	tracked_keys.fire_pri = 0;
	tracked_keys.fire_sec = 0;

	/* Make sure the file exists, if not, create it. */
	if (fp == NULL) {
		fprintf(stdout, "Could not open '~/.epiar-input.ecf`. Creating ... ");
		set_default_keybindings();
		save_keybindings();
		fprintf(stdout, "done.\n");
		return (0); /* nothing more to do in load_input_cfg() */
	} else {
		float file_version = 0.0f;
		/* read the file into the struct */
		fp = fopen("./.epiar-input.ecf", "rb");

		if (fp == NULL) {
			fprintf(stdout, "Could not open \"./.epiar-input.ecf\" for reading, assuming default bindings.\n");
			set_default_keybindings();
			return (0);
		}

		fread(&file_version, sizeof(file_version), 1, fp);

		if (file_version != 0.2f) {
			fprintf(stdout, "Incorrect file version. Please delete .epiar-input.ecf if you want to save keybindings.\n");
			set_default_keybindings();
			return (0);
		}

		if (!fread(&keys, sizeof(struct _keys), 1, fp)) {
			fprintf(stdout, "Could not read from \".epiar-input.ecf\" to get keybindings. Assuming default keys.\n");
			set_default_keybindings();
			fclose(fp);
			return (0);
		}

		fclose(fp);
	}

	return (0);
}

/******************************************************************************      
*
*   Name:
*      static SDLKey get_assigned_key(char *line);
*
*   Abstract:
*      Takes text, such as SDLK_LEFT, and returns am SDLKey value for that
*   key.
*
*   Context/Scope:
*      This file only.
*
*   Side Effects:
*      None.
*
*   Return Value:
*      SDLKey, the value of of the key descriped by char *line or,
*      -1 if an SDL value couldn't be associated with that line.
*
*   Assumptions:
*      None
*
******************************************************************************/
static SDLKey get_assigned_key(char *line) {
	if (!strcmp(line, "SDLK_LEFT")) return (SDLK_LEFT);
	if (!strcmp(line, "SDLK_RIGHT")) return (SDLK_RIGHT);
	if (!strcmp(line, "SDLK_UP")) return (SDLK_UP);
	if (!strcmp(line, "SDLK_DOWN")) return (SDLK_DOWN);
	if (!strcmp(line, "SDLK_ESCAPE")) return (SDLK_ESCAPE);
	if (!strcmp(line, "SDLK_SPACE")) return (SDLK_SPACE);
	if (!strcmp(line, "SDLK_TAB")) return (SDLK_TAB);
	if (!strcmp(line, "SDLK_PRINT")) return (SDLK_PRINT);
	if (!strcmp(line, "SDLK_w")) return (SDLK_w);
	if (!strcmp(line, "SDLK_RETURN")) return (SDLK_RETURN);
	if (!strcmp(line, "SDLK_a")) return (SDLK_a);
	if (!strcmp(line, "SDLK_b")) return (SDLK_b);
	if (!strcmp(line, "SDLK_c")) return (SDLK_c);
	if (!strcmp(line, "SDLK_d")) return (SDLK_d);
	if (!strcmp(line, "SDLK_e")) return (SDLK_e);
	if (!strcmp(line, "SDLK_f")) return (SDLK_f);
	if (!strcmp(line, "SDLK_g")) return (SDLK_g);
	if (!strcmp(line, "SDLK_h")) return (SDLK_h);
	if (!strcmp(line, "SDLK_i")) return (SDLK_i);
	if (!strcmp(line, "SDLK_j")) return (SDLK_j);
	if (!strcmp(line, "SDLK_k")) return (SDLK_k);
	if (!strcmp(line, "SDLK_l")) return (SDLK_l);
	if (!strcmp(line, "SDLK_m")) return (SDLK_m);
	if (!strcmp(line, "SDLK_n")) return (SDLK_n);
	if (!strcmp(line, "SDLK_o")) return (SDLK_o);
	if (!strcmp(line, "SDLK_p")) return (SDLK_p);
	if (!strcmp(line, "SDLK_q")) return (SDLK_q);
	if (!strcmp(line, "SDLK_r")) return (SDLK_r);
	if (!strcmp(line, "SDLK_s")) return (SDLK_s);
	if (!strcmp(line, "SDLK_t")) return (SDLK_t);
	if (!strcmp(line, "SDLK_u")) return (SDLK_u);
	if (!strcmp(line, "SDLK_v")) return (SDLK_v);
	if (!strcmp(line, "SDLK_w")) return (SDLK_w);
	if (!strcmp(line, "SDLK_x")) return (SDLK_x);
	if (!strcmp(line, "SDLK_y")) return (SDLK_y);
	if (!strcmp(line, "SDLK_z")) return (SDLK_z);
	if (!strcmp(line, "SDLK_PLUS")) return (SDLK_PLUS);
	if (!strcmp(line, "SDLK_MINUS")) return (SDLK_MINUS);
	if (!strcmp(line, "SDLK_F1")) return (SDLK_F1);
	if (!strcmp(line, "SDLK_F2")) return (SDLK_F2);
	if (!strcmp(line, "SDLK_F3")) return (SDLK_F3);
	if (!strcmp(line, "SDLK_F4")) return (SDLK_F4);

	return (-1);
}

/* returns a basic time stamp */
static char *time_stamp(void) {
	time_t raw_time;
	struct tm *timeinfo;
	char year[5];
	char month[3];
	char day[3];
	char hour[3];
	char min[3];
	char sec[3];
	char *buffer;

	buffer = (char *)malloc(sizeof(char) * 17);

	time( &raw_time );
	timeinfo = localtime(&raw_time);

	sprintf(year, "%d", timeinfo->tm_year + 1900);
	if (timeinfo->tm_mon < 10)
		sprintf(month, "0%d", timeinfo->tm_mon);
	else
		sprintf(month, "%d", timeinfo->tm_mon);
	if (timeinfo->tm_mday < 10)
		sprintf(day, "0%d", timeinfo->tm_mday);
	else
		sprintf(day, "%d", timeinfo->tm_mday);
	if (timeinfo->tm_hour < 10)
		sprintf(hour, "0%d", timeinfo->tm_hour);
	else
		sprintf(hour, "%d", timeinfo->tm_hour);
	if (timeinfo->tm_min < 10)
		sprintf(min, "0%d", timeinfo->tm_min);
	else
		sprintf(min, "%d", timeinfo->tm_min);
	if (timeinfo->tm_sec < 10)
		sprintf(sec, "0%d", timeinfo->tm_sec);
	else
		sprintf(sec, "%d", timeinfo->tm_sec);
	sprintf(buffer, "%s%s%s-%s%s%s", year, month, day, hour, min, sec);

	return (buffer);
}

/* takes SDLKey and returns char * of the name */
char *get_key_name(SDLKey key) {
	if (key == SDLK_LEFT) return ("Left");
	if (key == SDLK_RIGHT) return ("Right");
	if (key == SDLK_UP) return ("Up");
	if (key == SDLK_DOWN) return ("Down");
	if (key == SDLK_SPACE) return ("Spacebar");
	if (key == SDLK_TAB) return ("Tab");
	if (key == SDLK_PRINT) return ("Printscr");
	if (key == SDLK_RETURN) return ("Enter");
	if (key == SDLK_ESCAPE) return ("Escape");
	if (key == SDLK_a) return ("A");
	if (key == SDLK_b) return ("B");
	if (key == SDLK_c) return ("C");
	if (key == SDLK_d) return ("D");
	if (key == SDLK_e) return ("E");
	if (key == SDLK_f) return ("F");
	if (key == SDLK_g) return ("G");
	if (key == SDLK_h) return ("H");
	if (key == SDLK_i) return ("I");
	if (key == SDLK_j) return ("J");
	if (key == SDLK_k) return ("K");
	if (key == SDLK_l) return ("L");
	if (key == SDLK_m) return ("M");
	if (key == SDLK_n) return ("N");
	if (key == SDLK_o) return ("O");
	if (key == SDLK_p) return ("P");
	if (key == SDLK_q) return ("Q");
	if (key == SDLK_r) return ("R");
	if (key == SDLK_s) return ("S");
	if (key == SDLK_t) return ("T");
	if (key == SDLK_u) return ("U");
	if (key == SDLK_v) return ("V");
	if (key == SDLK_w) return ("W");
	if (key == SDLK_x) return ("X");
	if (key == SDLK_y) return ("Y");
	if (key == SDLK_z) return ("Z");
	if (key == SDLK_PLUS) return ("+");
	if (key == SDLK_MINUS) return ("-");
	if (key == SDLK_F1) return ("F1");
	if (key == SDLK_F2) return ("F2");
	if (key == SDLK_F3) return ("F3");
	if (key == SDLK_F4) return ("F4");

	return (NULL);
}

void save_keybindings(void) {
	float file_version = 0.2f;
	FILE *fp;

	fp = fopen("./.epiar-input.ecf", "wb");
	if (fp == NULL) {
		fprintf(stdout, "Could not create '~/.epiar-input.ecf' to save keybindings\n");
		return;
	}
	fwrite(&file_version, sizeof(file_version), 1, fp);

	if (!fwrite(&keys, sizeof(struct _keys), 1, fp))
		fprintf(stdout, "Could not write to \".epiar-input.ecf\" to save keybindings.\n");

	fclose(fp);
}

/* unlocks all keys */
int unlock_keys(void) {
	key_locks.quit = 0;
	key_locks.rotate_left = 0;
	key_locks.rotate_right = 0;
	key_locks.thrust = 0;
	key_locks.booster = 0;
	key_locks.fire = 0;
	key_locks.alt_fire = 0;
	key_locks.screenshot = 0;
	key_locks.select = 0;
	key_locks.next_ship = 0;
	key_locks.last_ship = 0;
	key_locks.toggle_audio = 0;
	key_locks.options = 0;
	key_locks.land = 0;
	key_locks.status = 0;
	key_locks.nav = 0;
	key_locks.hail = 0;
	key_locks.target = 0;
	key_locks.deselect_target = 0;
	key_locks.near_target = 0;
	key_locks.board = 0;
	key_locks.pri_next = 0;
	key_locks.pri_last = 0;
	key_locks.sec_next = 0;
	key_locks.sec_last = 0;

	return (1); /* always returns 1 */
}

/* locks all keys (except esc, screenshot, and toggle audio, that'd just be rude) */
int lock_keys(void) {
	key_locks.rotate_left = 1;
	key_locks.rotate_right = 1;
	key_locks.thrust = 1;
	key_locks.booster = 1;
	key_locks.fire = 1;
	key_locks.alt_fire = 1;
	key_locks.select = 1;
	key_locks.next_ship = 1;
	key_locks.last_ship = 1;
	key_locks.options = 1;
	key_locks.land = 1;
	key_locks.status = 1;
	key_locks.nav = 1;
	key_locks.hail = 1;
	key_locks.target = 1;
	key_locks.deselect_target = 1;
	key_locks.near_target = 1;
	key_locks.board = 1;
	key_locks.pri_next = 1;
	key_locks.pri_last = 1;
	key_locks.sec_next = 1;
	key_locks.sec_last = 1;

	return (1); /* always returns 1 (for missions code) */
}

int do_post_draw(void) {
	unsigned char force_full_update = 0;
	
	if (post_draw.screenshot) {
		char *a;
		char filename[32];
		
		post_draw.screenshot = 0;
		a = time_stamp();
		strcpy(filename, "screenshot-");
		strcat(filename, a);
		free(a);
		a = NULL;
		strcat(filename, ".bmp");
		SDL_SaveBMP(screen, filename);
	}
	if (post_draw.hail) {
		post_draw.hail = 0;
		hail();
		force_full_update = 1;
	}
	if (post_draw.land) {
		post_draw.land = 0;
		toggle_land();
		force_full_update = 1;
	}
	if (post_draw.options) {
		post_draw.options = 0;
		force_full_update = 1;
		if (!options(1))
			return (0);
	}
	if (post_draw.nav) {
		post_draw.nav = 0;
		nav_map();
		force_full_update = 1;
	}
	if (post_draw.board) {
		post_draw.board = 0;
		board_ship();
		force_full_update = 1;
	}
	
	if (force_full_update) {
		/* need to completely redraw the screen */
		black_fill(0, 0, 800, 600, 1);
		draw_frame(1);
	}
	
	return (1);
}

void unlock_key(char *name) {
	if (name == NULL)
		return;

	if (!strcmp(name, "rotate_left")) {
		key_locks.rotate_left = 0;
		return;
	}
	if (!strcmp(name, "rotate_right")) {
		key_locks.rotate_right = 0;
	}
	if (!strcmp(name, "thrust")) {
		key_locks.thrust = 0;
	}
}

void lock_key(char *name) {
	if (name == NULL)
		return;

	if (!strcmp(name, "rotate_left")) {
		key_locks.rotate_left = 1;
		return;
	}
	if (!strcmp(name, "rotate_right")) {
		key_locks.rotate_right = 1;
	}
	if (!strcmp(name, "thrust")) {
		key_locks.thrust = 1;
	}
}

void clear_events(void) {
	SDL_Event event;

	while (SDL_PollEvent(&event));
}

static void set_default_keybindings(void) {
	keys.quit = SDLK_ESCAPE;
	keys.rotate_left = SDLK_LEFT;
	keys.rotate_right = SDLK_RIGHT;
	keys.thrust = SDLK_UP;
	keys.booster = SDLK_TAB;
	keys.fire = SDLK_SPACE;
	keys.alt_fire = SDLK_w;
	keys.screenshot = SDLK_PRINT;
	keys.select = SDLK_RETURN;
	keys.next_ship = SDLK_PLUS;
	keys.last_ship = SDLK_MINUS;
	keys.toggle_audio = SDLK_q;
	keys.options = SDLK_F2;
	keys.land = SDLK_l;
	keys.status = SDLK_F4;
	keys.nav = SDLK_n;
	keys.hail = SDLK_h;
	keys.target = SDLK_t;
	keys.deselect_target = SDLK_r;
	keys.near_target = SDLK_y;
	keys.board = SDLK_b;
	keys.pri_next = SDLK_LSHIFT;
	keys.pri_last = SDLK_LALT;
	keys.sec_next = SDLK_RSHIFT;
	keys.sec_last = SDLK_RALT;
}

void flush_events(void) {
	SDL_Event event;

	while(SDL_PollEvent(&event));
}

/* resets all key tracking to default states */
void reset_input(void) {
	tracked_keys.esc = 0;
	tracked_keys.toggle_audio = 0;
	tracked_keys.screenshot = 0;
	tracked_keys.land = 0;
	tracked_keys.board = 0;
	tracked_keys.target = 0;
	tracked_keys.near_target = 0;
	tracked_keys.pri_next = 0;
	tracked_keys.pri_last = 0;
	tracked_keys.sec_next = 0;
	tracked_keys.sec_last = 0;
	tracked_keys.hail = 0;
	tracked_keys.nav = 0;
	tracked_keys.turn_left = 0;
	tracked_keys.turn_right = 0;
	tracked_keys.thrust = 0;
	tracked_keys.fire_pri = 0;
	tracked_keys.fire_sec = 0;
}
