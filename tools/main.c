/* utility to make eaf files, add files to them, remove files from them, etc. */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>

/* version to write when creating eaf files */
unsigned long int eaf_version[3] = {1, 1, 0};

int eaf_add_file(FILE *eaf, char *file);
int eaf_extract_file(FILE *eaf, char *file);
int eaf_list_files(FILE *eaf);
int eaf_remove_file(FILE *eaf, char *file);
int eaf_set_pos(FILE *eaf, char *file);
int eaf_convert_format(FILE *eaf, char *file);

char eaf_filename[80] = {0};

int main(int argc, char **argv) {
  FILE *fp_eaf = NULL;
  int i;

  /* set up correct file version */
  eaf_version[0] = htonl(eaf_version[0]);
  eaf_version[1] = htonl(eaf_version[1]);
  eaf_version[2] = htonl(eaf_version[2]);
  
  printf("eaf_util v1.0\nWritten by Chris Thielen\n\n");
  
  if (argc < 2) {
    printf("\tUsage:\n");
    printf("\t\t./eaf_util filename.eaf -a file_to_add.ext -r file_to_delete.ext -e file_to_extract.ext -l (-l lists all files)\n");
    return (-1);
  }
  
  strcpy(eaf_filename, argv[1]);
  
  fp_eaf = fopen(argv[1], "r+");
  if (!fp_eaf) {
    /* file doesn't exist, so create it */
    printf("file not found, creating ...\n");
    fp_eaf = fopen(argv[1], "w+");
    if (fp_eaf == NULL) {
      printf("Cannot access or create file \"%s\"\n", argv[1]);
      return (-1);
    } else {
      /* since we're creating a new eaf, be sure to put in the correct header */
      /* write the 3 byte EAF signature */
      fputc((int)'E', fp_eaf);
      fputc((int)'A', fp_eaf);
      fputc((int)'F', fp_eaf);
      /* write the 12 byte version (3 ints of 4 byte size) */
      fwrite(&eaf_version, sizeof(eaf_version), 1, fp_eaf);
    }
  }
  
  for (i = 2; i < argc; i++) {
    if (!strcmp(argv[i], "-a")) {
      if (eaf_add_file(fp_eaf, argv[i+1]) != 0)
	printf("error: could not add file \"%s\" to eaf file\n", argv[i+1]);
    }
    
    if (!strcmp(argv[i], "-e")) {
      if (eaf_extract_file(fp_eaf, argv[i+1]) != 0)
	printf("error: could not extract file \"%s\" to eaf file\n", argv[i+1]);
    }
    
    if (!strcmp(argv[i], "-l")) {
      if (eaf_list_files(fp_eaf) != 0)
	printf("error: could not list files\n");
    }
    
    if (!strcmp(argv[i], "-r")) {
      if (eaf_remove_file(fp_eaf, argv[i+1]) != 0)
	printf("error: could not remove file \"%s\"\n", argv[i+1]);
    }
    
    if (!strcmp(argv[i], "-f")) {
      if (eaf_set_pos(fp_eaf, argv[i+1]) != 0)
	printf("error: could not find file \"%s\"\n", argv[i+1]);
    }
    
    if (!strcmp(argv[i], "-c")) {
      if (eaf_convert_format(fp_eaf, argv[i+1]) != 0)
	printf("couldnt create converted file \"%s\"\n", argv[i+1]);
    }
  }
  
  fclose(fp_eaf);
  
  return (0);
}

