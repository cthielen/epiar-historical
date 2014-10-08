#include "ai/ai.h"
#include "asteroid/asteroid.h"
#include "audio/audio.h"
#include "audio/music.h"
#include "com_defs.h"
#include "comm/comm.h"
#include "force/force.h"
#include "game/game.h"
#include "game/update.h"
#include "hud/hud.h"
#include "input/input.h"
#include "network/net_sprite.h"
#include "network/network.h"
#include "racing/track.h"
#include "sprite/chunk.h"
#include "sprite/planet.h"
#include "sprite/fire.h"
#include "sprite/flare.h"
#include "sprite/gate.h"
#include "sprite/model.h"
#include "sprite/particle.h"
#include "sprite/r_ships.h"
#include "sprite/sprite.h"
#include "sprite/target.h"
#include "system/debug.h"
#include "system/math.h"
#include "system/plugin.h"
#include "system/rander.h"
#include "system/timer.h"
#include "system/trig.h"
#include "system/video/video.h"

Uint32 sort_time = 1500; /* time between fire/ship sorts (used to make list of what's closest for targetting and those things) */
Uint32 player_dead = 0;
Uint32 current_time = 0;
Uint32 loop_length = 0;
Uint32 r_ships_update = 0;
float time_scale = 1.0;
int camera_x = 0, camera_y = 0;

