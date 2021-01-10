#ifndef H_R_SHIPS
#define H_R_SHIPS

#include "sprite/sprite.h"

#define MAX_R_SHIPS 10
#define R_SHIPS_UPDATE 3000

extern struct _ship *r_ships[MAX_R_SHIPS];

/* note - no deinit b/c we only make and break ships - and before the system is used in another session, init_r_ships will be called and get rid of invalid referneces, so we really dont need a clean up function */
int init_r_ships(void);
void update_r_ships(unsigned char force_update);

#endif /* H_R_SHIPS */
