#include "audio/music.h"
#include "audio/playlist.h"
#include "includes.h"

#define MAX_SONGS 250

struct _playlist {
	char file[360];
};

struct _playlist playlist[MAX_SONGS];

static int num_songs = 0;
static int cur_song = 0;

int init_playlist(void) {
	extern char *playlist_filename;
	char line[360];
	FILE *fp;

	if (playlist_filename == NULL) {
		return (-1);
	}

	fp = fopen(playlist_filename, "r");
	if (fp == NULL) {
		fprintf(stdout, "failed. Couldn't open playlist.\n");
		return (-1);
	}

	while(fgets(line, 360, fp) != NULL) {
		strcpy(playlist[num_songs].file, &line[0]);
		playlist[num_songs].file[strlen(line) - 1] = 0;
		num_songs++;
	}

	fclose(fp);

	return (0);
}

void get_next_track(void) {
	if (num_songs == 0)
		return;
	load_music(playlist[cur_song].file);
	cur_song++;
	if (cur_song == num_songs) cur_song = 0; // and ... repeat!
}

int cleanup_playst(void) {
	// for future use
	return (0);
}
