#ifndef H_TRIG
#define H_TRIG

void init_trig(void);
#ifdef WIN32
float get_cos(int angle);
float get_sin(int angle);
float get_neg_sin(int angle);
#else
inline float get_cos(int angle);
inline float get_sin(int angle);
inline float get_neg_sin(int angle);
#endif
void rotate_point(int x, int y, int angle, int *rot_x, int *rot_y);
float get_opposite_angle(float angle);

#endif /* H_TRIG */
