#define MAX_NET_SHIPS 25

struct _net_ship {
	int x, y;
	short int angle;
	short int old_angle;
	struct _type *type;
	char callsign[20];
	int slot; /* assigned by server, if slot == -1 then this ship is not used (at that moment) */
	Uint32 last_update;
	SDL_Surface *surface;
	short int screen_x, screen_y;
};

struct _net_ship net_ships[MAX_NET_SHIPS];

void init_net_ships(void);
void new_net_ship(int slot, int type, int x, int y, short int angle, char *callsign);
void update_net_ships(void);
void draw_net_ships(void);
void erase_net_ships(void);
int is_ship_known(int slot);
