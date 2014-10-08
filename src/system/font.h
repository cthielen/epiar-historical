#ifndef H_FONT
#define H_FONT

#include "includes.h"
#include "system/afont_sdl.h"

/* wrappers for afont functions (note: apply some changes before returning/sending data to afont) */
afont *epiar_load_font_eaf(char *file, FILE *eaf);
afont *epiar_load_fp(FILE *fp);
void epiar_free(afont *a);
void epiar_size_text(afont *a, char *text, int *w, int *h, int *base);
SDL_Surface *epiar_render_text_surf(afont *a, Uint32 fg, Uint32 bg, afont_render_mode mode, SDL_PixelFormat *fmt, char *text);
void epiar_render_text(afont *a, Uint32 fg, Uint32 bg, afont_render_mode mode, SDL_Surface *s, int x, int y, char *text);

#endif /* H_FONT */
