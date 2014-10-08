#include "includes.h"
#include "menu/menu.h"
#include "network/network.h"
#include "sprite/sprite.h"
#include "system/init.h"
#include "system/path.h"
#include "system/video/video.h"

/* some extern'd variables */
char epiar_version[6] = "0.5.0";
int screen_width = 800;
int screen_height = 600;
int desired_fps = 30;
Uint32 audio_update_delay = 50;
char *playlist_filename = NULL;
unsigned char view_mode = 0;
int ship_to_follow = 0;
int desired_bpp = 16;
char *game_path;
unsigned char use_ogl = 0;
unsigned char skip_intro = 0;
FILE *epiar_eaf = NULL, *main_eaf = NULL;

static int parse_commandline(int argc, char *argv[]) {
	int i;

	for (i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "--help")) {
			fprintf(stdout, "Epiar %s\nhttp://www.epiar.net/\n\n", epiar_version);
			fprintf(stdout, " General Options:\n");
			fprintf(stdout, "    --help       - Display this message\n");
			fprintf(stdout, "    --version    - Display version number\n");
			fprintf(stdout, "\n");
			fprintf(stdout, " Graphic Options:\n");
			fprintf(stdout, "    --fullscreen - Play at fullscreen\n");
			fprintf(stdout, "    --windowed   - Play in a window\n");
			fprintf(stdout, "    --bpp num    - Attempt to use bpp bits per pixel\n");
			fprintf(stdout, "\n");
			fprintf(stdout, " Audio Options:\n");
			fprintf(stdout, "    --pls <file> - Read <file> and use that as a soundtrack\n");
			fprintf(stdout, "\n");
			fprintf(stdout, " Editor Options:\n");
			fprintf(stdout, "    --viewmode   - Play as camera\n");
			fprintf(stdout, "    --follow num - Follow sprite #num (requires --viewmode)\n");
			fprintf(stdout, " Debug Options:\n");
			fprintf(stdout, "    --skip       - Skips introduction\n");
			fprintf(stdout, "\n");
			exit(0);
		} else if (!strcmp(argv[i], "--version")) {
			fprintf(stdout, "%s\n", epiar_version);
			exit(0);
		} else if (!strcmp(argv[i], "--fullscreen")) {
			fullscreen = 1;
		} else if (!strcmp(argv[i], "--no-ships")) {
			skip_ship_load = 1;
		} else if (!strcmp(argv[i], "--windowed")) {
			fullscreen = 0;
		} else if (!strcmp(argv[i], "--viewmode")) {
			view_mode = 1;
		} else if (!strcmp(argv[i], "--network")) {
			network_to(argv[i+1]);
		} else if (!strcmp(argv[i], "--follow")) {
			ship_to_follow = atoi(argv[i + 1]);
		} else if (!strcmp(argv[i], "--pls")) {
			playlist_filename = (char *)malloc(sizeof(char) * 80);
			strcpy(playlist_filename, argv[i + 1]);
		} else if (!strcmp(argv[i], "--bpp")) {
			desired_bpp = atoi(argv[i+1]);
		} else if (!strcmp(argv[i], "--skip")) {
			skip_intro = 1;
		}
	}

	return (0);
}

int main(int argc, char *argv[]) {

	parse_commandline(argc, argv);

	get_absolute_path(argv[0]);

	init(desired_bpp);

	menu();

	de_init();

	return (0);
}
