#ifndef HUD_H
#define HUD_H

#include "includes.h"

extern unsigned char show_fps;

int init_hud(void);
void draw_hud(unsigned char ignore_redraw); /* ignore_redraw should be 1 on the first time the hud is drawn */
void erase_hud(unsigned char ignore_redraw);
void hud_message(char *message, Uint32 length);
void uninit_hud(void);
void update_hud(void); /* allows hud to update variables for drawing/erasing */

#endif
