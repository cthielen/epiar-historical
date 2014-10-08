/* The functions in this file were borrowed from SDL_gfx-2.0, which can (as of the time of this writing) be found at http://www.ferzkopp.net/Software/SDL_gfx-2.0/index.html. The functions in this file are licensed under the file in LICENSE.ZOOM. The rest of the source materials and data are licensed under the license found in the LICENSE file. LICENSE.ZOOM only applies to this file and it's header, src/system/video/zoom.h. Big thanks to the guys who wrote SDL_gfx-2.0 and all its contributors. */

#include "includes.h"
#include "system/video/zoom.h"

static void zoomSurfaceSize(int width, int height, double zoomx, double zoomy, int *dstwidth, int *dstheight);
static int zoomSurfaceY(SDL_Surface * src, SDL_Surface * dst);
static int zoomSurfaceRGBA(SDL_Surface * src, SDL_Surface * dst, int smooth);

typedef struct tColorRGBA {
  Uint8 r;
  Uint8 g;
  Uint8 b;
  Uint8 a;
} tColorRGBA;

SDL_Surface *zoomSurface(SDL_Surface * src, double zoomx, double zoomy, int smooth) {
  SDL_Surface *rz_src;
  SDL_Surface *rz_dst;
  int dstwidth, dstheight;
  int is32bit;
  int i, src_converted;
  
  /*
   * Sanity check 
   */
  if (src == NULL)
    return (NULL);
  
  /*
   * Determine if source surface is 32bit or 8bit 
   */
  is32bit = (src->format->BitsPerPixel == 32);
  if ((is32bit) || (src->format->BitsPerPixel == 8)) {
    /*
     * Use source surface 'as is' 
     */
    rz_src = src;
    src_converted = 0;
  } else {
    /*
     * New source surface is 32bit with a defined RGBA ordering 
     */
    rz_src =
      SDL_CreateRGBSurface(SDL_SWSURFACE, src->w, src->h, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
    SDL_BlitSurface(src, NULL, rz_src, NULL);
    src_converted = 1;
    is32bit = 1;
  }
  
  /* Get size if target */
  zoomSurfaceSize(rz_src->w, rz_src->h, zoomx, zoomy, &dstwidth, &dstheight);
  
  /*
   * Alloc space to completely contain the zoomed surface 
   */
  rz_dst = NULL;
  if (is32bit) {
    /*
     * Target surface is 32bit with source RGBA/ABGR ordering 
     */
    rz_dst =
      SDL_CreateRGBSurface(SDL_SWSURFACE, dstwidth, dstheight, 32,
			   rz_src->format->Rmask, rz_src->format->Gmask,
			   rz_src->format->Bmask, rz_src->format->Amask);
  } else {
    /*
     * Target surface is 8bit 
     */
    rz_dst = SDL_CreateRGBSurface(SDL_SWSURFACE, dstwidth, dstheight, 8, 0, 0, 0, 0);
  }
  
  /*
   * Lock source surface 
   */
  SDL_LockSurface(rz_src);
  /*
   * Check which kind of surface we have 
   */
  if (is32bit) {
    /*
     * Call the 32bit transformation routine to do the zooming (using alpha) 
     */
    zoomSurfaceRGBA(rz_src, rz_dst, smooth);
    /*
     * Turn on source-alpha support 
     */
    SDL_SetAlpha(rz_dst, SDL_SRCALPHA, 255);
  } else {
    /*
     * Copy palette and colorkey info 
     */
    for (i = 0; i < rz_src->format->palette->ncolors; i++) {
      rz_dst->format->palette->colors[i] = rz_src->format->palette->colors[i];
    }
    rz_dst->format->palette->ncolors = rz_src->format->palette->ncolors;
    /*
     * Call the 8bit transformation routine to do the zooming 
     */
    zoomSurfaceY(rz_src, rz_dst);
    SDL_SetColorKey(rz_dst, SDL_SRCCOLORKEY | SDL_RLEACCEL, rz_src->format->colorkey);
  }
  /*
   * Unlock source surface 
   */
  SDL_UnlockSurface(rz_src);
  
  /*
   * Cleanup temp surface 
   */
  if (src_converted) {
    SDL_FreeSurface(rz_src);
  }
  
  /*
   * Return destination surface 
   */
  return (rz_dst);
}

#define VALUE_LIMIT	0.001

static void zoomSurfaceSize(int width, int height, double zoomx, double zoomy, int *dstwidth, int *dstheight)
{
  /*
   * Sanity check zoom factors 
   */
  if (zoomx < VALUE_LIMIT) {
    zoomx = VALUE_LIMIT;
  }
  if (zoomy < VALUE_LIMIT) {
    zoomy = VALUE_LIMIT;
  }
  
  /*
   * Calculate target size 
   */
  *dstwidth = (int) ((double) width * zoomx);
  *dstheight = (int) ((double) height * zoomy);
  if (*dstwidth < 1) {
    *dstwidth = 1;
  }
  if (*dstheight < 1) {
    *dstheight = 1;
  }
}

static int zoomSurfaceRGBA(SDL_Surface * src, SDL_Surface * dst, int smooth)
{
  int x, y, sx, sy, *sax, *say, *csax, *csay, csx, csy, ex, ey, t1, t2, sstep;
  tColorRGBA *c00, *c01, *c10, *c11;
  tColorRGBA *sp, *csp, *dp;
  int sgap, dgap;
  
  /*
   * Variable setup 
   */
  if (smooth) {
    /*
     * For interpolation: assume source dimension is one pixel 
     */
    /*
     * smaller to avoid overflow on right and bottom edge.     
     */
    sx = (int) (65536.0 * (float) (src->w - 1) / (float) dst->w);
    sy = (int) (65536.0 * (float) (src->h - 1) / (float) dst->h);
  } else {
    sx = (int) (65536.0 * (float) src->w / (float) dst->w);
    sy = (int) (65536.0 * (float) src->h / (float) dst->h);
  }
  
  /*
   * Allocate memory for row increments 
   */
  if ((sax = (int *) malloc((dst->w + 1) * sizeof(Uint32))) == NULL) {
    return (-1);
  }
  if ((say = (int *) malloc((dst->h + 1) * sizeof(Uint32))) == NULL) {
    free(sax);
    return (-1);
  }
  
  /*
   * Precalculate row increments 
   */
  csx = 0;
  csax = sax;
  for (x = 0; x <= dst->w; x++) {
    *csax = csx;
    csax++;
    csx &= 0xffff;
    csx += sx;
  }
  csy = 0;
  csay = say;
  for (y = 0; y <= dst->h; y++) {
    *csay = csy;
    csay++;
    csy &= 0xffff;
    csy += sy;
  }
  
  /*
   * Pointer setup 
   */
  sp = csp = (tColorRGBA *) src->pixels;
  dp = (tColorRGBA *) dst->pixels;
  sgap = src->pitch - src->w * 4;
  dgap = dst->pitch - dst->w * 4;
  
  /*
   * Switch between interpolating and non-interpolating code 
   */
  if (smooth) {
    
    /*
     * Interpolating Zoom 
     */
    
    /*
     * Scan destination 
     */
    csay = say;
    for (y = 0; y < dst->h; y++) {
      /*
       * Setup color source pointers 
       */
      c00 = csp;
      c01 = csp;
      c01++;
      c10 = (tColorRGBA *) ((Uint8 *) csp + src->pitch);
      c11 = c10;
      c11++;
      csax = sax;
      for (x = 0; x < dst->w; x++) {
	
	/*
	 * Interpolate colors 
	 */
	ex = (*csax & 0xffff);
	ey = (*csay & 0xffff);
	t1 = ((((c01->r - c00->r) * ex) >> 16) + c00->r) & 0xff;
	t2 = ((((c11->r - c10->r) * ex) >> 16) + c10->r) & 0xff;
	dp->r = (((t2 - t1) * ey) >> 16) + t1;
	t1 = ((((c01->g - c00->g) * ex) >> 16) + c00->g) & 0xff;
	t2 = ((((c11->g - c10->g) * ex) >> 16) + c10->g) & 0xff;
	dp->g = (((t2 - t1) * ey) >> 16) + t1;
	t1 = ((((c01->b - c00->b) * ex) >> 16) + c00->b) & 0xff;
	t2 = ((((c11->b - c10->b) * ex) >> 16) + c10->b) & 0xff;
	dp->b = (((t2 - t1) * ey) >> 16) + t1;
	t1 = ((((c01->a - c00->a) * ex) >> 16) + c00->a) & 0xff;
	t2 = ((((c11->a - c10->a) * ex) >> 16) + c10->a) & 0xff;
	dp->a = (((t2 - t1) * ey) >> 16) + t1;
	
	/*
	 * Advance source pointers 
	 */
	csax++;
	sstep = (*csax >> 16);
	c00 += sstep;
	c01 += sstep;
	c10 += sstep;
	c11 += sstep;
	/*
	 * Advance destination pointer 
	 */
	dp++;
      }
      /*
       * Advance source pointer 
       */
      csay++;
      csp = (tColorRGBA *) ((Uint8 *) csp + (*csay >> 16) * src->pitch);
      /*
       * Advance destination pointers 
       */
      dp = (tColorRGBA *) ((Uint8 *) dp + dgap);
    }
    
  } else {
    
    /*
     * Non-Interpolating Zoom 
     */
    
    csay = say;
    for (y = 0; y < dst->h; y++) {
      sp = csp;
      csax = sax;
      for (x = 0; x < dst->w; x++) {
	/*
	 * Draw 
	 */
	*dp = *sp;
	/*
	 * Advance source pointers 
	 */
	csax++;
	sp += (*csax >> 16);
	/*
	 * Advance destination pointer 
	 */
	dp++;
      }
      /*
       * Advance source pointer 
       */
      csay++;
      csp = (tColorRGBA *) ((Uint8 *) csp + (*csay >> 16) * src->pitch);
      /*
       * Advance destination pointers 
       */
      dp = (tColorRGBA *) ((Uint8 *) dp + dgap);
    }
    
  }
  
  /*
   * Remove temp arrays 
   */
  free(sax);
  free(say);
  
  return (0);
}

/* 
   
8bit Zoomer without smoothing.

Zoomes 8bit palette/Y 'src' surface to 'dst' surface.

*/

static int zoomSurfaceY(SDL_Surface * src, SDL_Surface * dst)
{
  Uint32 x, y, sx, sy, *sax, *say, *csax, *csay, csx, csy;
  Uint8 *sp, *dp, *csp;
  int dgap;
  
  /*
   * Variable setup 
   */
  sx = (Uint32) (65536.0 * (float) src->w / (float) dst->w);
  sy = (Uint32) (65536.0 * (float) src->h / (float) dst->h);
  
  /*
   * Allocate memory for row increments 
   */
  if ((sax = (Uint32 *) malloc(dst->w * sizeof(Uint32))) == NULL) {
    return (-1);
  }
  if ((say = (Uint32 *) malloc(dst->h * sizeof(Uint32))) == NULL) {
    if (sax != NULL) {
      free(sax);
    }
    return (-1);
  }
  
  /*
   * Precalculate row increments 
   */
  csx = 0;
  csax = sax;
  for (x = 0; x < dst->w; x++) {
    csx += sx;
    *csax = (csx >> 16);
    csx &= 0xffff;
    csax++;
  }
  csy = 0;
  csay = say;
  for (y = 0; y < dst->h; y++) {
    csy += sy;
    *csay = (csy >> 16);
    csy &= 0xffff;
    csay++;
  }
  
  csx = 0;
  csax = sax;
  for (x = 0; x < dst->w; x++) {
    csx += (*csax);
    csax++;
  }
  csy = 0;
  csay = say;
  for (y = 0; y < dst->h; y++) {
    csy += (*csay);
    csay++;
  }
  
  /*
   * Pointer setup 
   */
  sp = csp = (Uint8 *) src->pixels;
  dp = (Uint8 *) dst->pixels;
  dgap = dst->pitch - dst->w;
  
  /*
   * Draw 
   */
  csay = say;
  for (y = 0; y < dst->h; y++) {
    csax = sax;
    sp = csp;
    for (x = 0; x < dst->w; x++) {
      /*
       * Draw 
       */
      *dp = *sp;
      /*
       * Advance source pointers 
       */
      sp += (*csax);
      csax++;
      /*
       * Advance destination pointer 
       */
      dp++;
    }
    /*
     * Advance source pointer (for row) 
     */
    csp += ((*csay) * src->pitch);
    csay++;
    /*
     * Advance destination pointers 
     */
    dp += dgap;
  }
  
  /*
   * Remove temp arrays 
   */
  free(sax);
  free(say);
  
  return (0);
}
