#ifndef H_UPGRADE
#define H_UPGRADE

#include "includes.h"

#include "sprite/planet.h"

#define MAX_UPGRADES            45

/* various upgrade structures */
struct _weapon_mod {
	Uint32 weapon_type;
	Uint32 lifetime;
	unsigned char auto_aiming;
	int range; /* in degrees */
	int initial_ammo;
	unsigned char needs_ammo;
	unsigned char used; /* indicates whether this mod is used */
};

struct _ammo_mod {
	Uint32 weapon_type;
	int quantity; /* how many ammo per purchase */
	int mass;
	unsigned char used; /* indicates whether this mod is used */
};

struct _map_mod {
	int planets_to_reveal[MAX_PLANETS]; /* which planets to reveal - -1 for used */
	unsigned char used;
};

struct _fuel_mod {
	float multiplier; /* used against rate of fuel consumption */
	unsigned char used;
};

struct _hull_mod {
	float multiplier; /* used against amount of hull damage */
	unsigned char used;
};

struct _shield_mod {
	float multiplier; /* used against amount of shield damage */
	unsigned char used;
};

struct _engine_mod {
	float max_velocity_multiplier;
	float acceleration_multiplier;
	unsigned char used;
};

struct _legal_mod {
	int status_to_set;
	unsigned char local_only; /* if not local only (local as in that alliance/group), then the whole universe */
	unsigned char used;
};

struct _visibility_mod {
	float chance_of_being_on_scanner; /* 0.0 - 1.0 number of whether or not you appear on a sensor */
	unsigned char visible; /* whether or ship is visible or not */
	unsigned char can_fire_while_cloaked; /* whether or not you can fire while cloaked */
	unsigned char radar_show_density; /* whether or not the radar shows the density of ships (making blips larger) */
	unsigned char audio_warning; /* whether or not the game beeps when something is near */
	unsigned char beep_on_hostile; /* if true, beep on hostile, if not, beep on target */
	unsigned char used;
};

struct _ship_mod {
	unsigned char escape_pod;
	unsigned char shuttlebay;
	int max_bay_mass; /* how much mass the bay can hold (determines which ships and how many for the bay) */
	unsigned char used;
};

/* upgrade information */
struct _upgrade {
	struct _weapon_mod weapon_mod;
	struct _ammo_mod ammo_mod;
	struct _map_mod map_mod;
	struct _fuel_mod fuel_mod;
	struct _hull_mod hull_mod;
	struct _shield_mod shield_mod;
	struct _engine_mod engine_mod;
	struct _legal_mod legal_mod;
	struct _visibility_mod visibility_mod;
	struct _ship_mod ship_mod;
};

extern struct _upgrade upgrades[MAX_UPGRADES];

extern int num_upgrades;

#endif /* H_UPGRADE */
