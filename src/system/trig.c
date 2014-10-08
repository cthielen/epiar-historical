#include "includes.h"
#include "system/trig.h"

#define PI 3.141592653589793
#define deg_to_rad(x) x*PI/180.0

struct _fire_cos {
	float value;
};

static struct _fire_cos fire_cos[361];

struct _fire_neg_sin {
	float value;
};

static struct _fire_neg_sin fire_neg_sin[361];

struct _fire_sin {
	float value;
};

static struct _fire_sin fire_sin[361];

void init_trig(void) {
	int i;

	for (i = 0; i < 361; i++) {
		fire_cos[i].value = cos(deg_to_rad((float)i));
		fire_neg_sin[i].value = -sin(deg_to_rad((float)i));
		fire_sin[i].value = sin(deg_to_rad((float)i));
	}
}

#ifdef WIN32
float get_cos(int angle) {
#else
inline float get_cos(int angle) {
#endif
	return (fire_cos[angle].value);
}

#ifdef WIN32
float get_neg_sin(int angle) {
#else
inline float get_neg_sin(int angle) {
#endif
	return (fire_neg_sin[angle].value);
}

#ifdef WIN32
float get_sin(int angle) {
#else
inline float get_sin(int angle) {
#endif
	return (fire_sin[angle].value);
}

void rotate_point(int x, int y, int angle, int *rot_x, int *rot_y) {
	float converted_angle;
	converted_angle = deg_to_rad((float)angle);

	*rot_x = (int)((float)x * (float)cos(converted_angle)) - ((float)y * (float)sin(converted_angle));
	*rot_y = (int)((float)x * (float)sin(converted_angle)) + ((float)y * (float)cos(converted_angle));
}

float get_opposite_angle(float angle) {
	float opp;
	opp = angle + 180.0;
	if (opp > 360.0) opp = opp - 360.0;

	return (opp);
}
