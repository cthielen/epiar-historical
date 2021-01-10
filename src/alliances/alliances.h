#ifndef H_ALLIANCE
#define H_ALLIANCE

#include "includes.h"
#include "sprite/model.h"

#define MAX_ALLIANCES 10

struct _alliance {
	char *name;
	char *logo;
	short int aggression; /* 0 - peaceful, 10 - warring */
	model_t **used_models; /* types the alliance uses - needed for ship generation and battles */
	int num_used_models;
};

extern struct _alliance alliances[MAX_ALLIANCES];

extern int num_alliances;

int load_alliances_eaf(FILE *eaf, char *filename);
void unload_alliances(void);
SDL_Surface *get_alliance_logo(char *alliance);
struct _alliance *get_alliance_pointer(char *alliance);
void alliance_ship_damaged(struct _alliance *alliance);

#endif /* H_ALLIANCE */
