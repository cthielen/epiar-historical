#include "includes.h"
#include "system/timer.h"

static Uint32 delay = 0;
static Uint32 pause_start_time = 1;

Uint32 get_ticks(void) {
  if (pause_start_time) {
    /* printf("Trying to get ticks while timer paused, reutnring 0\n");
       assert(0); */
    return (0);
  }
  
  return (SDL_GetTicks() - delay);
}

void pause_timer(void) {
  pause_start_time = SDL_GetTicks();
}

void unpause_timer(void) {
  if (pause_start_time) {
    delay += SDL_GetTicks() - pause_start_time;
    pause_start_time = 0;
  }
}

/* clears the "delay" value - I don't think this is necessary but it makes sense to keep the delay value per-scenario */
void reset_timer(void) {
  delay = 0;
}
