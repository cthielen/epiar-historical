#include "game/update.h"
#include "network/net_sprite.h"
#include "network/network.h"
#include "sprite/model.h"
#include "system/video/video.h"

struct _net_ship net_ships[MAX_NET_SHIPS];

static int find_free_net_ship_slot(void);

void init_net_ships(void) {

}

/* creates a new net ship in the given slot and overrides one if there's one there already */
void new_net_ship(int slot, int type, int x, int y, short int angle, char *callsign) {

}

static int find_free_net_ship_slot(void) {

}

void update_net_ships(void) {

}

void draw_net_ships(void) {

}

void erase_net_ships(void) {

}

/* checks to see if the net ship is already known to this client. returns 0 if ship is known already */
int is_ship_known(int slot) {

	return (-1);
}
