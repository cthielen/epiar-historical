#include "com_defs.h"
#include "game/update.h"
#include "includes.h"
#include "racing/track.h"
#include "system/eaf.h"
#include "system/math.h"
#include "system/path.h"
#include "system/video/video.h"

short int num_tracks;
SDL_Surface *marker_on = NULL, *marker_off = NULL;

int init_tracks_eaf(FILE *eaf, char *filename) {
	char line[80], name[80];
	int x, y;
	short int class;
	int prize;
	short int buoy_x, buoy_y, buoy_angle, num_buoys;
	unsigned char in_buoy;
	SDL_Surface *temp;
	int bytes_read = 0;
	unsigned long int htemp, filesize = 0;
	
	num_tracks = 0;
	
	if (eaf_find_file(eaf, filename) != 0) {
		printf("Couldn't find file \"%s\" in EAF archive.\n", filename);
		return (-1);
	}
	
	fseek(eaf, 25, SEEK_CUR); /* skip the filename portion of the file header */
	fread(&htemp, sizeof(unsigned long int), 1, eaf);
	filesize = ntohl(htemp);
	
	while(bytes_read < (signed)filesize) {
		int bytes_left = filesize - bytes_read;
		memset(line, 0, sizeof(char) * 80);
		
		if (bytes_left > 80)
			bytes_left = 80; /* so we don't read beyond the end of the file */
		
		if (fgets(line, bytes_left, eaf) == NULL)
			bytes_read = filesize; /* bail out, we're done reading */
		
		bytes_read += strlen(line); /* remember how many bytes we've read */
		
		if (bytes_left == 1)
			bytes_read++; /* fix for differences in strlen() */
		
		if (!strncmp(&line[0], "track = {", 9)) {
			name[0] = 0;
			class = 0;
			x = 0;
			y = 0;
			prize = 0;
			num_buoys = 0;
			in_buoy = 0;
			tracks[num_tracks].w = 0;
			tracks[num_tracks].h = 0;
		}
		if (!strncmp(&line[0], "\tname = ", 8)) {
			strncpy(name, &line[8], strlen(line) - 9);
			name[strlen(line) - 9] = 0;
			tracks[num_tracks].name = (char *)malloc(sizeof(char) * (strlen(name) + 1));
			strcpy(tracks[num_tracks].name, name);
			tracks[num_tracks].name[strlen(name)] = 0;
		}
		if (!strncmp(&line[0], "\tclass = ", 9)) tracks[num_tracks].class = atoi(&line[10]);
		if (!strncmp(&line[0], "\tx = ", 5)) tracks[num_tracks].x = atoi(&line[5]);
		if (!strncmp(&line[0], "\ty = ", 5)) tracks[num_tracks].y = atoi(&line[5]);
		if (!strncmp(&line[0], "\tprize = ", 9)) tracks[num_tracks].prize = atoi(&line[9]);
		if (!strncmp(&line[0], "\tbuoy pair = ", 13))
			in_buoy = 1;
		if (!strncmp(&line[0], "\t\tx = ", 6) && in_buoy) buoy_x = atoi(&line[6]);
		if (!strncmp(&line[0], "\t\ty = ", 6) && in_buoy) buoy_y = atoi(&line[6]);
		if (!strncmp(&line[0], "\t\tangle = ", 10) && in_buoy) buoy_angle = atoi(&line[10]);
		if (!strncmp(&line[0], "\t}", 2) && in_buoy) {
			tracks[num_tracks].buoys[num_buoys][0] = buoy_x;
			tracks[num_tracks].buoys[num_buoys][1] = buoy_y;
			tracks[num_tracks].buoys[num_buoys][2] = (buoy_angle * (3.141592654 / 180.0)); /* convert to radians */
			tracks[num_tracks].buoys[num_buoys][3] = 0;
			if ((buoy_x > tracks[num_tracks].w) || (buoy_x < -tracks[num_tracks].w)) {
				if (buoy_x > 0)
					tracks[num_tracks].w = buoy_x;
				else
					tracks[num_tracks].w = -buoy_x;
			}
			if ((buoy_y > tracks[num_tracks].h) || (buoy_y < -tracks[num_tracks].h)) {
				if (buoy_y > 0)
					tracks[num_tracks].h = buoy_y;
				else
					tracks[num_tracks].h = -buoy_y;
			}
			in_buoy = 0;
			num_buoys++;
		}
		if (!strncmp(&line[0], "}", 1)) {
			tracks[num_tracks].num_buoys = num_buoys;
			tracks[num_tracks].buoys_cleared = 0;
			tracks[num_tracks].w += 40; /* buffer for error (clean up) */
			tracks[num_tracks].h += 40;
			num_tracks++;
		}
	}
	
	temp = eaf_load_png(epiar_eaf, "racing/marker_on.png");
	if (temp == NULL) {
		fprintf(stdout, "Could not load \"marker_on.png\".\n");
		exit(-1);
	}
	SDL_SetColorKey(temp, SDL_RLEACCEL | SDL_SRCCOLORKEY, blue);
	marker_on = SDL_DisplayFormat(temp);
	SDL_FreeSurface(temp);
	
	temp = eaf_load_png(epiar_eaf, "racing/marker_off.png");
	if (temp == NULL) {
		fprintf(stdout, "Could not load \"marker_on.png\".\n");
		exit(-1);
	}
	SDL_SetColorKey(temp, SDL_RLEACCEL | SDL_SRCCOLORKEY, blue);
	marker_off = SDL_DisplayFormat(temp);
	SDL_FreeSurface(temp);
	
	return (0);
}

