#ifndef H_PARTICLE
#define H_PARTICLE

#define NUM_PARTICLES 600

struct _particle {
	float world_x, world_y;
	short int screen_x, screen_y;
	int time_left; /* if time_left is zero, particle slot is considered "off" and "free for use" */
	Uint32 length;
	float momentum_x, momentum_y;
	int angle;
	Uint32 color;
};

struct _particle particles[NUM_PARTICLES];

void new_explosion(int world_x, int world_y, float momentum_x, float momentum_y, int density, Uint32 length);
void erase_particles(void);
void draw_particles(void);
void init_particles(void);
void uninit_particles(void);
void chunk_blast(int world_x, int world_y, int density, Uint32 length, int angle, int spread);

#endif /* H_PARTICLE */