/* adds file to the end of eaf file given by FILE poitner eaf */
int eaf_add_file(FILE *eaf, char *file) {
	FILE *fp = NULL;
	unsigned long int temp, filesize = 0;
	char filename[25] = {0};
	int i;

	assert(eaf);
	assert(file);
	assert(strlen(file) > 0);
	assert(strlen(file) < 25);

	fp = fopen(file, "rb");
	if (fp == NULL)
		return (-1);

	fseek(eaf, 0, SEEK_END); /* go to the end of the eaf file */

	/* figure out size of 'file' */
	fseek(fp, 0, SEEK_END);
	temp = (unsigned long int)ftell(fp);
	fseek(fp, 0, SEEK_SET);

	/* write the 25 byte file name information to the eaf file */
	strncpy(filename, file, 24);
	fwrite(filename, sizeof(char) * 25, 1, eaf);

	filesize = htonl(temp);
	/* write the filesize to the eaf file */
	fwrite(&filesize, sizeof(unsigned long int), 1, eaf);

	/* copy the file data to the eaf file */
	for (i = 0; i < temp; i++) {
		int byte;

		byte = fgetc(fp);
		fputc(byte, eaf);
	}

	fclose(fp);

	return (0);
}

/* extracts 'file' from the eaf file and writes it to current directory */
int eaf_extract_file(FILE *eaf, char *file) {
	FILE *fp = NULL;
	unsigned long int filesize = 0;
	char filename[25] = {0};
	int i;
	unsigned char found = 0;
	unsigned long int eaf_filesize = 0, temp;
	int bytes_read = 0;

	assert(eaf);
	assert(file);
	assert(strlen(file) > 0);
	assert(strlen(file) < 25);

	/* figure out how big the eaf file is, so we know when to stop reading */
	fseek(eaf, 0, SEEK_END);
	eaf_filesize = (unsigned long int)ftell(eaf);

	/* search through the eaf file looking for a file to match the passed filename ('file') */
	fseek(eaf, 15, SEEK_SET); /* go to the beginning of the eaf, just after the header */

	/* since we skipped the header, we must remember we've already read 15 bytes (15 bytes being the header) */
	bytes_read += 15;

	/* loop through all files, looking for our a file with filename 'file' */
	while((!found) && (bytes_read < (signed)eaf_filesize)) {
		memset(filename, 0, sizeof(char) * 25);
		filesize = 0;

		fread(&filename, sizeof(char), 25, eaf);
		bytes_read += 25;
		fread(&temp, sizeof(unsigned long int), 1, eaf);
		filesize = ntohl(temp);
		bytes_read += 4;

		if (!strcmp(filename, file)) {
			/* this is our file, so extract it */
			fp = fopen(filename, "wb");
			if (fp == NULL) {
				printf("Cannot open \"%s\" for write access while trying to extract file\n", filename);
				return (-1);
			}

			for (i = 0; i < filesize; i++) {
				int byte;

				byte = fgetc(eaf);
				fputc(byte, fp);
			}

			fclose(fp);
			return (0);
		} else {
			/* this is not our file, so skip it */
			fseek(eaf, filesize, SEEK_CUR);
			bytes_read += filesize;
		}
	}

	return (-1);
}

int eaf_list_files(FILE *eaf) {
  int bytes_read = 0;
  unsigned long int eaf_filesize = 0;
  char filename[25];
  unsigned long int filesize = 0, temp = 0;
  int file_num = 0;
  
  assert(eaf);
  
  /* find out how big the eaf file is */
  fseek(eaf, 0, SEEK_END);
  eaf_filesize = (unsigned long int)ftell(eaf);

  /* go 15 bytes into the file (right past the header) */
  fseek(eaf, 15, SEEK_SET);
  bytes_read += 15;
  
  /* read until the end of the file, listing all files (and their sizes) found along the way */
  while(bytes_read < (signed)eaf_filesize) {
    memset(filename, 0, sizeof(char) * 25);
    filesize = 0;
    
    fread(filename, sizeof(char) * 25, 1, eaf);
    bytes_read += 25;
    fread(&temp, 4, 1, eaf); // 32-bit unsigned long int = 4
    filesize = ntohl(temp);
    bytes_read += 4;
    printf("%d: %s (%d bytes)\n", file_num, filename, (int)filesize);
    file_num++; /* simple counter to track # of files */
    
    /* skip 'filesize' number of bytes through the file to get to the next file entry */
    fseek(eaf, filesize, SEEK_CUR);
    bytes_read += filesize;
  }
  
  return (0);
}

