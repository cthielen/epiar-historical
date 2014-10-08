#include "includes.h"

#include "osdep/win32/win32_video.h"
#include "system/video/backbuffer.h"
#include "system/video/video.h"

void putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel) {
	int bpp = surface->format->BytesPerPixel;
	Uint8 *p = (Uint8*)(((Uint8*)surface->pixels)+(y*surface->pitch)+(x*bpp));

	if (bpp == 1)
		*p = pixel;
	else
	if (bpp == 2)
		*(Uint16*)p = pixel;
	else
	if (bpp == 3)
	{
		if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
		{
			p[0] = (pixel >> 16) & 0xff;
			p[1] = (pixel >> 8) & 0xff;
			p[2] = pixel & 0xff;
		}
		else
		{
			p[0] = pixel & 0xff;
			p[1] = (pixel >> 8) & 0xff;
			p[2] = (pixel >> 16) & 0xff;
		}
	}
	else
	if (bpp == 4)
	{
		*(Uint32*)p = pixel;
	}
}

/******************************************************************************      
*
*   Name:
*      inline Uint32 getpixel(SDL_Surface *surface, int x, int y);
*
*   Abstract:
*      Gets a pixel value off a surface at the given (x, y) coordinate.
*
*   Context/Scope:	
*      Called throughout.
*
*   Side Effects:
*      Slow. If a function is to call this a lot, it should have this "built
*   in", and shouldn't be calling this function.
*
*   Return Value:
*      Uint32 (the pixel value)
*
*   Assumptions:
*      Assumes surface is either a software surface or is locked.
*
******************************************************************************/
/* special thanks to the guys on sdl@libsdl.org for this one */
Uint32 getpixel(SDL_Surface *surface, int x, int y) {
	int bpp = surface->format->BytesPerPixel;
	Uint8 *p = (Uint8*)(((Uint8*)surface->pixels)+(y*surface->pitch)+(x*bpp));
	Uint32 pixel = 0;

	if (bpp == 1)
		pixel = *p;
	else
	if (bpp == 2)
		pixel = *(Uint16 *)p;
	else
	if (bpp == 3)
	{
		if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
			pixel = p[0] << 16 | p[1] << 8 | p[2];
		else
			pixel = p[0] | p[1] << 8 | p[2] << 16;
	}
	else
	if (bpp == 4)
		pixel = *(Uint32 *)p;

	return (pixel);
}
