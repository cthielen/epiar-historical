#include "includes.h"

#define MAX_PLUGINS 50

typedef struct _ep_plugin {
	int (*init) (void);
	void *(*get_info) (void);
	int (*equip) (void);
	int (*unequip) (void);
	void (*uninit) (void);
	void (*erase) (void);
	void (*update) (void);
	void (*draw) (void);
	void (*reset) (void);
	char *name;
	unsigned char need_loop; /* set to 1 if you need your erase, update, and draw functions called, else, they wont be */
} ep_plugin;

struct _plugins {
	ep_plugin *plugin;
	void *handle;
};

extern struct _plugins *plugins[MAX_PLUGINS];

extern int num_plugins;

/* general initalization functions */
void init_plugins(void);
void uninit_plugins(void);
int load_plugin(const char *filename);
void reset_plugins(void);

/* functions with specific purposes (used in main loop, used in buying, etc.) */
void plugins_erase(void);
void plugins_update(void);
void plugins_draw(void);

/* functions needed for the landing dialog */
struct _upgrade_info *plugin_find_upgrade_commodity(char *name);
int plugin_do_equip(char *name); /* true if equipped, false for not equipped */
