#include "asteroid/asteroid.h"
#include "audio/audio.h"
#include "audio/music.h"
#include "audio/playlist.h"
#include "com_defs.h"
#include "game/game.h"
#include "game/update.h"
#include "gui/gui.h"
#include "hud/hud.h"
#include "includes.h"
#include "input/input.h"
#include "main.h"
#include "sprite/chunk.h"
#include "sprite/fire.h"
#include "sprite/flare.h"
#include "sprite/gate.h"
#include "sprite/particle.h"
#include "sprite/planet.h"
#include "sprite/sprite.h"
#include "sprite/weapon.h"
#include "system/debug.h"
#include "system/eaf.h"
#include "system/init.h"
#include "system/path.h"
#include "system/plugin.h"
#include "system/rander.h"
#include "system/trig.h"
#include "system/video/video.h"

int init(int bpp) {
	char wm_caption[20];

	fprintf(stdout, "Epiar %s\n", epiar_version);
	fprintf(stdout, "http://www.epiar.net/\n");
	fprintf(stdout, "\nPlease report all bugs at http://bugs.epiar.net/\n\n");

	setup_video(screen_width, screen_height, bpp, fullscreen);

	init_colors(); /* basically sets up common Uint32s to avoid calls to SDL_MapRGB() */

	/* load the main archive file (used throughout epiar) */
	if ((epiar_eaf = eaf_open_file(apply_game_path("epiar.eaf"))) == NULL)
		printf("Couldn't open epiar.eaf file.\n");
	if ((main_eaf = eaf_open_file(apply_game_path("main.eaf"))) == NULL)
		printf("Couldn't open epiar.eaf file.\n");

	init_audio();
	init_music();

	SDL_ShowCursor(0);

	sprintf(wm_caption, "Epiar [%s]", epiar_version);

	SDL_WM_SetCaption(wm_caption, wm_caption);

	load_input_cfg();

	init_trig();
	init_playlist();

	gui_init();

	init_plugins();

	srand(time(NULL));

	return (0);
}

void de_init(void) {
	extern Uint32 average_loop_time;

	cleanup_video();
	cleanup_music();
	cleanup_audio();
	uninit_plugins();

	gui_quit();

	/* close the main eaf archive file */
	if (eaf_close_file(epiar_eaf) != 0) {
		printf("Couldn't close epiar.eaf file.\n");
	} else {
		epiar_eaf = NULL;
	}
	if (eaf_close_file(main_eaf) != 0) {
		printf("Couldn't close main.eaf file.\n");
	} else {
		main_eaf = NULL;
	}

	assert(game_path != NULL);
	free(game_path);
	game_path = NULL;

	if (average_loop_time == 0) average_loop_time = 18; /* in case they quit on menu */
#ifndef NDEBUG
	fprintf(stdout, "Average fps: %f\n", average_session_fps);
#endif
}

int init_subsystems(void) {

  init_hud();
  init_flares();
  init_starfield();
  init_fire();
  init_particles();
  init_chunks();
  init_video_new_game();
  reset_update();
  reset_plugins();
  init_asteroids();

  return (0);
}

int uninit_subsystems(void) {

  uninit_hud();
  uninit_flares();
  uninit_starfield();
  uninit_fire();
  uninit_particles();
  uninit_chunks();
  uninit_asteroids();

  return (0);
}
