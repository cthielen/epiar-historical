#include "includes.h"

typedef enum {RADIATING} f_type;

void init_force(void);
void deinit_force(void);
void update_force(void);
void new_force(f_type type, int x, int y, float strength, short int dist, Uint32 lifetime);
