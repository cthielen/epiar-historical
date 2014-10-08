struct _keys {
	SDLKey quit;
	SDLKey rotate_left;
	SDLKey rotate_right;
	SDLKey thrust;
	SDLKey booster;
	SDLKey fire;
	SDLKey alt_fire;
	SDLKey screenshot;
	SDLKey select;
	SDLKey next_ship;
	SDLKey last_ship;
	SDLKey toggle_audio;
	SDLKey options;
	SDLKey land;
	SDLKey status;
	SDLKey nav;
	SDLKey hail;
	SDLKey target;
	SDLKey deselect_target;
	SDLKey near_target;
	SDLKey board;
	SDLKey pri_next;
	SDLKey pri_last;
	SDLKey sec_next;
	SDLKey sec_last;
} keys;

struct {
	unsigned char quit,
	              rotate_left,
	              rotate_right,
	              thrust,
	              booster,
	              fire,
	              alt_fire,
	              screenshot,
	              select,
	              next_ship,
	              last_ship,
	              toggle_audio,
	              options,
	              land,
	              status,
	              nav,
	              hail,
	              target,
                  deselect_target,
				  near_target,
				  board,
				  pri_next,
				  pri_last,
				  sec_next,
				  sec_last;
} key_locks;

/* certain actions, requiring the gui, must be done right after draw, and not after erase */
/* (when get_input() is run), in order to get the background transparency */
struct _post_draw {
	unsigned char screenshot;
	unsigned char hail;
	unsigned char land;
	unsigned char options;
	unsigned char nav;
	unsigned char board;
} post_draw;

int load_input_cfg(void);
int get_input(void);
char *get_key_name(SDLKey key);
void save_keybindings(void);
int unlock_keys(void); /* returns 1 always */
int lock_keys(void); /* returns 1 always (for missions) */
int do_post_draw(void);
void unlock_key(char *name);
void lock_key(char *name);
void clear_events(void);
void flush_events(void);
void reset_input(void);
