#include "includes.h"
#include "main.h"
#include "system/path.h"
#include "system/plugin.h"

struct _plugins *plugins[MAX_PLUGINS] = { NULL };

static void autoload_plugins(char *pd);

int num_plugins = 0;

void init_plugins(void) {
#ifdef LINUX
	char plugin_path[120] = {0};

	sprintf(plugin_path, "%splugins/", apply_game_path("./"));

	/* have to give it a real variable as apply_game_path() changes */
	autoload_plugins(plugin_path);

#endif
}

void uninit_plugins(void) {
#ifdef LINUX
	int i;

	for (i = 0; i < num_plugins; i++) {
		plugins[i]->plugin->uninit();
		if (dlclose(plugins[i]->handle) != 0)
			fprintf(stdout, "Error closing shared object file. Error: %s\n", dlerror());
		free(plugins[i]);
		plugins[i] = NULL;
	}

	num_plugins = 0;

#endif
}

int load_plugin(const char *filename) {
#ifdef LINUX
	void *(*gpi)(void);
	char *error;

	plugins[num_plugins] = (struct _plugins *)malloc(sizeof(struct _plugins));

	plugins[num_plugins]->handle = dlopen(filename, RTLD_NOW);
	if (plugins[num_plugins]->handle == NULL) {
		fprintf(stdout, "Could not load plugin. Error: %s\n", dlerror());
		free(plugins[num_plugins]);
		return (-1);
	}

	gpi = dlsym(plugins[num_plugins]->handle, "get_plugin_pointer");
	if ((error = dlerror()) != NULL) {
		fprintf(stdout, "Could not load plugin. Error: %s\n", error);
		free(plugins[num_plugins]);
		return (-1);
	}

	plugins[num_plugins]->plugin = (ep_plugin *)gpi();

	plugins[num_plugins]->plugin->init();

	num_plugins++;

	return (num_plugins-1); /* return where the plugin is */

#endif
}

void plugins_erase(void) {
#ifdef LINUX
	int i;

	for (i = 0; i < num_plugins; i++) {
		if (plugins[i]->plugin->need_loop)
			plugins[i]->plugin->erase();
	}

#endif
}

void plugins_update(void) {
#ifdef LINUX
	int i;

	for (i = 0; i < num_plugins; i++) {
		if (plugins[i]->plugin->need_loop)
			plugins[i]->plugin->update();
	}

#endif
}

void plugins_draw(void) {
#ifdef LINUX
	int i;

	for (i = 0; i < num_plugins; i++) {
		if (plugins[i]->plugin->need_loop)
			plugins[i]->plugin->draw();
	}

#endif
}

/* find a plugin with the name, if found, return pointer to that plugin's struct _upgrade_info, otherwise, return NULL */
struct _upgrade_info *plugin_find_upgrade_commodity(char *name) {
#ifdef LINUX
	int i;

	for (i = 0; i < num_plugins; i++) {
		if (!strcmp(plugins[i]->plugin->name, name)) {
			return (plugins[i]->plugin->get_info());
		}
	}

	return (NULL);

#endif
}

/* true for equipped, false for not equipped (needed by src/land/upgrade.c to know if money should be deducted) */
int plugin_do_equip(char *name) {
#ifdef LINUX
	int i;

	for (i = 0; i < num_plugins; i++) {
		if (!strcmp(plugins[i]->plugin->name, name)) {
			return (plugins[i]->plugin->equip()); /* this'll return one if bought, zero if not */
		}
	}

	return (0);

#endif
}

void reset_plugins(void) {
#ifdef LINUX
	int i;

	for (i = 0; i < num_plugins; i++)
		plugins[i]->plugin->reset();

#endif
}

static void autoload_plugins(char *pd) {
#ifdef LINUX
	struct dirent **namelist = NULL;
	int n;

	if (pd == NULL)
		return;
	if ((signed)strlen(pd) <= 0)
		return;

	printf("Autoloading plugins ...\n");

	/* read the directory and look for "*.so" files */
	n = scandir(pd, &namelist, 0, alphasort);
	if (n < 0)
		perror("scandir");
	else {
		while(n--) {
			char *file = namelist[n]->d_name;
			int length = (signed)strlen(file);

			/* has to be at least "a.so" */
			if (length > 4) {
				if ((file[length - 1] == 'o') && (file[length - 2] == 's') && (file[length - 3] == '.')) {
					/* filename ends in ".so", so, assume it's a plugin */
					char plugin_path[120] = {0};
					int slot = -1;

					sprintf(plugin_path, "%s/%s", pd, file);

					/* load the plugin */
					slot = load_plugin(plugin_path);

					if (slot != -1)
						printf("    Plugin #%d: \"%s\"\n", slot, plugins[slot]->plugin->name);
				}
			}
			free(namelist[n]);
		}
		free(namelist);
	}

	printf("Done.\n");
#endif
}
