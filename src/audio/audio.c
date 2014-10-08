#include "audio/audio.h"
#include "game/update.h"
#include "gui/gui.h"
#include "includes.h"
#include "sprite/sprite.h"
#include "system/debug.h"
#include "system/path.h"
#include "system/trig.h"
#include "system/video/video.h"

#define DISTANCE_FACTOR 60.0

unsigned char audio_enabled = 0, muted = 0;

#ifndef NAUDIO
ALvoid *audio_context = NULL;
ALCdevice *audio_device = NULL;

ALuint ship_source[10];
ALuint explosion_source[3];
ALuint effects_source[2]; /* for random little effects like clicks and beeps */
#endif

int init_audio(void) {
#ifndef NAUDIO
	ALsizei size;
	ALsizei bits;
	ALsizei freq;
	ALsizei format;
	ALboolean err;
	ALuint weapon1, select1, select2;
	static void *wave = NULL;
#ifdef WIN32
		ALint error;
		ALvoid *data;
		ALboolean loop;
#endif

	debug_message("Initializing OpenAL ... ");
	audio_device = alcOpenDevice(NULL);
	if (audio_device == NULL)
		debug_message("NULL device (warning.).\n");
	else
		debug_message("done.\n");
	audio_context = alcCreateContext(audio_device, NULL);

	debug_message("Creating OpenAL context ... ");
	err = alcGetError(NULL);
	if (err != ALC_NO_ERROR || audio_context == NULL) {
		debug_message("failed.\n");
		return (-1);
	}
	debug_message("done.\n");

	atexit(cleanup_audio);

	debug_message("Making OpenAL context current ... ");
	alcMakeContextCurrent(audio_context);
	if (alcGetError(NULL) != ALC_NO_ERROR) {
		debug_message("failed.\n");
		return (-1);
	}
	debug_message("done.\n");

	debug_message("Generating sources ... ");
	alGenSources(10, ship_source);
	alGenSources(7, fire_source);
	alGenSources(3, explosion_source);
	alGenSources(2, effects_source);
	debug_message("done.\n");

	/* set attenuation and doppler information */
	alDistanceModel(AL_INVERSE_DISTANCE);
	alDopplerVelocity(1132);
	alDopplerFactor(1.2);

	alSourcei(ship_source[0], AL_LOOPING, 1);

	alGenBuffers(1, &weapon1);
	alGenBuffers(1, &select1);
	alGenBuffers(1, &select2);

	debug_message("Loading sounds ... ");
#ifdef WIN32
		/* load weapon1.wav sound */
		alutLoadWAVFile(apply_game_path("audio/effects/weapon1.wav"),&format,&data,&size,&freq,&loop);
		if ((error = alGetError()) != AL_NO_ERROR) {
			fprintf(stdout, "Error loading wave file.");
			/* exit(-1); */
		}

		alBufferData(weapon1,format,data,size,freq);
		if ((error = alGetError()) != AL_NO_ERROR) {
			fprintf(stdout, "Error loading sound buffer.");
			/* exit(-1); */
		}

		alutUnloadWAV(format,data,size,freq);
		if ((error = alGetError()) != AL_NO_ERROR) {
			fprintf(stdout, "Error unloading wave file.");
			/* exit(-1); */
		}
#else
	/* load weapon1.wav */
	err = alutLoadWAV(apply_game_path("audio/effects/weapon1.wav"), &wave, &format, &size, &bits, &freq);
	if (err == AL_FALSE) {
		debug_message("failed.\n");
		return (1);
	}

	alBufferData(weapon1, format, wave, size, freq);
	free(wave); /* openal makes a local copy of wave data */

	/* load select1.wav */
	err = alutLoadWAV(apply_game_path("audio/effects/select1.wav"), &wave, &format, &size, &bits, &freq);
	if (err == AL_FALSE) {
		debug_message("failed.\n");
		return (1);
	}

	alBufferData(select1, format, wave, size, freq);
	free(wave);

	/* load select2.wav */
	err = alutLoadWAV(apply_game_path("audio/effects/select2.wav"), &wave, &format, &size, &bits, &freq);
	if (err == AL_FALSE) {
		debug_message("failed.\n");
		return (1);
	}

	alBufferData(select2, format, wave, size, freq);
	free(wave);
#endif
	alSourcei(fire_source[0], AL_BUFFER, weapon1);
	alSourcei(fire_source[1], AL_BUFFER, weapon1);
	alSourcei(fire_source[2], AL_BUFFER, weapon1);
	alSourcei(fire_source[3], AL_BUFFER, weapon1);
	alSourcei(fire_source[4], AL_BUFFER, weapon1);
	alSourcei(fire_source[5], AL_BUFFER, weapon1);
	alSourcei(fire_source[6], AL_BUFFER, weapon1);
	alSourcei(effects_source[0], AL_BUFFER, select1);
	alSourcei(effects_source[0], AL_SOURCE_RELATIVE, AL_TRUE);
	alSourcei(effects_source[1], AL_BUFFER, select2);
	alSourcei(effects_source[1], AL_SOURCE_RELATIVE, AL_TRUE);
	debug_message("done.\n");

	audio_enabled = 1;
	muted = 0;
#endif

	return (0);
}

void cleanup_audio(void) {
#ifndef NAUDIO
	if (audio_enabled) {
		alcMakeContextCurrent(NULL);
		alcDestroyContext(audio_context);
		alcCloseDevice(audio_device);
		audio_context = NULL;
		audio_enabled = 0;
	}
#endif
}

