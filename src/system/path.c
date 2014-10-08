#include "includes.h"
#include "main.h"
#include "system/path.h"

char *add_extraneous_dirs(char *argv);
char *strip_path_of_binary(char *argv);

/* takes a path relative to the game's dir and applies a full path (should always be used when opening a file) */
char *apply_game_path(const char *ppath) {
	static char *path = NULL; /* this could be a static array and that might speed this up a bit */
	int game_path_len = 0;
	int ppath_len = 0;

	if (ppath == NULL)
		return (NULL);

	if (path) {
		free(path);
		path = NULL;
	}

	/* find the len of both of these */
	while(game_path[game_path_len]) game_path_len++;
	while(ppath[ppath_len]) ppath_len++;

	path = (char *)malloc(sizeof(char) * (game_path_len + ppath_len + 1));
	memset(path, 0, sizeof(char) * (game_path_len + ppath_len + 1));
	strcpy(path, game_path);
	strcat(path, ppath);
	ppath_len = 0; /* reuse it */
	while(path[ppath_len]) ppath_len++;
	path[ppath_len] = 0;

	return (path);
}

#ifdef WIN32
int get_absolute_path(char *argv) {
	game_path = (char *)malloc(sizeof(char) * 3);
	game_path[0] = '.';
	game_path[1] = '/';
	game_path[2] = 0;

	return (0);
}
#endif

#ifdef LINUX
char *strip_path_of_binary(char *argv) {
	int len = 0, i, blen = 0;
	char *stripped = NULL;

	if (argv == NULL)
		return (NULL);

	while(argv[len]) len++;

	/* figure out how large (in number of characters) the binary name is */
	for (i = len - 1; i >= 0; i--) {
		if (argv[i] == '/')
			break;
		else
			blen++;
	}

	stripped = (char *)malloc(sizeof(char) * (len - blen + 1));
	memset(stripped, 0, sizeof(char) * (len - blen + 1));
	strncpy(stripped, argv, len - blen);

	return (stripped);
}

char *add_extraneous_dirs(char *argv) {
	static char *wo_bin = NULL;
	int wo_bin_len = 0;

	if (argv == NULL)
		return (NULL);

	if (wo_bin != NULL) {
		free(wo_bin);
		wo_bin = NULL;
	}

	wo_bin = strip_path_of_binary(argv);

	while(wo_bin[wo_bin_len]) wo_bin_len++;

	wo_bin[wo_bin_len] = 0;

	return(wo_bin);
}

int get_absolute_path(char *argv0) 
{
	int err;
	char sym[256] = {0};
	char *sym_str = NULL;
	char abs[256] = {0};
	char *stripped = NULL;
	char *argv = NULL;

	if (argv0 == NULL) {
		printf("The system did not set argv[0], exiting non-gracefully...\n");
		return (-1);
	}

	/* test for the symlink existing somewhere in PATH */
	if (!strcmp(argv0, "epiar")) {
		char *path_var = NULL;
		char test[80] = {0};
		int i = 0, j = 0;
		path_var = getenv("PATH");

		/* test each path for the symlink or hardlink, see if argv[0] exists in any of them, */
		/* if so, assume that first one is the path */
		while(path_var[i]) {
			/* store individual paths (separated by a colon) in test */
			if (path_var[i] == ':') {
				/* complete path found, so check it for the binary */
				FILE *fp;
				char temp[90] = {0};
				sprintf(temp, "%s/%s", test, argv0);

				fp = fopen(temp, "r");
				if (fp != NULL) {
					fclose(fp);
					/* found the directory that contains the file (usually a symlink) */
					argv = (char *)malloc(sizeof(char) * (strlen(test) + strlen(argv0) + 2));
					memset(argv, 0, sizeof(char) * (strlen(test) + strlen(argv0) + 2));
					sprintf(argv, "%s/%s", test, argv0);
					break;
				}

				memset(test, 0, 80);
				j = 0;
			} else {
				test[j] = path_var[i];
				j++;
			}

			/* increment loop */
			i++;
		}
	}

	/* basically, if it wasnt set up by the above code */
	if (argv == NULL) {
		argv = (char *)malloc(sizeof(char) * (strlen(argv0) + 1));
		memset(argv, 0, sizeof(char) * (strlen(argv0) + 1));
		strcpy(argv, argv0);
	}

	err = readlink(argv, sym, 256);

	/*
		if (errno == EINVAL) {
			fprintf(stdout, "not a symlink\n");
		}
	*/

	if (sym[0] != 0) {
		sym_str = strip_path_of_binary(sym);
	}

	if (getcwd(abs, 256) == NULL) {
		fprintf(stdout, "getcwd() failed to find absolute path\n");
		if (sym_str)
			free(sym_str);
		if (stripped)
			free(stripped);
		if (argv)
			free(argv);
		return (-1);
	}

	abs[strlen(abs)] = '/'; /* need to append this */

	stripped = strip_path_of_binary(argv);
	if ((sym[0] == 0) && (stripped[0] == '/')) {
		game_path = (char *)malloc(sizeof(char) * (strlen(stripped) + 1));
		memset(game_path, 0, sizeof(char) * (strlen(stripped) + 1));
		strcpy(game_path, stripped);
		if (sym_str)
			free(sym_str);
		if (stripped)
			free(stripped);
		if (argv)
			free(argv);
		return (0);
	}

	if ((argv[0] == '/') && (sym_str != NULL))
		stripped[0] = 0;

	if ((abs[0] == '/') && (strcmp(stripped, "./")) && (strncmp(stripped, "../", 3)))
		abs[0] = 0;

	if (sym_str == NULL) {
		game_path = (char *)malloc(sizeof(char) * (strlen(abs) + strlen(stripped) + 1));
		memset(game_path, 0, sizeof(char) * (strlen(abs) + strlen(stripped) + 1));
		sprintf(game_path, "%s%s", abs, stripped);
	} else {
		game_path = (char *)malloc(sizeof(char) * (strlen(abs) + strlen(stripped) + strlen(sym_str) + 1));
		memset(game_path, 0, sizeof(char) * (strlen(abs) + strlen(stripped) + strlen(sym_str) + 1));
		sprintf(game_path, "%s%s%s", abs, stripped, sym_str);
	}
	
	if (sym_str)
		free(sym_str);
	if (stripped)
		free(stripped);
	if (argv)
		free(argv);

	return (0);
}
#endif /* LINUX */
