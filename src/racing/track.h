#define MAX_BUOY_PAIRS 100
#define MAX_TRACKS       3

struct _track {
	char *name;
	int x, y;
	int screen_x, screen_y;
	short int num_buoys, buoys_cleared, class, w, h; /* w, h is max. width height of track (used for drawing) */
	int prize;
	int buoys[MAX_BUOY_PAIRS][4]; /* x, y, angle, on/off */
} tracks[MAX_TRACKS];

extern short int num_tracks;

/* system functions */
int init_tracks_eaf(FILE *eaf, char *filename);
void deinit_tracks(void);

/* video functions */
void draw_tracks(void);
void erase_tracks(void);

/* game loop functions */
void update_tracks(int old_x, int old_y, int new_x, int new_y);
