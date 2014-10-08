#include "includes.h"
#include "osdep/osdep.h"
#include "sprite/player.h"
#include "sprite/sprite.h"
#include "system/path.h"
#include "system/plugin.h"
#include "system/video/backbuffer.h"
#include "system/video/video.h"

#define PLUGIN_VERSION 1

static unsigned char has_sensor = 0;

int status_sensor_init(void);
void status_sensor_deinit(void);
void status_sensor_reset(void);
void *status_sensor_info(void);
int equip_status_sensor(void);
int unequip_status_sensor(void);
void erase_status_sensor(void);
void draw_status_sensor(void);
void update_status_sensor(void);

#ifdef LINUX
static ep_plugin myplug = {
	init: status_sensor_init,
	uninit: status_sensor_deinit,
	name: "Status Sensor",
	need_loop: 1,
	get_info: status_sensor_info,
	equip: equip_status_sensor,
	unequip: unequip_status_sensor,
	erase: erase_status_sensor,
	draw: draw_status_sensor,
	update: update_status_sensor,
	reset: status_sensor_reset,
};

ep_plugin *get_plugin_pointer() {
	return (&myplug);
}
#endif

int status_sensor_init(void) {

	return (PLUGIN_VERSION);
}

void status_sensor_deinit(void) {

}

void *status_sensor_info(void) {
	return (NULL);
}

int equip_status_sensor(void) {

	return (1);
}

int unequip_status_sensor(void) {

	return (0);
}

void erase_status_sensor(void) {

}

void draw_status_sensor(void) {

}

void update_status_sensor(void) {

}

void status_sensor_reset(void) {

	return;
}