/* removes a file from the eaf file */
int eaf_remove_file(FILE *eaf, char *file) {
	int bytes_read = 0;
	int bytes_written = 0;
	unsigned long int eaf_filesize = 0;
	char filename[25];
	unsigned long int filesize, temp;
	unsigned char file_found = 0;
	FILE *temp_fp = NULL;

	assert(eaf);
	assert(file);
	assert(strlen(file) > 0);
	assert(strlen(file) < 25);

	/* find out how big the eaf file is */
	fseek(eaf, 0, SEEK_END);
	eaf_filesize = (unsigned long int)ftell(eaf);

	/* set the file position to just after the header */
	fseek(eaf, 15, SEEK_SET);
	bytes_read += 15;

	/* verify 'file' is a file in the eaf file before we do anything else */
	while(bytes_read < (signed)eaf_filesize) {
		memset(filename, 0, sizeof(char) * 25);
		filesize = 0;

		fread(filename, sizeof(char) * 25, 1, eaf);
		bytes_read += 25;
		fread(&temp, sizeof(int), 1, eaf);
		filesize = ntohl(temp);
		bytes_read += 4;

		if (!strcmp(filename, file)) {
			file_found = 1;
			break;
		}

		/* skip ahead the correct # of bytes */
		fseek(eaf, filesize, SEEK_CUR);
		bytes_read += filesize;
	}

	if (!file_found) {
		printf("file \"%s\" was not found in the eaf archive\n", file);
		return (-1);
	}

	/* okay, to remove the file, we must open the eaf file and a temp file, copy the eaf file */
	/* to the temp file, just, skipping the removed file, then, we reopen the eaf file in destroy mode (we open */
	/* it for writing which erases it, and copy the temp file (which is the original eaf, just, without the */
	/* removed file), and then kill the temp file */
	temp_fp = fopen("temp_eaf.102", "wb");
	if (temp_fp == NULL) {
		printf("couldnt open temp file \"temp_eaf.102\" for copying (part of removing file process)\n");
		return (-1);
	}

	/* write the header */
	/* write the 3 byte EAF signature */
	fputc((int)'E', temp_fp);
	fputc((int)'A', temp_fp);
	fputc((int)'F', temp_fp);
	/* write the 12 byte version (3 ints of 4 byte size) */
	fwrite(&eaf_version, sizeof(eaf_version), 1, temp_fp);

	/* seek in the current-going-to-be-destroyed eaf file to right after the eaf file */
	fseek(eaf, 15, SEEK_SET);
	bytes_read = 15;

	while(bytes_read < (signed)eaf_filesize) {
		unsigned char exclude = 0;
		fread(filename, sizeof(char), 25, eaf);
		bytes_read += 25;

		if (!strcmp(filename, file))
			exclude = 1; /* dont write this file's data, since we wanna remove it */

		if (!exclude)
			fwrite(filename, sizeof(char) * 25, 1, temp_fp);

		fread(&temp, sizeof(int), 1, eaf);
		filesize = ntohl(temp);
		bytes_read += 4;
		if (!exclude)
			fwrite(&temp, sizeof(int), 1, temp_fp);

		if (exclude) {
			/* if we're removing this file, just seek past it and remember how many bytes we read (seeked past) */
			fseek(eaf, filesize, SEEK_CUR);
			bytes_read += filesize;
		} else {
			/* we're including this file, so write it to the new eaf */
			int i;
			int byte;

			for (i = 0; i < filesize; i++) {
				byte = fgetc(eaf);
				fputc(byte, temp_fp);
			}

			bytes_read += filesize;
		}
	}

	/* now that the temp eaf is created, but without the file we didnt want, copy _that_ new file over */
	fclose(eaf);
	eaf = fopen(eaf_filename, "wb");
	if (eaf == NULL) {
		printf("could not reopen eaf file for writing (part of the removing file process)\n");
		return (-1);
	}

	fclose(temp_fp);
	temp_fp = fopen("temp_eaf.102", "rb");
	if (temp_fp == NULL) {
		printf("could not reopen temp file \"temp_eaf.102\" in reading mode\n");
		return (-1);
	}

	/* determine how big the temp eaf file is */
	fseek(temp_fp, 0, SEEK_END);
	eaf_filesize = (int)ftell(temp_fp);

	/* seek back to the beginning of the temp file and eaf file */
	fseek(temp_fp, 0, SEEK_SET);
	fseek(eaf, 0, SEEK_SET);

	/* copy the bytes over */
	while(bytes_written < (signed)eaf_filesize) {
		int byte;
		byte = fgetc(temp_fp);
		fputc(byte, eaf);
		bytes_written++;
	}

	fclose(temp_fp);

	/* should probably delete the temp file here (if i could find the function) */

	return (0);
}

