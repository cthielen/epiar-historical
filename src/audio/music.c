#include "audio/audio.h"
#include "audio/music.h"
#include "audio/playlist.h"
#include "includes.h"
#include "system/debug.h"
#include "system/path.h"

#define MUSIC_BUF_SIZE 11000 /* this value could be adjusted for various computers */

unsigned char music_enabled = 0;
unsigned char music_playing = 0;

#ifndef NAUDIO
/* OpenAL music info */
static ALuint music_source = 0;
static ALuint music_buffer = 0;

/* Vorbis info */
static OggVorbis_File music_file;
static vorbis_info *music_info = NULL;
static int music_section = -1;
static unsigned char music_file_loaded = 0;

static ALshort buf[MUSIC_BUF_SIZE];
static int buf_count = 0;
static int buf_pos = -1;
#endif /* NAUDIO */

/*
SDL_Thread *music_update_thread = NULL;
*/

int update_music_thread(void *arg) {
	/*
#ifdef LINUX
	arg += 0;
#endif

	while(1) {
		update_music();
		SDL_Delay(35);
	}
	*/

	return (0);
}

void init_music(void) {
#ifndef NAUDIO
	/*
	alGenStreamingBuffers_LOKI(1, &music_buffer);
	alGenSources(1, &music_source);
	alSourcei(music_source, AL_SOURCE_RELATIVE, AL_TRUE);
	alSourcei(music_source, AL_BUFFER, music_buffer);
	if (alGetError() != AL_NO_ERROR) return;
	music_update_thread = SDL_CreateThread(update_music_thread, NULL);
	if (music_update_thread == NULL)
		fprintf(stdout, "Could not create music update thread.\n");
	music_enabled = 1;
	*/
#endif
}

void cleanup_music(void) {
#ifndef NAUDIO
	/*
	if (music_enabled) {
		alSourceStop(music_source);
		alDeleteBuffers(1, &music_buffer);
		alDeleteSources(1, &music_source);
		if (music_file_loaded) {
			ov_clear(&music_file);
			music_file_loaded = 0;
		}
		if (music_update_thread != NULL) {
			SDL_KillThread(music_update_thread);
			music_update_thread = NULL;
		}
		music_enabled = 0;
	}
	*/
#endif
}

void load_music(char *filename) {
#ifndef NAUDIO
	/*
	FILE *fp;

	if (music_file_loaded) {
		ov_clear(&music_file);
		music_file_loaded = 0;
	}

	fp = fopen(apply_game_path(filename), "r");
	if (fp == NULL) {
		fprintf(stdout, "Could not open music file: %s\n", filename);
		return;
	}

	if (ov_open(fp, &music_file, NULL, 0) < 0) {
		debug_message("Couldn't attach to libvorbis.\n");
		fclose(fp);
		return;
	}

	music_info = ov_info(&music_file, -1);
	music_file_loaded = 1;
	*/
#endif
}

void start_music(void) {
#ifndef NAUDIO
	/*
	if (music_enabled && music_file_loaded) {
		alSourcePlay(music_source);
		music_playing = 1;
	}
	*/
#endif
}

void stop_music(void) {
#ifndef NAUDIO
	/*
	if (music_enabled) {
		alSourceStop(music_source);
		music_playing = 0;
	}
	*/
#endif
}