void update_universe(void) {
	int i;
	static Uint32 last_time = 0; /* merged from update_particles() */
	static Uint32 last_sort_time = 0;
	extern unsigned char view_mode;
	extern int ship_to_follow;
	int old_player_x, old_player_y;

	current_time = get_ticks();

	/* calculate the length of the last loop (to get current fps) */
	if (last_time == 0) last_time = current_time;
	loop_length = current_time - last_time;
	last_time = current_time;

	if (!view_mode) {
		if (player_dead) {
			if (current_time > player_dead) {
				game_over();
			}
		}
	}

	if ((last_sort_time + sort_time) < current_time) {
		update_targets();
		sort_fires();
		clean_models(); /* removes unused surface caches */
		if (!view_mode) {
			loose_target(player.ship->target);
			player.ship->target = acquire_target(player.ship, 1, NULL);
		}
		last_sort_time = current_time;
	}

	if (player.target.type != TARGET_NONE) {
		float dist;
		if (player.target.type == TARGET_SHIP) {
			struct _ship *ship = (struct _ship *)player.target.target;
			dist = (float)get_distance_sqrd(player.ship->world_x, player.ship->world_y, ship->world_x, ship->world_y);
		} else if (player.target.type == TARGET_GATE) {
			struct _gate *gate = (struct _gate *)player.target.target;
			dist = (float)get_distance_sqrd(player.ship->world_x, player.ship->world_y, gate->world_x, gate->world_y);
		}
		/* remember dist is squared - make sure they aren't selecting somebody far away */
		if (dist > COMM_DIST_SQRD) {
			player.target.target = NULL;
			player.target.type = TARGET_NONE;
			player.target.dist = 0.0f;
			reset_target_cycle();
		}
	}

	/* if the player is jumping, it is IMPORTANT that we do the auto-moving (the gravity field) before the updates, as input comes before updates and this is a sort of forced input */
	if (player.jump != NULL) {
		/* player is jumping */
		if (player.jump->stage == 0) {
			slow_down(player.ship);
			if (player.jump->old_dist == 0.0) {
				player.jump->old_dist = get_distance_sqrd(player.ship->world_x, player.ship->world_y, player.jump->x, player.jump->y);
			}
			if ((player.ship->momentum_x == 0) && (player.ship->momentum_y == 0)) {
				player.jump->stage = 1;
			}
		} else if (player.jump->stage == 1) {
			unsigned char facing;
			/* first, get to the line up point */
			facing = ai_turn_towards(player.ship, player.jump->x, player.jump->y);
			if (facing) {
				player.ship->angle = (int)get_angle_to(player.ship->world_x, player.ship->world_y, player.jump->x, player.jump->y);
				player.jump->stage = 2;
			}
		} else if (player.jump->stage == 2) {
			int dist;
			player.ship->accel = 1;
			cap_momentum(player.ship, MOMENTUM_CAP);
			dist = get_distance_sqrd(player.ship->world_x, player.ship->world_y, player.jump->x, player.jump->y);
			if (dist > player.jump->old_dist) {
				player.jump->old_dist = 0.0;
				player.jump->stage = 0;
			} else {
				player.jump->old_dist = dist;
			}
			/* now check to see if we're at the line up point, if so, face gate and go */
			if (near_point(player.ship, player.jump->x, player.jump->y)) {
				player.jump->stage = 3;
			}
		} else if (player.jump->stage == 3) {
			slow_down(player.ship);
			if ((player.ship->momentum_x == 0) && (player.ship->momentum_y == 0)) {
				player.jump->stage = 4;
			}
		} else if (player.jump->stage == 4) {
			/* lined up with the gate already from stage 0, so face the gate, then accelerate */
			unsigned char facing;
			facing = ai_turn_towards(player.ship, player.jump->gate_x, player.jump->gate_y);
			if (facing) {
				player.ship->angle = (int)get_angle_to(player.ship->world_x, player.ship->world_y, player.jump->gate_x, player.jump->gate_y);
				player.jump->stage = 5;
			}
		} else if (player.jump->stage == 5) {
			player.ship->accel = 1;
			cap_momentum(player.ship, MOMENTUM_CAP);
			/* see if we're in the gate, if so, set to stage 2 */
			if (near_point(player.ship, player.jump->gate_x, player.jump->gate_y)) {
				player.jump->stage = 6;
				player.jump->time = current_time;
				player.jump->old_momentum_x = player.ship->momentum_x; /* save old momentum */
				player.jump->old_momentum_y = player.ship->momentum_y;
			}
		} else if (player.jump->stage == 6) {
			/* in the gate, so let's go! */
			player.ship->momentum_x = 2.5 * get_cos(player.ship->angle) * average_loop_time;
			player.ship->momentum_y = 2.5 * get_neg_sin(player.ship->angle) * average_loop_time;
			if ((player.jump->time + 750) < current_time) {
				/* jump is complete */
				player.ship->world_x = player.jump->gate->destination->world_x;
				player.ship->world_y = player.jump->gate->destination->world_y;
				player.ship->momentum_x = player.jump->old_momentum_x; /* restore original momentum */
				player.ship->momentum_y = player.jump->old_momentum_y;
				free(player.jump);
				player.jump = NULL;
				cap_momentum(player.ship, player.ship->engine->top_speed * 2.0);
				unlock_keys();
				update_r_ships(1); /* force an update */
			}
		}
	}

	/* update a.i. before moving the camera (just like we update player coords based on input before the cam) */
	update_ai();
	if (current_time > (r_ships_update + R_SHIPS_UPDATE)) {
		update_r_ships(0);
		r_ships_update = current_time + R_SHIPS_UPDATE;
	}

	/* misc. check - if player's target is cloaked, deselect that target */
	if (player.target.type == TARGET_SHIP) {
		struct _ship *ship = (struct _ship *)player.target.target;
		if (ship->status & SHIP_CLOAKING) {
			player.target.target = NULL;
			player.target.type = TARGET_NONE;
			player.target.dist = 0.0f;
		}
	}
	if (player.ship->target) {
		assert(player.ship->target->offender);
		if ((player.ship->target->offender->status & SHIP_CLOAKING) || (player.ship->target->offender->status & SHIP_DECLOAKING)) {
			loose_target(player.ship->target);
			player.ship->target = NULL;
		}
	}

	/* perform camera/screen coords updates */
	/* calculate the camera based on the player's world coords */
	if (view_mode && (ship_to_follow != 0)) {
		camera_x = ships[ship_to_follow]->world_x - (screen_width / 2);
		camera_y = ships[ship_to_follow]->world_y - (screen_height / 2);
	} else {
		if (!player_dead) {
			camera_x = player.ship->world_x - (screen_width / 2);
			camera_y = player.ship->world_y - (screen_height / 2);
		}
	}

	/* eye candy! */
	update_flares();

	/* update particles */
	for (i = 0; i < NUM_PARTICLES; i++) {
		if (particles[i].time_left) {
			int color;
			particles[i].time_left -= loop_length;
			if (particles[i].time_left < 0) particles[i].time_left = 0;
			particles[i].world_x += (float)(((float)particles[i].momentum_x * 5.0f) * (float)time_scale);
			particles[i].world_y += (float)(((float)particles[i].momentum_y * 5.0f) * (float)time_scale);
			particles[i].screen_x = (int)(particles[i].world_x - camera_x);
			particles[i].screen_y = (int)(particles[i].world_y - camera_y);
			color = (255 * particles[i].time_left) / particles[i].length;
			particles[i].color = map_rgb(color, color, color);
		}
	}
	/* end of updating particles */

	update_chunks();

	/* calculate the ships screen coords (player is always the same) */
	for (i = 0; i < MAX_SHIPS; i++) {
		if (ships[i]) {
			ships[i]->screen_x = (int)(ships[i]->world_x - camera_x);
			ships[i]->screen_y = (int)(ships[i]->world_y - camera_y);
		}
	}

	/* calculate the planets screen coords */
	for (i = 0; i < num_planets; i++) {
		planets[i]->screen_x = (int)(planets[i]->world_x - camera_x);
		planets[i]->screen_y = (int)(planets[i]->world_y - camera_y);
	}

	/* calculate the jump gates screen coords */
	for (i = 0; i < num_gates; i++) {
		gates[i]->screen_x = (int)(gates[i]->world_x - camera_x);
		gates[i]->screen_y = (int)(gates[i]->world_y - camera_y);
	}

	/* update player's world coords */
	if (!view_mode) {
		player.ship->velocity += (player.ship->accel * average_loop_time) / 10000;
		if ((player.ship->boost) && (player.ship->has_booster)) {
			if (player.ship->velocity > (player.ship->max_velocity * player.ship->boost_strength)) player.ship->velocity = (player.ship->max_velocity * player.ship->boost_strength);
		} else {
			if (player.ship->velocity > player.ship->max_velocity) player.ship->velocity = player.ship->max_velocity;
		}
		if (player.ship->accel > 0.0f) {
			player.ship->momentum_x += player.ship->velocity * get_cos(player.ship->angle) * average_loop_time;
			player.ship->momentum_y += player.ship->velocity * get_neg_sin(player.ship->angle) * average_loop_time;
			if (player.jump == NULL) {
				if (player.ship->momentum_x > 15) player.ship->momentum_x = 15;
				if (player.ship->momentum_y > 15) player.ship->momentum_y = 15;
				if (player.ship->momentum_x < -15) player.ship->momentum_x = -15;
				if (player.ship->momentum_y < -15) player.ship->momentum_y = -15;
			}
			player.ship->accel = 0;
		}
		old_player_x = player.ship->world_x; /* old coords, needed by update_tracks() */
		old_player_y = player.ship->world_x;
		if (!player_dead) {
			player.ship->world_x += player.ship->momentum_x * time_scale;
			player.ship->world_y += player.ship->momentum_y * time_scale;
		}
	}

	update_tracks(old_player_x, old_player_y, player.ship->world_x, player.ship->world_y);

	/* update everybody else's world coords (factor in accel and all that basically) */
	for (i = 0; i < MAX_SHIPS; i++) {
		if (ships[i]) {
			ships[i]->velocity += (ships[i]->accel * average_loop_time) / 10000;
			if (ships[i]->velocity > ships[i]->max_velocity) ships[i]->velocity = ships[i]->max_velocity;
			if (ships[i]->accel > 0) {
				ships[i]->momentum_x += ships[i]->velocity * get_cos(ships[i]->angle) * average_loop_time;
				ships[i]->momentum_y += ships[i]->velocity * get_neg_sin(ships[i]->angle) * average_loop_time;
				if (ships[i]->momentum_x > (float)ships[i]->engine->top_speed) ships[i]->momentum_x = (float)ships[i]->engine->top_speed;
				if (ships[i]->momentum_y > (float)ships[i]->engine->top_speed) ships[i]->momentum_y = (float)ships[i]->engine->top_speed;
				if (ships[i]->momentum_x < -1 * (float)ships[i]->engine->top_speed) ships[i]->momentum_x = -1 * (float)ships[i]->engine->top_speed;
				if (ships[i]->momentum_y < -1 * (float)ships[i]->engine->top_speed) ships[i]->momentum_y = -1 * (float)ships[i]->engine->top_speed;
				ships[i]->accel = 0;
			}
			ships[i]->world_x += ((float)ships[i]->momentum_x * time_scale);
			ships[i]->world_y += ((float)ships[i]->momentum_y * time_scale);
		}
	}
	
	/* update cloaking */
	for (i = 0; i < MAX_SHIPS; i++) {
		if (ships[i]) {
			if (ships[i]->status & SHIP_CLOAKING) {
				int alpha;
				
				if ((ships[i]->cloak_start + 2500) < current_time) {
					alpha = 0; /* fully cloaked */
				} else {
					/* partially cloaked */
					alpha = (int)(((float)(current_time - ships[i]->cloak_start) / 2500.0f) * 255.0f);
					
					alpha = 255 - alpha;
					if (alpha < 0)
						alpha = 0;
				}
				
				if (alpha > 255) {
					printf("debug: fixing incorrect alpha of %d\n", alpha);
					alpha = 255;
				}
				
				if (ships[i]->cloak != (Uint8)alpha)
					ships[i]->cloak = (Uint8)alpha;
			}
			if (ships[i]->status & SHIP_DECLOAKING) {
				int alpha;
				
				if ((ships[i]->cloak_start + 2500) < current_time) {
					alpha = 255; /* fully cloaked */
				} else {
					/* partially cloaked */
					alpha = (int)(((float)(current_time - ships[i]->cloak_start) / 2500.0f) * 255.0f);
				}
				
				if (alpha > 255)
					alpha = 255;
				if (alpha < 0)
					alpha = 0;
				
				if (ships[i]->cloak != (Uint8)alpha)
					ships[i]->cloak = (Uint8)alpha;
			}
		}
	}
	
	/* if the player has a self-repairing hull, we apply it here (before they receieve damage from a weapon) */
	if (player.ship->plating.repairing) {
		if ((player.ship->hull_strength < player.ship->model->hull_life) && (player.ship->hull_strength > 0)) {
			float amt;
			/* they have a self-repairing hull and their hull is damaged */
			/* porportion gives you full hull repair in 2.5 mins */
			amt = (player.ship->model->hull_life * loop_length) / 150000;
			printf("repairing by %f\n", amt);
			player.ship->hull_strength += amt;
			/* ensure we didnt put their hull life over the max */
			if (player.ship->hull_strength > player.ship->model->hull_life)
				player.ship->hull_strength = player.ship->model->hull_life;
		}
	}

	/* update the starfield */
	update_starfield();

	update_fire();

	update_asteroids();

	plugins_update();

	/* last, push things around */
	update_force();

	update_hud();

	send_status();

	update_net_ships();

	update_targets();
}

