#include "alliances/alliances.h"
#include "com_defs.h"
#include "gui/gui.h"
#include "includes.h"
#include "input/input.h"
#include "sprite/gate.h"
#include "sprite/planet.h"
#include "sprite/sprite.h"
#include "system/eaf.h"
#include "system/esf.h"
#include "system/font.h"
#include "system/math.h"
#include "system/path.h"
#include "system/trig.h"
#include "system/video/video.h"

struct _gate *gates[MAX_GATES];
int num_gates = 0;
static gui_session *gate_session = NULL;
static struct _gate *selected_gate = NULL;

static void nm_btn_handle(void);
static void jump_btn_handle(void);
static int add_gate(struct _gate *gate);
static struct _gate *get_gate_pointer(char *destination);
static struct _gate *create_gate(void);
static int free_gate(struct _gate *gate);

int init_gates_eaf(FILE *eaf, char *filename) {
	parsed_file *gates_esf;
	int i, j;

	if ((gates_esf = esf_new_handle()) == NULL) {
		printf("Could not allocate memory for gate parser handle\n");
		return (-1);
	}

	if (esf_set_filter(gates_esf, "gate") != 0) {
		printf("Could not set parser filter.\n");
		return (-1);
	}

	if (esf_parse_file_eaf(eaf, gates_esf, filename) != 0) {
		printf("Error parsing gates esf \"%s\"\n", filename);
		return (-1);
	}

	/* now read the parsed data and add gates */
	for (i = 0; i < gates_esf->num_items; i++) {
		struct _gate *new_gate = create_gate();
		assert(new_gate);

		for (j = 0; j < gates_esf->items[i].num_keys; j++) {
			char *name = gates_esf->items[i].keys[j].name;

			/* read through the known keys for gates and set data accordingly */
			if (!strcmp(name, "Name")) {
				char *gate_name = gates_esf->items[i].keys[j].value.cp;

				new_gate->name = (char *)malloc(sizeof(char) * (strlen(gate_name) + 1));
				memset(new_gate->name, 0, sizeof(char) * (strlen(gate_name) + 1));

				strcpy(new_gate->name, gate_name);
			} else if (!strcmp(name, "X")) {
				int x = gates_esf->items[i].keys[j].value.i;

				new_gate->world_x = x;
			} else if (!strcmp(name, "Y")) {
				int y = gates_esf->items[i].keys[j].value.i;

				new_gate->world_y = y;
			} else if (!strcmp(name, "Top Image")) {
				char *top_image = gates_esf->items[i].keys[j].value.cp;

				new_gate->top_image = (char *)malloc(sizeof(char) * (strlen(top_image) + 1));
				memset(new_gate->top_image, 0, sizeof(char) * (strlen(top_image) + 1));

				strcpy(new_gate->top_image, top_image);
			} else if (!strcmp(name, "Bottom Image")) {
				char *bottom_image = gates_esf->items[i].keys[j].value.cp;

				new_gate->bottom_image = (char *)malloc(sizeof(char) * (strlen(bottom_image) + 1));
				memset(new_gate->bottom_image, 0, sizeof(char) * (strlen(bottom_image) + 1));

				strcpy(new_gate->bottom_image, bottom_image);
			} else if (!strcmp(name, "Destination")) {
				char *dest_name = gates_esf->items[i].keys[j].value.cp;

				new_gate->dest_name = (char *)malloc(sizeof(char) * (strlen(dest_name) + 1));
				memset(new_gate->dest_name, 0, sizeof(char) * (strlen(dest_name) + 1));

				strcpy(new_gate->dest_name, dest_name);
			} else if (!strcmp(name, "Defenders")) {
				int num_defenders = gates_esf->items[i].keys[j].value.i;

				new_gate->num_defenders = num_defenders;
			}
		}

		if (add_gate(new_gate) != 0) {
			free_gate(new_gate);
			break;
		}
	}

	if (esf_close_handle(gates_esf) != 0) {
		printf("Could not close gate parser handle\n");
		return (-1);
	}

	/* and finally resolve where all gates point to */
	for (i = 0; i < num_gates; i++) {
		gates[i]->destination = get_gate_pointer(gates[i]->dest_name);
		assert(gates[i]->destination != NULL);
		free(gates[i]->dest_name);
		gates[i]->dest_name = NULL;
		gates[i]->angle = get_angle_to(gates[i]->world_x, gates[i]->world_y, gates[i]->destination->world_x, gates[i]->destination->world_y);
	}

	return (0);
}

void unload_gates(void) {
	int i;

	for (i = 0; i < num_gates; i++) {
		free_gate(gates[i]);
	}

	num_gates = 0;
}

