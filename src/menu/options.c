#include "game/update.h"
#include "gui/gui.h"
#include "hud/hud.h"
#include "input/input.h"
#include "menu/menu.h"
#include "menu/options.h"
#include "system/video/video.h"

unsigned char game_running = 0;

/* main session */
static gui_session *config_session = NULL;

/* detail tab variables */
static gui_label *ai_desc_lbl, *star_lbl, *traffic_lbl, *hud_lbl, *video_lbl, *fullscreen_lbl, *show_fps_lbl;
static gui_text_entry *ai_dist_entry, *star_entry, *traffic_entry;
static gui_checkbox *hud_cb, *fullscreen_cb, *show_fps_cb;

/* keys tab variables */
static gui_keybox *gkb_rotate_left, *gkb_rotate_right, *gkb_thrust, *gkb_booster, *gkb_fire, *gkb_alt_fire, *gkb_screenshot, *gkb_select, *gkb_options, *gkb_land, *gkb_status, *gkb_nav, *gkb_hail, *gkb_target, *gkb_near_target, *gkb_board, *gkb_pri_next, *gkb_pri_last, *gkb_sec_next, *gkb_sec_last;
static gui_label *gkb_desc_lbl;

/* audio tab variables */
static gui_label *audio_lbl;

/* file tab variables */
static gui_label *files_lbl;

/* plugins tab variables */
static gui_label *plugins_lbl;

/* return value, set by button callbacks */
static unsigned char continue_playing = 1;
static unsigned char game_in_progress = 0;

static void create_keys_tab(void);
static void create_details_tab(void);
static void create_audio_tab(void);
static void create_files_tab(void);
static void create_plugins_tab(void);

/* ui callbacks */
static void tab_callback(int selected);
static void done_btn_callback(void);
static void leave_btn_callback(void);
static void fullscreen_cb_callback(void);
static void show_fps_cb_callback(void);
static void ai_dist_entry_cb(char *text);
static void star_entry_cb(char *text);

/* returns true to continue playing, false to quit game */
int options(unsigned char in_game) {
  gui_window *wnd;
  gui_frame *frm;
  gui_tab *left_tabs;
  gui_button *done_btn, *leave_btn;

  game_running = 0;

  game_in_progress = game_running = in_game; /* used to know whether to redraw starfield when value changes */

  config_session = gui_create_session();
  
  wnd = gui_create_window(225, 100, 350, 425, config_session);
  
  done_btn = gui_create_button(425, 485, 125, 25, "Done", config_session);
  gui_button_set_callback(done_btn, done_btn_callback);
  
  if (in_game) {
    leave_btn = gui_create_button(255, 485, 125, 25, "Leave", config_session);
    gui_button_set_callback(leave_btn, leave_btn_callback);
  }
  
  frm = gui_create_frame(265, 120, 290, 360, config_session);
  
  left_tabs = gui_create_tab(245, 120, "Detail\nKeys\nAudio\nFile\nPlugins\n", 0, config_session);
  gui_tab_set_callback(left_tabs, tab_callback);
  
  create_details_tab();
  create_keys_tab();
  create_audio_tab();
  create_files_tab();
  create_plugins_tab();
  
  gui_session_activate(config_session);
  
  gui_session_show_all(config_session);
  
  gui_main(config_session);
  
  gui_session_destroy_all(config_session);
  gui_destroy_session(config_session);
  
  config_session = NULL;
  
  return (continue_playing);
}