int get_exact_distance(int x1, int y1, int x2, int y2) {
	return (sqrt(((x1 - x2) * (x1 - x2)) + ((y1 - y2) * (y1 - y2))));
}

void update_planet_nearby_shots(struct _ship *ship) {
	int i;
	int x = ship->world_x;
	int y = ship->world_y;

	for (i = 0; i < num_planets; i++) {
		if (planets[i]->alliance == ship->alliance) {
			/* okay, so the shot was fired by somebody of another alliance, was it nearby? */
			int dist = get_distance_sqrd(x, y, planets[i]->world_x, planets[i]->world_y);
			/* we'll just hard code 3 million units as the planet's proximity */
			if (dist < 3000000) {
				/* different alliance fired nearby, let's keep track o' the bastard */
				int j;
				for (j = 0; j < MAX_TRACK; j++) {
					if (planets[i]->offenders[j] == NULL) {
						/* found a slot to store the sucker in, let's keep track o' the baddie */
						planets[i]->offenders[j] = ship;
						break;
					}
				}
			}
		}
	}
}

int get_screen_coords(int x, int y, unsigned char x_coord) {
	if (x_coord) {
		return((int)((x - (int)camera_x)));
	} else {
		return((int)((y - (int)camera_y)));
	}

	/* something ridiculously offscreen */
	return (-400);
}

/* resets values in update loop for a new scenario */
void reset_update(void) {
	player_dead = 0; /* set time of player death to false (hasnt died) */
}
