#ifndef H_EAF
#define H_EAF

#include "includes.h"

FILE *eaf_open_file(char *file);
int eaf_close_file(FILE *eaf);
int eaf_find_file(FILE *eaf, char *file);
SDL_Surface *eaf_load_png(FILE *eaf, char *file);

#endif /* H_EAF */