/* sets 'eaf' to the beginning of the file named 'file' - 0 on success */
int eaf_set_pos(FILE *eaf, char *file) {
	int bytes_read = 0;
	unsigned long int eaf_filesize = 0;
	char filename[25];
	unsigned long int filesize, temp;

	assert(eaf);
	assert(file);
	assert(strlen(file) > 0);
	assert(strlen(file) < 25);

	/* how out how big the eaf file is */
	fseek(eaf, 0, SEEK_END);
	eaf_filesize = (unsigned long int)ftell(eaf);

	/* skip to right past the header */
	fseek(eaf, 15, SEEK_SET);
	bytes_read = 15;

	/* run through the file, looking for 'file' */
	while(bytes_read < (signed)eaf_filesize) {
		memset(filename, 0, sizeof(char) * 25);
		filesize = 0;

		fread(filename, sizeof(char) * 25, 1, eaf);
		bytes_read += 25;

		if (!strcmp(filename, file)) {
			/* we're at the correct place, just, take it back to the very beginning (25 bytes ago) */
			fseek(eaf, -25, SEEK_CUR);
			return (0);
		} else {
			/* not the file we want, so run past this one too */
			fread(&temp, sizeof(int), 1, eaf);
			filesize = ntohl(temp);
			bytes_read += 4;
			fseek(eaf, filesize, SEEK_CUR);
			bytes_read += filesize;
		}
	}

	return (-1);
}

/* converts from eaf 1.0.0 format to 1.1.0 */
int eaf_convert_format(FILE *eaf, char *file) {
  FILE *new = NULL;
  unsigned long int eaf_filesize = 0, bytes_read = 0;

  if (!eaf)
    return (-1);
  if (!file)
    return (-1);
  if (strlen(file) <= 0)
    return (-1);

  new = fopen(file, "wb");
  if (!new) {
    printf("couldnt open \"%s\" for write access\n", file);
    return (-1);
  }

  /* write header of new eaf */
  /* write the 3 byte EAF signature */
  fputc((int)'E', new);
  fputc((int)'A', new);
  fputc((int)'F', new);
  /* write the 12 byte version (3 ints of 4 byte size) */
  fwrite(&eaf_version, sizeof(eaf_version), 1, new);

  /* seek past old version */
  fseek(eaf, 0, SEEK_END);
  eaf_filesize = (unsigned long int)ftell(eaf);
  fseek(eaf, 0, SEEK_SET);
  fseek(eaf, 15, SEEK_CUR); /* skip old header */

  bytes_read = 15;

  /* now just read all the files and write them */
  while(bytes_read < eaf_filesize) {
    char filename[25] = {0};
    unsigned long int new_filesize = 0;
    int old_filesize, i = 0;

    /* read old file header */
    fread(filename, sizeof(char) * 25, 1, eaf);
    fread(&old_filesize, sizeof(int), 1, eaf);
    printf("converting .. file \"%s\" of size %d\n", filename, old_filesize);
    bytes_read += 29;
    /* write new file header */
    fwrite(filename, sizeof(char) * 25, 1, new);
    new_filesize = htonl((unsigned long int)old_filesize);
    fwrite(&new_filesize, sizeof(unsigned long int), 1, new);

    /* and copy the bytes */
    while(i < old_filesize) {
      int byte;

      byte = fgetc(eaf);
      fputc(byte, new);
      i++;
    }
    bytes_read += old_filesize;
  }

  fclose(new);

  return (0);
}
