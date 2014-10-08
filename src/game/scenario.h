#ifndef H_SCENARIO
#define H_SCENARIO

#include "includes.h"

enum diff_rating {NEWBIE, EASY, AVERAGE, DIFFICULT, INSANE};

typedef struct _ep_scenario {
  FILE *eaf;
  char *name;
  char *author;
  char *image;
  char *desc;
  enum diff_rating difficulty;
  char *website;
  char *filename;
  char *intro_msg;
} ep_scenario;

extern FILE *loaded_eaf;

ep_scenario *load_scenario_eaf(char *filename);
int close_scenario(ep_scenario *scen);
void do_scenario(ep_scenario *scen);
ep_scenario *do_scenario_select(void);

#endif /* H_SCENARIO */
