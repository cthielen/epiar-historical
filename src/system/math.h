#ifndef H_MATH
#define H_MATH

#include "includes.h"

float get_distance(float x1, float y1, float x2, float y2);
float get_distance_sqrd(float x1, float y1, float x2, float y2);
float get_angle_to(float x1, float y1, float x2, float y2); /* perspective is from the first set of coords */
int line_intersect(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, float *x_inter, float *y_inter);

#endif /* H_MATH */