static void tab_callback(int selected) {
  if (selected == 0) {
    /* turn on details tab */
    ai_desc_lbl->visible = 1;
    ai_dist_entry->visible = 1;
    star_entry->visible = 1;
    star_lbl->visible = 1;
    hud_cb->visible = 1;
    hud_lbl->visible = 1;
    traffic_entry->visible = 1;
    traffic_lbl->visible = 1;
    video_lbl->visible = 1;
    fullscreen_cb->visible = 1;
    fullscreen_lbl->visible = 1;
    show_fps_cb->visible = 1;
    show_fps_lbl->visible = 1;
  } else {
    /* turn off details tab */
    ai_desc_lbl->visible = 0;
    ai_dist_entry->visible = 0;
    star_entry->visible = 0;
    star_lbl->visible = 0;
    hud_cb->visible = 0;
    hud_lbl->visible = 0;
    traffic_entry->visible = 0;
    traffic_lbl->visible = 0;
    video_lbl->visible = 0;
    fullscreen_cb->visible = 0;
    fullscreen_lbl->visible = 0;
    show_fps_cb->visible = 0;
    show_fps_lbl->visible = 0;
  }

  if (selected == 1) {
    /* turn on keys tab */
    gkb_desc_lbl->visible = 1;
    gkb_rotate_left->visible = 1;
    gkb_rotate_left->label->visible = 1;
    gkb_rotate_right->visible = 1;
    gkb_rotate_right->label->visible = 1;
    gkb_thrust->visible = 1;
    gkb_thrust->label->visible = 1;
    gkb_booster->visible = 1;
    gkb_booster->label->visible = 1;
    gkb_fire->visible = 1;
    gkb_fire->label->visible = 1;
    gkb_alt_fire->visible = 1;
    gkb_alt_fire->label->visible = 1;
    gkb_screenshot->visible = 1;
    gkb_screenshot->label->visible = 1;
    gkb_select->visible = 1;
    gkb_select->label->visible = 1;
    gkb_options->visible = 1;
    gkb_options->label->visible = 1;
    gkb_land->visible = 1;
    gkb_land->label->visible = 1;
    gkb_status->visible = 1;
    gkb_status->label->visible = 1;
    gkb_nav->visible = 1;
    gkb_nav->label->visible = 1;
    gkb_hail->visible = 1;
    gkb_hail->label->visible = 1;
    gkb_target->visible = 1;
    gkb_target->label->visible = 1;
    gkb_near_target->visible = 1;
    gkb_near_target->label->visible = 1;
    gkb_board->visible = 1;
    gkb_board->label->visible = 1;
    gkb_pri_next->visible = 1;
    gkb_pri_next->label->visible = 1;
    gkb_pri_last->visible = 1;
    gkb_pri_last->label->visible = 1;
    gkb_sec_next->visible = 1;
    gkb_sec_next->label->visible = 1;
    gkb_sec_last->visible = 1;
    gkb_sec_last->label->visible = 1;
  } else {
    /* turn off keys tab */
    gkb_desc_lbl->visible = 0;
    gkb_rotate_left->visible = 0;
    gkb_rotate_left->label->visible = 0;
    gkb_rotate_right->visible = 0;
    gkb_rotate_right->label->visible = 0;
    gkb_thrust->visible = 0;
    gkb_thrust->label->visible = 0;
    gkb_booster->visible = 0;
    gkb_booster->label->visible = 0;
    gkb_fire->visible = 0;
    gkb_fire->label->visible = 0;
    gkb_alt_fire->visible = 0;
    gkb_alt_fire->label->visible = 0;
    gkb_screenshot->visible = 0;
    gkb_screenshot->label->visible = 0;
    gkb_select->visible = 0;
    gkb_select->label->visible = 0;
    gkb_options->visible = 0;
    gkb_options->label->visible = 0;
    gkb_land->visible = 0;
    gkb_land->label->visible = 0;
    gkb_status->visible = 0;
    gkb_status->label->visible = 0;
    gkb_nav->visible = 0;
    gkb_nav->label->visible = 0;
    gkb_hail->visible = 0;
    gkb_hail->label->visible = 0;
    gkb_target->visible = 0;
    gkb_target->label->visible = 0;
    gkb_near_target->visible = 0;
    gkb_near_target->label->visible = 0;
    gkb_board->visible = 0;
    gkb_board->label->visible = 0;
    gkb_pri_next->visible = 0;
    gkb_pri_next->label->visible = 0;
    gkb_pri_last->visible = 0;
    gkb_pri_last->label->visible = 0;
    gkb_sec_next->visible = 0;
    gkb_sec_next->label->visible = 0;
    gkb_sec_last->visible = 0;
    gkb_sec_last->label->visible = 0;
  }
  /* turn on/off audio tab */
  if (selected == 2)
    audio_lbl->visible = 1;
  else
    audio_lbl->visible = 0;
  /* turn on/off files tab */
  if (selected == 3)
    files_lbl->visible = 1;
  else
    files_lbl->visible = 0;
  /* turn on/off plugins tab */
  if (selected == 4)
    plugins_lbl->visible = 1;
  else
    plugins_lbl->visible = 0;

  gui_session_show_all(config_session);
}

