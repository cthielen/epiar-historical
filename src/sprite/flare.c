#include "com_defs.h"
#include "sprite/flare.h"
#include "sprite/sprite.h"
#include "system/timer.h"
#include "system/video/video.h"

void init_flares(void) {
	int i;

	for (i = 0; i < NUM_FLARES; i++) {
		flares[i].last_time = 0;
		flares[i].on = 0;
		flares[i].ship = NULL;
	}
}

void uninit_flares(void) {
	int i;

	for (i = 0; i < NUM_FLARES; i++) {
		flares[i].last_time = 0;
		flares[i].on = 0;
		flares[i].ship = NULL;
	}
}

void new_flare(struct _ship *who, int hit_angle) {
	int i, slot = -1;

	/* avoid multiple shield flares per ship at a time */
	if (who->has_shield_flare)
		return;

	for (i = 0; i < NUM_FLARES; i++) {
		if (flares[i].on == 0) {
			slot = i;
			break;
		}
	}

	if (slot != -1) {
		int opp_angle = (hit_angle + 180) % 360;
		flares[slot].on = 1;
		flares[slot].frame = 0;
		flares[slot].ship = who;
		flares[slot].ship->has_shield_flare = 1;
		flares[slot].frame_switch = 0;
		flares[slot].start_angle = opp_angle - 35;
		if (flares[slot].start_angle < 0)
			flares[slot].start_angle = 360 + flares[slot].start_angle; /* keep it in bounds */
		flares[slot].end_angle = (opp_angle + 35) % 360; /* keep it in bounds */
	}
}

void update_flares(void) {
	int i;
	Uint32 current_time;

	current_time = get_ticks();

	for (i = 0; i < NUM_FLARES; i++) {
		if (flares[i].on) {
			if ((flares[i].last_time + MS_FRAME) < current_time) {
				if (flares[i].frame_switch == 0) {
					flares[i].frame++;
					if (flares[i].frame == NUM_FRAMES_PER_FLARE) flares[i].frame_switch = 1;
				} else {
					flares[i].frame--;
					if (flares[i].frame < 0) {
						flares[i].ship->has_shield_flare = 0;
						flares[i].ship = NULL;
						flares[i].on = 0;
					}
				}
				flares[i].last_time = current_time;
			}
		}
	}
}

/* drawArc(screen, 125, 125, 50, 0, 90, white, 0.5); */

void draw_flares(void) {
#ifndef WIN32
#warning draw_flares() for shield flares is not implemented
#endif
}

void erase_flares(void) {
#ifndef WIN32
#warning erase_flares() for shield flares is not implemented
#endif
}