/* called by load_gates_esf() to add newly found gates into gate structure */
static int add_gate(struct _gate *gate) {
	int i;

	if (num_gates < MAX_GATES) {
		/* generate the gate defenders */
		for (i = 0; i < gate->num_defenders; i++) {
			struct _ship *ship = create_ship();
			char sname[40] = {0};

			sprintf(sname, "%s Defender #%d", gate->name, i);

			ship->name = (char *)malloc(sizeof(char) * (strlen(sname) + 1));
			memset(ship->name, 0, sizeof(char) * (strlen(sname) + 1));
			strcpy(ship->name, sname);
			ship->model = get_model_pointer("Gate Patrol");
			ship->alliance = get_alliance_pointer("Independent");
			ship->class = (char *)malloc(sizeof(char) * (strlen("Gate Defender") + 1));
			memset(ship->class, 0, sizeof(char) * (strlen("Gate Defender") + 1));
			strcpy(ship->class, "Gate Defender");
			srand(time(NULL));
			ship->world_x = gate->world_x + (rand() % 250);
			ship->world_y = gate->world_y + (rand() % 250);
			/* set a box of dest points (so they fly around the gate) */
			ship->ai.dest_point[0][0] = gate->world_x - 1000;
			ship->ai.dest_point[0][1] = gate->world_y - 1000;
			ship->ai.dest_point[1][0] = gate->world_x + 1000;
			ship->ai.dest_point[1][1] = gate->world_y - 1000;
			ship->ai.dest_point[2][0] = gate->world_x - 1000;
			ship->ai.dest_point[2][1] = gate->world_y + 1000;
			ship->ai.dest_point[3][0] = gate->world_x + 1000;
			ship->ai.dest_point[3][1] = gate->world_y + 1000;
			ship->ai.dest_heading = 0;
			add_ship(ship);
			assert(ship);
			ship->gate = gate; /* assign it its gate */
		}

		gates[num_gates] = gate;

		num_gates++;
		return (0);
	}

	return (-1);
}

struct _jump *create_jump(struct _gate *gate) {
	struct _jump *jump = malloc(sizeof(struct _jump));

	jump->gate = gate;
	jump->angle = gate->angle;
	/* set to a point on a line 100 units away (line up for the gate) */
	jump->x = gate->world_x - (350.0 * get_cos(gate->angle));
	jump->y = gate->world_y + (350.0 * get_sin(gate->angle));
	/* figure out where the center of the gate is */
	jump->gate_x = gate->world_x;
	jump->gate_y = gate->world_y;
	jump->stage = 0;
	jump->time = 0;
	jump->old_dist = 0.0; /* used when getting lined up w/ the gate */

	return (jump);
}

static struct _gate *get_gate_pointer(char *destination) {
	int i;

	for (i = 0; i < num_gates; i++) {
		if (!strcmp(gates[i]->name, destination))
			return (gates[i]);
	}

	printf("Returning NULL for get_gate_pointer()\n");

	return (NULL);
}

int hail_gate(struct _gate *gate) {
	gui_window *hail_win;
	gui_button *nm_btn, *jump_btn;
	gui_label *lbl1, *lbl2;
	gui_image *image;
	int w, h, base;
	char message[80]; /* the message from the jump gate (under the pict) */
	
	selected_gate = gate;
	
	/* setup the gui */
	gate_session = gui_create_session();
	
	hail_win = gui_create_window(253, 125, 295, 350, gate_session);
	nm_btn = gui_create_button(323, 435, 155, 22, "Never mind", gate_session);
	jump_btn = gui_create_button(323, 405, 155, 22, "Jump", gate_session);
	
	gui_button_set_callback(nm_btn, nm_btn_handle);
	gui_button_set_callback(jump_btn, jump_btn_handle);
	
	image = gui_create_image_eaf(epiar_eaf, 263, 135, "gates/gate_hail.png", gate_session);
	assert(image);
	
	sprintf(message, "%s receieves you\n", gate->name);
	epiar_size_text(gui_font_normal, message, &w, &h, &base);
	lbl1 = gui_create_label(400 - (w/2), 355, message, gate_session);
	assert(lbl1);
	
	sprintf(message, "Usage fee is 75 credits\n");
	epiar_size_text(gui_font_normal, message, &w, &h, &base);
	lbl2 = gui_create_label(400 - (w/2), 375, message, gate_session);
	assert(lbl2);
	
	gui_session_show_all(gate_session);
	
	/* hand control over to the gui main loop */
	gui_main(gate_session);
	
	gui_session_destroy_all(gate_session);
	gui_destroy_session(gate_session);
	selected_gate = NULL;
	
	if (player.jump == NULL)
		return (0); /* no jump was created, so, return false */
	
	return (1);
}

static void nm_btn_handle(void) {
	gate_session->active = 0;
}

static void jump_btn_handle(void) {
	if (player.credits >= 75) {
		player.credits -= 75;
	} else {
		gui_alert("Not enough credits");
		gui_session_show_all(gate_session); /* ensure a redraw */
		return;
	}
	if (player.jump == NULL) {
		player.jump = create_jump(selected_gate);
		lock_keys();
	}
	gate_session->active = 0;
}

/* gets an empty gate pointer */
static struct _gate *create_gate(void) {
	struct _gate *gate = (struct _gate *)malloc(sizeof(struct _gate));

	if (!gate)
		return (NULL);

	gate->name = NULL;
	gate->top_image = NULL;
	gate->bottom_image = NULL;
	gate->world_x = 0;
	gate->world_y = 0;
	gate->screen_x = -1000;
	gate->screen_y = -1000;
	gate->num_defenders = 0;
	gate->angle = 0;
	gate->destination = NULL;
	gate->dest_name = NULL;
	gate->top_surface = NULL;
	gate->bottom_surface = NULL;

	return (gate);
}

static int free_gate(struct _gate *gate) {
	if (!gate)
		return (-1);

	if (gate->name)
		free(gate->name);
	if (gate->top_image)
		free(gate->top_image);
	if (gate->bottom_image)
		free(gate->bottom_image);

	gate->destination = NULL;

	if (gate->top_surface)
		SDL_FreeSurface(gate->top_surface);
	if (gate->bottom_surface != NULL)
		SDL_FreeSurface(gate->bottom_surface);

	return (0);
}
