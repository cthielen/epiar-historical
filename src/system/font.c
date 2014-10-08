#include "system/afont_sdl.h"
#include "system/eaf.h"
#include "system/font.h"

/* basic wrapper, just here for easy changing */
afont *epiar_load_fp(FILE *fp) {
	return (afont_load_fp(fp));
}

/* basic wrapper, just here for easy changing */
void epiar_free(afont *a) {
	afont_free(a);
}

/* basic wrapper, just here for easy changing */
void epiar_size_text(afont *a, char *text, int *w, int *h, int *base) {
	afont_size_text(a, text, w, h, base);
}

/* loads a font file from an eaf archive */
afont *epiar_load_font_eaf(char *file, FILE *eaf) {

  if (!eaf)
    return (NULL);

  if (eaf_find_file(eaf, file) != 0) {
    printf("Couldn't find font in EAF archive.\n");
    return (NULL);
  }

  /* skip ahead 29 bytes (past the filename and filesize information) */
  fseek(eaf, 29, SEEK_CUR);

  return (epiar_load_fp(eaf));
}

/* wrapper for afont_render_text_surf - adds a black colorkey */
SDL_Surface *epiar_render_text_surf(afont *a, Uint32 fg, Uint32 bg, afont_render_mode mode, SDL_PixelFormat *fmt, char *text) {
	SDL_Surface *s;
	Uint32 new_fg, new_bg;

	 new_fg = (((fg & fmt->Rmask) >> fmt->Rshift) << (24 + fmt->Rloss)) | (((fg & fmt->Gmask) >> fmt->Gshift) << (16 + fmt->Gloss)) | (((fg & fmt->Bmask) >> fmt->Bshift) << (8 + fmt->Bloss)) | (((fg & fmt->Amask) >> fmt->Ashift) << fmt->Aloss);
	 new_bg = (((bg & fmt->Rmask) >> fmt->Rshift) << (24 + fmt->Rloss)) | (((bg & fmt->Gmask) >> fmt->Gshift) << (16 + fmt->Gloss)) | (((bg & fmt->Bmask) >> fmt->Bshift) << (8 + fmt->Bloss)) | (((bg & fmt->Amask) >> fmt->Ashift) << fmt->Aloss);

	s = afont_render_text_surf(a, new_fg, new_bg, mode, fmt, text);

	SDL_SetColorKey(s, SDL_SRCCOLORKEY | SDL_RLEACCEL, (Uint32)SDL_MapRGB(s->format, 0, 0, 0));

	return (s);
}


/* wrapper for afont_render_text - converts sdl packed pixels to standard pixel format */
void epiar_render_text(afont *a, Uint32 fg, Uint32 bg, afont_render_mode mode, SDL_Surface *s, int x, int y, char *text) {
	Uint32 new_fg, new_bg;
	SDL_PixelFormat *fmt = s->format;

	 new_fg = (((fg & fmt->Rmask) >> fmt->Rshift) << (24 + fmt->Rloss)) | (((fg & fmt->Gmask) >> fmt->Gshift) << (16 + fmt->Gloss)) | (((fg & fmt->Bmask) >> fmt->Bshift) << (8 + fmt->Bloss)) | (((fg & fmt->Amask) >> fmt->Ashift) << fmt->Aloss);
	 new_bg = (((bg & fmt->Rmask) >> fmt->Rshift) << (24 + fmt->Rloss)) | (((bg & fmt->Gmask) >> fmt->Gshift) << (16 + fmt->Gloss)) | (((bg & fmt->Bmask) >> fmt->Bshift) << (8 + fmt->Bloss)) | (((bg & fmt->Amask) >> fmt->Ashift) << fmt->Aloss);

	afont_render_text(a, new_fg, new_bg, mode, s, x, y, text);
}
