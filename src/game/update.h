#include "sprite/fire.h"
#include "sprite/sprite.h"

extern Uint32 sort_time; /* time in ms between ship/fire "list of which is closest" updates */
extern Uint32 current_time; /* the time for any operations in the loop to use */
extern Uint32 loop_length; /* how long a single loop took (how much time, in ms, has passed since the last update) */
extern Uint32 player_dead; /* time of death for player, needs to be false upon new scenario, if a time is set and that time is exceeded, scenario ends w/ player death */
extern int camera_x, camera_y;

void update_universe(void);
void sort_fires(void);
void update_planet_nearby_shots(struct _ship *ship);
int get_screen_coords(int x, int y, unsigned char x_coord);
int get_exact_distance(int x1, int y1, int x2, int y2);
void reset_update(void); /* resets values in update loop for a new scenario */
