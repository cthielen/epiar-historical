#include "comm/comm.h"
#include "game/update.h"
#include "gui/gui.h"
#include "hud/hud.h"
#include "sprite/gate.h"
#include "sprite/player.h"
#include "sprite/sprite.h"
#include "system/math.h"

/* display the hailing list and handle it */
void hail(void) {
	char msg[100] = {0};
	static Uint32 last_message = 0;

	if (player.target.type == TARGET_SHIP) {
		struct _ship *ship = (struct _ship *)player.target.target;
		
		if (ship->name)
			sprintf(msg, "Hailing starship \"%s\"", ship->name);
		else
			sprintf(msg, "Hailing %s class starship", ship->model->name);
		
		hud_message(msg, 3000);
		draw_hud(1);
		
		hail_ship(ship);
	} else if (player.target.type == TARGET_GATE) {
		struct _gate *gate = (struct _gate *)player.target.target;
		
		sprintf(msg, "Hailing jumpgate \"%s\"", gate->name);
		
		hud_message(msg, 3000);
		draw_hud(1);
		
		hail_gate(gate);
	} else {
		if (current_time > (last_message + 3000)) {
			hud_message("No target selected to hail.", 3000);
			last_message = current_time;
		}
	}
}
