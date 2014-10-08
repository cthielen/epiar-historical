#include "osdep/win32/win32_misc.h"
#include "system/video/backbuffer.h"

/* sideeffect: dirties the pixel if it's okay */
int bounds_okay(int x, int y) {
	extern int scr_top, scr_left, scr_bottom, scr_right;
	if (x >= scr_left) {
		if (y >= scr_top)
			if (x < scr_right)
				if (y < scr_bottom) {
					dirty_pixel(x, y);
					return (1);
				}
	}

	return (0);
}