/* creates keys tab (disabled by default) */
static void create_keys_tab(void) {
	/* create the desc. label and hide it */
	gkb_desc_lbl = gui_create_label(297, 132, "Click any box to change key binding", config_session);
	gkb_desc_lbl->visible = 0;
	/* create all the key binding boxes and hide them */
	gkb_rotate_left = gui_create_keybox(285, 155, "Rotate Left", &keys.rotate_left, config_session);
	gkb_rotate_left->visible = 0;
	gkb_rotate_left->label->visible = 0;
	gkb_rotate_right = gui_create_keybox(285, 175, "Rotate Right", &keys.rotate_right, config_session);
	gkb_rotate_right->visible = 0;
	gkb_rotate_right->label->visible = 0;
	gkb_thrust = gui_create_keybox(285, 195, "Thrust", &keys.thrust, config_session);
	gkb_thrust->visible = 0;
	gkb_thrust->label->visible = 0;
	gkb_booster = gui_create_keybox(285, 215, "Booster", &keys.booster, config_session);
	gkb_booster->visible = 0;
	gkb_booster->label->visible = 0;
	gkb_fire = gui_create_keybox(285, 235, "Fire", &keys.fire, config_session);
	gkb_fire->visible = 0;
	gkb_fire->label->visible = 0;
	gkb_alt_fire = gui_create_keybox(285, 255, "Alt Fire", &keys.alt_fire, config_session);
	gkb_alt_fire->visible = 0;
	gkb_alt_fire->label->visible = 0;
	gkb_screenshot = gui_create_keybox(285, 275, "Screenshot", &keys.screenshot, config_session);
	gkb_screenshot->visible = 0;
	gkb_screenshot->label->visible = 0;
	gkb_select = gui_create_keybox(285, 295, "Select", &keys.select, config_session);
	gkb_select->visible = 0;
	gkb_select->label->visible = 0;
	gkb_options = gui_create_keybox(285, 315, "Options", &keys.options, config_session);
	gkb_options->visible = 0;
	gkb_options->label->visible = 0;
	gkb_land = gui_create_keybox(285, 335, "Land", &keys.land, config_session);
	gkb_land->visible = 0;
	gkb_land->label->visible = 0;
	gkb_status = gui_create_keybox(285, 355, "Status", &keys.status, config_session);
	gkb_status->visible = 0;
	gkb_status->label->visible = 0;
	gkb_nav = gui_create_keybox(285, 375, "Nav Map", &keys.nav, config_session);
	gkb_nav->visible = 0;
	gkb_nav->label->visible = 0;
	gkb_hail = gui_create_keybox(285, 395, "Hail", &keys.hail, config_session);
	gkb_hail->visible = 0;
	gkb_hail->label->visible = 0;
	gkb_target = gui_create_keybox(285, 415, "Target", &keys.target, config_session);
	gkb_target->visible = 0;
	gkb_target->label->visible = 0;
	gkb_near_target = gui_create_keybox(425, 155, "Nearest", &keys.near_target, config_session);
	gkb_near_target->visible = 0;
	gkb_near_target->label->visible = 0;
	gkb_board = gui_create_keybox(425, 175, "Board", &keys.board, config_session);
	gkb_board->visible = 0;
	gkb_board->label->visible = 0;
	gkb_pri_next = gui_create_keybox(425, 195, "Pri Next", &keys.pri_next, config_session);
	gkb_pri_next->visible = 0;
	gkb_pri_next->label->visible = 0;
	gkb_pri_last = gui_create_keybox(425, 215, "Pri Last", &keys.pri_last, config_session);
	gkb_pri_last->visible = 0;
	gkb_pri_last->label->visible = 0;
	gkb_sec_next = gui_create_keybox(425, 235, "Sec Next", &keys.sec_next, config_session);
	gkb_sec_next->visible = 0;
	gkb_sec_next->label->visible = 0;
	gkb_sec_last = gui_create_keybox(425, 255, "Sec Last", &keys.sec_last, config_session);
	gkb_sec_last->visible = 0;
	gkb_sec_last->label->visible = 0;
}

