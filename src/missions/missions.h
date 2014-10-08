/* notice there's no handle - only one mission at a time */

#include "sprite/ship.h"

int load_mission_eaf(FILE *eaf, char *filename);
void close_mission(void);
int update_mission(void); /* updates (and executes) and mission related objectives */
/* functions called from elsewhere to see if it affects any mission */
void mission_ship_destroyed(struct _ship *ship, unsigned char landed);
void mission_player_boarded(struct _ship *ship);
void mission_player_landed(struct _planet *planet);
