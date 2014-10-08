#include "alliances/alliances.h"
#include "asteroid/asteroid.h"
#include "audio/audio.h"
#include "audio/music.h"
#include "com_defs.h"
#include "force/force.h"
#include "game/game.h"
#include "game/scenario.h"
#include "game/update.h"
#include "gui/gui.h"
#include "hud/hud.h"
#include "includes.h"
#include "input/input.h"
#include "main.h"
#include "missions/missions.h"
#include "outfit/outfit.h"
#include "racing/track.h"
#include "sprite/flare.h"
#include "sprite/planet.h"
#include "sprite/r_ships.h"
#include "sprite/sprite.h"
#include "system/init.h"
#include "system/path.h"
#include "system/timer.h"
#include "system/video/video.h"

#define TIME_TO_FADE 1500 /* the time (in ms) for the end game loop fade out */

Uint32 average_loop_time = 0;
float average_session_fps = 0;
Uint32 total_play_time = 0;
Uint32 total_frames_drawn = 0;
static unsigned char loop;
static Uint32 game_over_message_start_time;
Uint32 game_start_time = 0;
static unsigned char gave_scenario_msg;
float current_fps = 0.0f;

static void do_new_scenario_message(char *desc);

/* initialize all variables, game loop expects this to be done, whether it's
   for a new game or a saved game, game_loop doesnt initialize any variables,
   only modifies them according to the game itself */
void init_new_game(ep_scenario *scen) {
	unsigned char loading_main = 0;

	if (!strcmp(scen->name, "Epiar Main Simulation")) loading_main = 1;

	/* hide the mouse and clear the screen */
	SDL_ShowCursor(0);
	black_fill(0, 0, 800, 600, 1);

	init_subsystems();

	init_ships();

	/* load tracks (note, no default tracks) */
	if (init_tracks_eaf(scen->eaf, "tracks.ert") != 0)
		printf("No courses found in EAF archive.\n");

	/* load engines */
	load_engines_eaf(main_eaf, "engines.esf");
	if (!loading_main) {
		if (load_engines_eaf(scen->eaf, "engines.esf") != 0)
			printf("No engines found in EAF archive.\n");
	}

	/* load shields */
	load_shields_eaf(main_eaf, "shields.esf");
	if (!loading_main) {
		if (load_shields_eaf(scen->eaf, "shields.esf") != 0)
			printf("No shields found in EAF archive.\n");
	}

	/* load weapons */
	load_weapons_eaf(main_eaf, "weapons.esf");
	if (!loading_main) {
		if (load_weapons_eaf(scen->eaf, "weapons.esf") != 0)
			printf("No weapons found in EAF archive.\n");
	}

	/* load types */
	load_models_eaf(main_eaf, "models.esf");
	if (!loading_main) {
		if (load_models_eaf(scen->eaf, "models.esf") != 0)
			printf("No models found in EAF archive.\n");
	}

	/* load alliances */
	load_alliances_eaf(main_eaf, "alliances.esf");
	if (!loading_main) {
		if (load_alliances_eaf(scen->eaf, "alliances.esf") != 0)
			printf("No alliances found in EAF archive.\n");
	}

	/* load outfits */
	load_outfits_eaf(main_eaf, "outfit.esf");
	if (!loading_main) {
		if (load_outfits_eaf(scen->eaf, "outfit.esf") != 0)
			printf("No additional outfits found in scenario.\n");
	}

	if (init_gates_eaf(scen->eaf, "gates.esf") != 0)
		printf("No gates found in EAF archive.\n");
	if (load_planets_eaf(scen->eaf, "planets.esf") != 0)
		printf("No planets found in EAF archive.\n");
	if (load_asteroids_eaf(scen->eaf, "asteroids.esf") != 0)
		printf("No asteroid fields found in EAF archive.\n");
	if (load_ships_eaf(scen->eaf, "ships.esf") != 0)
		printf("No ships found in EAF archive.\n");

	init_force();

	unlock_keys();

	init_r_ships();

	game_start_time = SDL_GetTicks();
	gave_scenario_msg = 0;
}