/* clears track data */
void deinit_tracks(void) {
	int i;

	for (i = 0; i < num_tracks; i++) {
		free(tracks[i].name);
		tracks[i].name = NULL;
	}

	num_tracks = 0;

	if (marker_on) {
		SDL_FreeSurface(marker_on);
		marker_on = NULL;
	}
	if (marker_off) {
		SDL_FreeSurface(marker_off);
		marker_off = NULL;
	}
}

void draw_tracks(void) {
	int i;

	for (i = 0; i < num_tracks; i++) {
		if ((tracks[i].screen_x + tracks[i].w) > 0)
			if ((tracks[i].screen_x - tracks[i].w) < scr_right)
				if ((tracks[i].screen_y + tracks[i].h) > 0)
					if ((tracks[i].screen_y - tracks[i].h) < scr_bottom) {
						int j;

						/* within range of a track to draw, so ... draw it! */
						for (j = 0; j < tracks[i].num_buoys; j++) {
							int buoy_scr_x = tracks[i].screen_x + tracks[i].buoys[j][0], buoy_scr_y = tracks[i].screen_y - tracks[i].buoys[j][1];
							int buoy1_x, buoy1_y, buoy2_x, buoy2_y;

							/* the buoy coords are actually the center of pair of buoys, rotated about the specified angle, so, draw both */
							buoy1_x = buoy_scr_x + (int)(cos(tracks[i].buoys[j][2]) * 85.0);
							buoy1_y = buoy_scr_y - (int)(sin(tracks[i].buoys[j][2]) * 85.0);
							buoy2_x = buoy_scr_x + (int)(-cos(tracks[i].buoys[j][2]) * 85.0);
							buoy2_y = buoy_scr_y - (int)(-sin(tracks[i].buoys[j][2]) * 85.0);

							if ((buoy1_x > -40) && (buoy1_x < (scr_right + 40)) && (buoy1_y > -40) && (buoy1_y < (scr_bottom + 40))) {
								SDL_Rect src, dest;

								/* draw the first part of the buoy part */
								src.x = 0;
								src.y = 0;
								src.w = marker_on->w;
								src.h = marker_on->h;
								dest.x = buoy1_x - (src.w / 2);
								dest.y = buoy1_y - (src.h / 2);
								dest.w = marker_on->w;
								dest.h = marker_on->h;
								if (tracks[i].buoys[j][3])
									blit_surface(marker_off, &src, &dest, 0);
								else
									blit_surface(marker_on, &src, &dest, 0);
							}
							if ((buoy2_x > -40) && (buoy2_x < (scr_right + 40)) && (buoy2_y > -40) && (buoy2_y < (scr_bottom + 40))) {
								SDL_Rect src, dest;

								/* draw the first part of the buoy part */
								src.x = 0;
								src.y = 0;
								src.w = marker_on->w;
								src.h = marker_on->h;
								dest.x = buoy2_x - (src.w / 2);
								dest.y = buoy2_y - (src.h / 2);
								dest.w = marker_on->w;
								dest.h = marker_on->h;
								if (tracks[i].buoys[j][3])
									blit_surface(marker_off, &src, &dest, 0);
								else
									blit_surface(marker_on, &src, &dest, 0);
							}
						}
					}
	}
}

