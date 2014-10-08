#include "game/scenario.h"
#include "includes.h"

extern Uint32 average_loop_time;
extern Uint32 game_start_time;
extern Uint32 total_play_time, total_frames_drawn;
extern float average_session_fps, current_fps;

void init_new_game(ep_scenario *scen);
void game_loop(ep_scenario *scen);
void game_over(void);
void pause_loop(int delay);
