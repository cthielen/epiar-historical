#ifndef H_TARGET
#define H_TARGET

#include "sprite/gate.h"
#include "sprite/sprite.h"

#define MAX_TARGETS MAX_GATES + MAX_SHIPS

/* holds information for target cycling */
struct _targets {
  enum {TARGET_NONE, TARGET_SHIP, TARGET_GATE} type;
  void *target;
  float dist;
};

extern struct _targets targets[MAX_TARGETS];

void update_targets(void); /* recontructs list of 15 closest targets */
void reset_target_cycle(void);
void cycle_targets(void);
void set_target_nearest(void);

#endif