/* creates details tab (showing and enabled by default) */
static void create_details_tab(void) {
  char temp[25] = {0};

  /* create the detail tab (& keep it initially visible) */
  ai_desc_lbl = gui_create_label(285, 132, "A.I. Distance Updates (in milliseconds)", config_session);
  
  /* ai dist */
  ai_dist_entry = gui_create_text_entry(285, 152, 250, 24, 4, 1, config_session);
  sprintf(temp, "%d", sort_time);
  gui_text_entry_set_focus(config_session, ai_dist_entry);
  gui_text_entry_set_text(ai_dist_entry, temp);
  gui_text_entry_set_callback(ai_dist_entry, ai_dist_entry_cb);
  
  /* star density */
  star_entry = gui_create_text_entry(285, 200, 250, 24, 3, 1, config_session);
  memset(temp, 0, sizeof(char) * 25);
  sprintf(temp, "%d", num_stars);
  gui_text_entry_set_text(star_entry, temp);
  star_lbl = gui_create_label(285, 180, "Starfield density (25-999)", config_session);
  gui_text_entry_set_callback(star_entry, star_entry_cb);

  /* average sector traffic */
  traffic_entry = gui_create_text_entry(285, 248, 250, 24, 1, 1, config_session);
  gui_text_entry_set_text(traffic_entry, "4");
  traffic_lbl = gui_create_label(285, 228, "Average Sector Traffic", config_session);
  
  /* multi-line hud */
  hud_cb = gui_create_checkbox(285, 285, 1, config_session);
  hud_lbl = gui_create_label(303, 282, "Use multi-lined HUD", config_session);
  
  /* video options label */
  video_lbl = gui_create_label(285, 315, "Video Options:", config_session);
  
  /* fullscreen */
  fullscreen_cb = gui_create_checkbox(285, 338, fullscreen, config_session);
  gui_checkbox_set_callback(fullscreen_cb, fullscreen_cb_callback);
  fullscreen_lbl = gui_create_label(303, 335, "Fullscreen", config_session);

  /* show fps */
  show_fps_cb = gui_create_checkbox(285, 360, show_fps, config_session);
  show_fps_lbl = gui_create_label(303, 357, "Show FPS", config_session);
  gui_checkbox_set_callback(show_fps_cb, show_fps_cb_callback);
}

static void create_audio_tab(void) {
  audio_lbl = gui_create_label(310, 270, "Audio Not Supported in this Version", config_session);
  audio_lbl->visible = 0;
}

static void create_files_tab(void) {
  files_lbl = gui_create_label(292, 270, "Saved Games Not Supported in this Version", config_session);
  files_lbl->visible = 0;
}

static void create_plugins_tab(void) {
  plugins_lbl = gui_create_label(308, 270, "Plugins API Incomplete in this Version", config_session);
  plugins_lbl->visible = 0;
}

static void done_btn_callback(void) {
  continue_playing = 1;
  config_session->active = 0;
}

static void leave_btn_callback(void) {
  continue_playing = 0;
  config_session->active = 0;
}

static void fullscreen_cb_callback(void) {
  if (!SDL_WM_ToggleFullScreen(screen)) {
		/* not in X11 or under BeOS, so this isnt supported */
	  if (!fullscreen)
		screen = SDL_SetVideoMode(800, 600, screen->format->BitsPerPixel, SDL_FULLSCREEN);
	  else
		  screen = SDL_SetVideoMode(800, 600, screen->format->BitsPerPixel, 0);

	  if (game_running) {
		  draw_frame(1);
	  } else {
		  draw_background();
		  draw_menu_text();
		  menu_draw_frame();
	  }
	  gui_session_show_all(config_session);
  }

  /* toggle it */
  fullscreen = (unsigned char)(((int)fullscreen + 1) % 2);
}

static void show_fps_cb_callback(void) {
  /* toggle showing fps */
  if (show_fps)
    show_fps = 0;
  else
    show_fps = 1;
}

static void ai_dist_entry_cb(char *text) {
  int dist;

  if (!text)
    return;
  if (strlen(text) <= 0)
    return;

  dist = atoi(text);

  if (dist >= 150)
    sort_time = dist; /* sort_time is from update.h */
}

static void star_entry_cb(char *text) {
  int stars;

  if (!text)
    return;
  if (strlen(text) <= 0)
    return;

  stars = atoi(text);

  if (stars >= 25) {
    if (game_in_progress)
      erase_starfield();
    uninit_starfield();
    num_stars = stars;
    init_starfield();
    if (game_in_progress)
      draw_starfield();
    gui_session_show_all(config_session);
  }
}
