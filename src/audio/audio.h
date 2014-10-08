#include "includes.h"

#ifndef NAUDIO
ALuint fire_source[7];
#endif

unsigned char audio_enabled;

int init_audio(void);
void cleanup_audio(void);
void update_audio(void);
void toggle_mute(void);
void resume_audio(void); /* continues playing normal audio unless muted */
void stop_audio(void);
#ifndef NAUDIO
ALuint *load_wave(const char *filename);
#endif
void audio_options_menu(void);
#ifndef NAUDIO
ALuint *load_sound(char *filename);
#endif