void update_audio(void) {
#ifndef NAUDIO
	static Uint32 last_time = 0;
	ALfloat position[3];
	ALfloat velocity[3];
	ALfloat orientation[6];
	int i;
	extern Uint32 audio_update_delay;

	if (!audio_enabled)
		return;

	if (muted)
		return;

	if (last_time == 0) last_time = SDL_GetTicks();

	/* update openal positions every so often (exact time defined by audio_update_delay in ms) */
	if (SDL_GetTicks() > (last_time + audio_update_delay)) {

		/* update player */
		position[0] = (ALfloat)player.ship->world_x / DISTANCE_FACTOR;
		position[1] = (ALfloat)player.ship->world_y / DISTANCE_FACTOR;
		position[2] = (ALfloat)0.0;
		alListenerfv(AL_POSITION, position);

		orientation[0] = 0;
		orientation[1] = 0;
		orientation[2] = 1.0;
		orientation[3] = get_cos(player.ship->angle);
		orientation[4] = get_neg_sin(player.ship->angle);
		orientation[5] = 0;
		alListenerfv(AL_ORIENTATION, orientation);

		velocity[0] = (ALfloat)player.ship->velocity * get_cos(player.ship->angle) / DISTANCE_FACTOR;
		velocity[1] = (ALfloat)player.ship->velocity * get_neg_sin(player.ship->angle) / DISTANCE_FACTOR;
		velocity[2] = (ALfloat)0.0;

		/* update other ships */
		for (i = 0; i < 10; i++) {
			if (i == num_ships) break;
			position[0] = (ALfloat)ordered_ships[i]->world_x / DISTANCE_FACTOR;
			position[1] = (ALfloat)ordered_ships[i]->world_y / DISTANCE_FACTOR;
			position[2] = (ALfloat)0.0;
			alSourcefv(ship_source[i], AL_POSITION, position);

			velocity[0] = (ALfloat)ordered_ships[i]->velocity * get_cos(ordered_ships[i]->angle) / DISTANCE_FACTOR;
			velocity[1] = (ALfloat)ordered_ships[i]->velocity * get_neg_sin(ordered_ships[i]->angle) / DISTANCE_FACTOR;
			velocity[2] = (ALfloat)0.0;
			alSourcefv(ship_source[i], AL_VELOCITY, velocity);
			/* start playing a source if it's close enough */
			if (ordered_ships[i]->was_close == 0) {
				alSourceStop(ship_source[i]);
				alSourcei(ship_source[i], AL_BUFFER, ordered_ships[i]->type->engine->sound);
				alSourcePlay(ship_source[i]);
				ordered_ships[i]->was_close = 1;
			}
		}
		/* turn off toggles for ships that arent close */
		for (i = 10; i < MAX_SHIPS; i++) {
			ships[i].was_close = 0;
		}

		/* update other fires */
		for (i = 0; i < 7; i++) {
			if (i == num_fires) break;
			if (ordered_fires[i] == NULL) break;
			if (ordered_fires[i]->weapon == NULL) break;

			position[0] = (ALfloat)ordered_fires[i]->world_x / DISTANCE_FACTOR;
			position[1] = (ALfloat)ordered_fires[i]->world_y / DISTANCE_FACTOR;
			position[2] = (ALfloat)0.0;
			alSourcefv(fire_source[i], AL_POSITION, position);

			/* fire velocity hard-coded to 4 */
			velocity[0] = (ALfloat)4 * get_cos(ordered_fires[i]->angle) / DISTANCE_FACTOR;
			velocity[1] = (ALfloat)4 * get_neg_sin(ordered_fires[i]->angle) / DISTANCE_FACTOR;
			velocity[2] = (ALfloat)0.0;
			alSourcefv(fire_source[i], AL_VELOCITY, velocity);
		}

		last_time = SDL_GetTicks();
	}
#endif
}

void toggle_mute(void) {
#ifndef NAUDIO
	int i;

	if (!audio_enabled)
		return;

	if (muted) {
		for (i = 0; i < 10; i++)
			alSourcePlay(ship_source[i]);
		muted = 0;
	} else {
		stop_audio();
		muted = 1;
	}
#endif
}

void stop_audio(void) {
#ifndef NAUDIO
	int i;

	for (i = 0; i < 10; i++)
		alSourceStop(ship_source[i]);
	for (i = 0; i < 7; i++)
		alSourceStop(fire_source[i]);
#endif
}

/* continues playing normal audio unless muted */
void resume_audio(void) {
#ifndef NAUDIO
	if (!muted) {
		int i;

		for (i = 0; i < 10; i++)
			alSourcePlay(ship_source[i]);
	}
#endif
}

#ifndef NAUDIO
ALuint *load_sound(char *filename) {
	ALsizei size, bits, freq, format;
	ALboolean err;
	ALuint *buff = (ALuint *)malloc(sizeof(ALuint));
#ifdef WIN32
	ALint error;
	ALvoid *data;
	ALboolean loop;
#endif
	void *wave = NULL;

	alGenBuffers(1, buff);
#ifdef WIN32
	alutLoadWAVFile(filename, &format, &data, &size, &freq, &loop);
	if ((error = alGetError()) != AL_NO_ERROR) {
		fprintf(stdout, "Error loading wave file.");
	}

	alBufferData(*buff, format, data, size, freq);
	if ((error = alGetError()) != AL_NO_ERROR) {
		fprintf(stdout, "Error loading sound buffer.");
	}

	alutUnloadWAV(format,data,size,freq);
	if ((error = alGetError()) != AL_NO_ERROR) {
		fprintf(stdout, "Error unloading wave file.");
	}
#else
	err = alutLoadWAV(filename, &wave, &format, &size, &bits, &freq);
	if (err == AL_FALSE) {
		fprintf(stdout, "Could not load wave file \"%s\".\n", filename);
	}
	alBufferData(*buff, format, wave, size, freq);
	free(wave);
#endif

	return (buff);
}
#endif
