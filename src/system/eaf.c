#include "includes.h"
#include "system/eaf.h"

unsigned long int major_vers, minor_vers, micro_vers;

FILE *eaf_open_file(char *file) {
	char sig[4] = {0};
	FILE *eaf = NULL;
	unsigned long int temp;

	if (file == NULL)
		return (NULL);
	if (strlen(file) <= 0)
		return (NULL);

	eaf = fopen(file, "rb");
	if (eaf == NULL) {
		printf("Could not open \"%s\".\n", file);
		return (NULL);
	}

	fread(sig, sizeof(char) * 3, 1, eaf);
	if (strcmp(sig, "EAF")) {
		printf("File \"%s\" is not an EAF archive.\n", file);
		fclose(eaf);
		eaf = NULL;
		return (NULL);
	}

	fread(&temp, 4, 1, eaf); // 32-bit unsigned long int = 4
	major_vers = ntohl(temp);
	fread(&temp, 4, 1, eaf);
	minor_vers = ntohl(temp);
	fread(&temp, 4, 1, eaf);
	micro_vers = ntohl(temp);

	if ((major_vers != 1) || (minor_vers != 1) || (micro_vers != 0)) {
		printf("File \"%s\" is not an EAF archive or is not of EAF version 1.1.0.\n", file);
		printf("File version is %d.%d.%d\n", (int)major_vers, (int)minor_vers, (int)micro_vers);
		fclose(eaf);
		eaf = NULL;
		return (NULL);
	}

	return (eaf);
}

int eaf_close_file(FILE *eaf) {
	if (eaf != NULL) {
		fclose(eaf);
		eaf = NULL;
		return (0);
	}

	return (-1);
}

/* sets 'eaf' to the beginning of the file named 'file' - 0 on success */
int eaf_find_file(FILE *eaf, char *file) {
  int bytes_read = 0;
  int eaf_filesize = 0;
  char filename[25];
  int filesize;
  
  assert(eaf);
  assert(file);
  
  if ((strlen(file) >= 25) || (strlen(file) <= 0)) {
    printf("Filename \"%s\" is too long to be in a .EAF archive.\n", file);
    return (-1);
  }
  
  /* how out how big the eaf file is */
  fseek(eaf, 0, SEEK_END);
  eaf_filesize = (int)ftell(eaf);
  
  /* skip to right past the header */
  fseek(eaf, 15, SEEK_SET);
  bytes_read = 15;
  
  /* run through the file, looking for 'file' */
  while(bytes_read < eaf_filesize) {
    memset(filename, 0, sizeof(char) * 25);
    filesize = 0;
    
    fread(filename, sizeof(char) * 25, 1, eaf);
    bytes_read += 25;
    
    if (!strcmp(filename, file)) {
      /* we're at the correct place, just, take it back to the very beginning (25 bytes ago) */
      fseek(eaf, -25, SEEK_CUR);
      return (0);
    } else {
      unsigned long int temp;
      /* not the file we want, so run past this one too */
      fread(&temp, 4, 1, eaf); // 32-bit unsigned long int = 4
      filesize = ntohl(temp);
      bytes_read += 4;
      fseek(eaf, filesize, SEEK_CUR);
      bytes_read += filesize;
    }
  }
  
  return (-1);
}

SDL_Surface *eaf_load_png(FILE *eaf, char *file) {
	SDL_RWops *rw;
	SDL_Surface *image;

	if (!file)
		return (NULL);
	if (strlen(file) >= 25) {
		printf("Filename \"%s\" is too long to be a .EAF archive.\n", file);
	}

	assert(eaf);
	assert(file);

	if ((strlen(file) >= 25) || (strlen(file) <= 0)) {
		printf("Filename \"%s\" is too long to be in a .EAF archive.\n", file);
		return (NULL);
	}

	if (eaf_find_file(eaf, file) != 0)
		return (NULL); /* file doesn't exist in eaf */

	fseek(eaf, 29, SEEK_CUR); /* move ahead 29 bytes (to skip past the char[25] filename and 4 byte filesize */

	rw = SDL_RWFromFP(eaf, 0);
	assert(rw);

	image = IMG_LoadPNG_RW(rw);
	if (image == NULL) {
		SDL_FreeRW(rw);
		return (NULL); /* could not load image */
	}

	SDL_FreeRW(rw);

	return (image);
}
