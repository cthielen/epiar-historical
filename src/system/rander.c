#include "includes.h"
#include "rander.h"

float rander(float x, float y) {
	float r, factor;

	r = (y - x) + 1;
	factor = RAND_MAX / r;

	return ((rand()/factor) + x);
}
