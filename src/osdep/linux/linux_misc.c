#include "osdep/linux/linux_misc.h"

inline int bounds_okay(int x, int y) {
	if (x >= 0)
		if (y >= 0)
			if (x < 800)
				if (y < 600)
					return (1);

	return (0);
}