void game_loop(ep_scenario *scen) {
  SDL_Surface *fader;
  SDL_Rect src, dest;
  char msg[60];
  unsigned char asked_about_end_scenario = 0;
  int a;
  int num_flips = 0;
  Uint32 game_start_time = get_ticks();
  static int num_sessions = 0;
  Uint32 session_play_time = 0;
  
  game_over_message_start_time = get_ticks();
  
  update_universe(); /* call before first loop to make sure all sprites
		      * have valid screen coords */
  
  sprintf(msg, "Epiar %s written by Entropy Development Studios", epiar_version);
  
  hud_message(msg, 3500);
  
  loop = 1;
  
  /* main game loop, right here! look no further! */
  while(loop) {
    erase_frame();
    loop = get_input();
    update_universe();
    update_audio();
    a = update_mission();
    draw_frame(0);
    num_flips++;
    total_frames_drawn++;
    current_fps = (float)num_flips / (float)(((float)get_ticks() - (float)game_start_time) / 1000.0f);
    flip();
    if (!do_post_draw()) /* performs anything that must be after drawing */
      loop = 0;
    if (a == 1) {
      if (!asked_about_end_scenario) {
	asked_about_end_scenario = 1;
	gui_alert("Congratulations! Mission successful!");
	if (gui_question("Do you wish to end the scenario?") == 1)
	  loop = 0;
	black_fill(0, 0, 800, 600, 1);
	draw_frame(1); /* redraw over the dialog */
      }
    } else if (a == 2) {
      if (!asked_about_end_scenario) {
	asked_about_end_scenario = 1;
	gui_alert("Failed! Mission incomplete!");
	if (gui_question("Do you wish to end the scenario?") == 1)
	  loop = 0;

	black_fill(0, 0, 800, 600, 1);
	draw_frame(1); /* redraw over the dialog */
      }
    }
    if (!gave_scenario_msg) {
      pause_timer();
      do_new_scenario_message(scen->intro_msg);
      unpause_timer();
      gave_scenario_msg = 1;
    }
    pause_loop(-1);
  }
  
  num_sessions++;
  session_play_time = get_ticks() - game_start_time;
  printf("Session #%d had %f fps\n", num_sessions, (float)num_flips / (float)((float)session_play_time / 1000.0f));
  total_play_time += session_play_time;
  average_session_fps = (float)total_frames_drawn / (float)((float)total_play_time / 1000.0f);
  
  /* fade out the last frame when exiting the main game loop */
  fader = SDL_CreateRGBSurface(SDL_SWSURFACE, screen->w, screen->h, screen->format->BitsPerPixel, screen->format->Rmask, screen->format->Gmask, screen->format->Bmask, screen->format->Amask);
  src.x = 0;
  src.y = 0;
  src.w = screen->w;
  src.h = screen->h;
  dest = src;
  SDL_BlitSurface(screen, &src, fader, &dest);
  
  SDL_SetAlpha(fader, SDL_SRCALPHA, 255);
  
  loop = 1;
  
  while(loop) {
    Uint8 change_amount;
    static Uint32 old_time;
    old_time = SDL_GetTicks();
    blit_surface(fader, &src, &dest, 1);
    flip();
    black_fill(0, 0, dest.w, dest.h, 1);
    change_amount = (255 * (SDL_GetTicks() - old_time)) / TIME_TO_FADE;
    if ((fader->format->alpha - change_amount) < 0) loop = 0;
    SDL_SetAlpha(fader, SDL_SRCALPHA, (int)(fader->format->alpha - change_amount));
  }
  
  SDL_FreeSurface(fader);
  
  stop_music();
  stop_audio();
  
  /* unload the scenario */
  unload_models();
  unload_weapons();
  unload_shields();
  unload_engines();
  unload_ships();
  unload_asteroids();
  unload_planets();
  unload_gates();
  uninit_subsystems();
  unload_alliances();
  deinit_tracks();
  deinit_force();
  unload_outfits();
  reset_timer();
}

/* desired is ignored if set to -1, otherwise, it overrides command line desired-fps (used by gui to ensure quick return) */
void pause_loop(int delay) {
	static Uint32 old_time = 0; /* last time the loop was finished */
	static Uint32 desired_loop_time = 0; /* how long, in ms, you _want_ the loop to be (used in frame cap) */
	Uint32 last_loop; /* time it took since the last loop */
	extern int desired_fps; /* the desired frames per second */
	Uint32 delay_length = 0; /* how long to wait to keep under the frame cap */
	Uint32 current_time;

	if (delay != -1) {
		old_time = SDL_GetTicks(); /* pass anything but -1 if you didnt update this for a while but you dont want it to seem like the loop took years (gui) */
		return;
	}

	current_time = SDL_GetTicks();

	if (desired_loop_time == 0) desired_loop_time = (unsigned)(1000 / desired_fps);

	if (old_time == 0) old_time = current_time;

	last_loop = current_time - old_time;

	if (last_loop < desired_loop_time) {
		delay_length = desired_loop_time - last_loop;
		SDL_Delay(delay_length);
	}

	if (average_loop_time == 0)
		average_loop_time = last_loop;
	else {
		if (delay_length == 0)
			average_loop_time = (average_loop_time + last_loop) / 2;
		else
			average_loop_time = (average_loop_time + (last_loop + delay_length)) / 2;
	}

	old_time = current_time;
}

void game_over(void) {
#ifndef NDEBUG
	fprintf(stdout, "Aw, you only lasted %d seconds!\n", (SDL_GetTicks() - game_over_message_start_time) / 1000);
#endif
	loop = 0;
}

/* used by the dialog (below) */
gui_session *session;

static void btn_callback(void) {
	session->active = 0;
}

/* brings up the new scenario message box */
static void do_new_scenario_message(char *desc) {
	gui_window *window;
	gui_button *btn;
	gui_textbox *tb;

	session = gui_create_session();

	window = gui_create_window(250, 210, 300, 180, session);
	btn = gui_create_button(310, 345, 180, 30, "Done", session);
	gui_button_set_callback(btn, btn_callback);
	tb = gui_create_textbox(265, 225, 270, 115, desc, session);

	gui_session_show_all(session);

	gui_main(session);

	gui_session_destroy_all(session);
	gui_destroy_session(session);

	black_fill(0, 0, 800, 600, 1);
	draw_frame(1);
}
