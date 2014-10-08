#include "includes.h"

#define NUM_FLARES 15
#define NUM_FRAMES_PER_FLARE 6
#define MS_FRAME 5 /* how long a shield flare frame is on the screen before the next frame is shown (in ms) */
#define FLARE_D_COLOR (255.0 / (float)NUM_FRAMES_PER_FLARE)

struct _flare {
	Uint32 last_time; /* last time the frame was updated */
	unsigned char on;
	unsigned char frame_switch; /* used to know whether to increase or decrease the frames */
	int frame;
	struct _ship *ship;
	int start_angle, end_angle;
};

struct _flare flares[NUM_FLARES];

void init_flares(void);
void uninit_flares(void);
void update_flares(void);
void new_flare(struct _ship *who, int hit_angle);
void draw_flares(void);
void erase_flares(void);