void erase_tracks(void) {
	int i;

	for (i = 0; i < num_tracks; i++) {
		if ((tracks[i].screen_x + tracks[i].w) > 0)
			if ((tracks[i].screen_x - tracks[i].w) < scr_right)
				if ((tracks[i].screen_y + tracks[i].h) > 0)
					if ((tracks[i].screen_y - tracks[i].h) < scr_bottom) {
						int j;

						/* within range of a track to erase, so ... erase it! */
						for (j = 0; j < tracks[i].num_buoys; j++) {
							int buoy_scr_x = tracks[i].screen_x + tracks[i].buoys[j][0], buoy_scr_y = tracks[i].screen_y - tracks[i].buoys[j][1];
							int buoy1_x, buoy1_y, buoy2_x, buoy2_y;

							/* the buoy coords are actually the center of pair of buoys, rotated about the specified angle, so, draw both */
							buoy1_x = buoy_scr_x + (int)(cos(tracks[i].buoys[j][2]) * 85.0);
							buoy1_y = buoy_scr_y - (int)(sin(tracks[i].buoys[j][2]) * 85.0);
							buoy2_x = buoy_scr_x + (int)(-cos(tracks[i].buoys[j][2]) * 85.0);
							buoy2_y = buoy_scr_y - (int)(-sin(tracks[i].buoys[j][2]) * 85.0);

							if ((buoy1_x > -40) && (buoy1_x < (scr_right + 40)) && (buoy1_y > -40) && (buoy1_y < (scr_bottom + 40))) {
								SDL_Rect dest;

								/* erase the first part of the buoy part */
								dest.w = marker_on->w;
								dest.h = marker_on->h;
								dest.x = buoy1_x - (dest.w / 2);
								dest.y = buoy1_y - (dest.h / 2);
								black_fill_rect(&dest, 0);
							}
							if ((buoy2_x > -40) && (buoy2_x < (scr_right + 40)) && (buoy2_y > -40) && (buoy2_y < (scr_bottom + 40))) {
								SDL_Rect dest;

								/* erase the first part of the buoy part */
								dest.w = marker_on->w;
								dest.h = marker_on->h;
								dest.x = buoy2_x - (dest.w / 2);
								dest.y = buoy2_y - (dest.h / 2);
								black_fill_rect(&dest, 0);
							}
						}
					}
	}
}

void update_tracks(int old_x, int old_y, int new_x, int new_y) {
	int i;

	/* update screen coords of tracks */
	for (i = 0; i < num_tracks; i++) {
		tracks[i].screen_x = tracks[i].x - camera_x;
		tracks[i].screen_y = tracks[i].y - camera_y;
	}

	/* check to see if the player passed through any buoy pairs */
	for (i = 0; i < num_tracks; i++) {
		if ((tracks[i].screen_x + tracks[i].w) > 0)
			if ((tracks[i].screen_x - tracks[i].w) < scr_right)
				if ((tracks[i].screen_y + tracks[i].h) > 0)
					if ((tracks[i].screen_y - tracks[i].h) < scr_bottom) {
						/* track is somewhere onscreen, so check its buoys */
						int j;

						for (j = 0; j < tracks[i].num_buoys; j++) {
							int b_scr_x = tracks[i].x + tracks[i].buoys[j][0];
							int b_scr_y = tracks[i].y - tracks[i].buoys[j][1];

							if (get_distance_from_player_sqrd(b_scr_x, b_scr_y) < 7225) {
								/* buoy pair is near player */
								float x, y;
								int b_x1, b_y1, b_x2, b_y2;

								b_x1 = b_scr_x + (int)(cos(tracks[i].buoys[j][2]) * 85.0);
								b_y1 = b_scr_y - (int)(sin(tracks[i].buoys[j][2]) * 85.0);
								b_x2 = b_scr_x + (int)(-cos(tracks[i].buoys[j][2]) * 85.0);
								b_y2 = b_scr_y - (int)(-sin(tracks[i].buoys[j][2]) * 85.0);

								if (line_intersect(old_x, old_y, new_x, new_y, b_x1, b_y1, b_x2, b_y2, &x, &y) == 0) {
									if (!tracks[i].buoys[j][3]) {
										tracks[i].buoys[j][3] = 1;
										tracks[i].buoys_cleared++;
									}
								}
								/* all buoys cleared, so reset it */
								if (tracks[i].buoys_cleared == tracks[i].num_buoys) {
									int k;

									for (k = 0; k < tracks[i].num_buoys; k++) {
										tracks[i].buoys[k][3] = 0;

									tracks[i].buoys_cleared = 0;
									}
								}
							}
						}
					}
	}
}
