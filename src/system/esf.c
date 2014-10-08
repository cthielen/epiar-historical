#include "system/eaf.h"
#include "system/esf.h"

#define LINE_LENGTH 1000

enum _esf_line_data {UNKNOWN, KEY, SUBITEM_START, SUBITEM_END};

static void esf_count_items(parsed_file *pfile, FILE *eaf, char *filename);
static void esf_extract_information(char *line, parsed_key *key);
static enum _esf_line_data esf_check_line(char *line);

parsed_file *esf_new_handle(void) {
	parsed_file *pfile = (parsed_file *)malloc(sizeof(parsed_file));

	if (!pfile)
		return (NULL);

	pfile->filter = NULL;
	pfile->num_items = 0;
	pfile->items = NULL;

	return (pfile);
}

int esf_close_handle(parsed_file *pfile) {
	int i, j, k, l;

	if (!pfile)
		return (-1);

	for (i = 0; i < pfile->num_items; i++) {
		for (j = 0; j < pfile->items[i].num_keys; j++) {
			if (pfile->items[i].keys[j].type == ESF_STRING) {
				char *string = pfile->items[i].keys[j].value.cp;
				free(string);
			}
			if (pfile->items[i].keys[j].name)
				free(pfile->items[i].keys[j].name);
		}
		free(pfile->items[i].keys);
		for (k = 0; k < pfile->items[i].num_subitems; k++) {
			for (l = 0; l < pfile->items[i].subitems[k].num_keys; l++) {
				if (pfile->items[i].subitems[k].keys[l].type == ESF_STRING) {
					char *string = pfile->items[i].subitems[k].keys[l].value.cp;
					free(string);
				}
				if (pfile->items[i].subitems[k].keys[l].name)
					free(pfile->items[i].subitems[k].keys[l].name);
			}
			free(pfile->items[i].subitems[k].keys);
		}
		if (pfile->items[i].subitems)
			free(pfile->items[i].subitems);
	}

	free(pfile->items);

	if (pfile->filter)
		free(pfile->filter);
	free(pfile);

	return (0);
}

int esf_set_filter(parsed_file *pfile, char *filter) {

	if (!pfile)
		return (-1);
	if (!filter)
		return (-1);
	if (strlen(filter) <= 0)
		return (-1);

	/* remove an old filter if one exists */
	if (pfile->filter)
		free(pfile->filter);

	pfile->filter = (char *)malloc(sizeof(char) * (strlen(filter) + 1));
	strcpy(pfile->filter, filter);
	pfile->filter[strlen(filter)] = 0; /* null-terminate */

	return (0);
}

int esf_parse_file_eaf(FILE *eaf, parsed_file *pfile, char *filename) {
	char line[LINE_LENGTH];
	int bytes_read = 0;
	unsigned long int temp, filesize = 0;
	int on_item = 0;
	int on_key = 0;
	unsigned in_item = 0;
	unsigned in_subitem = 0;
	int on_subitem = 0;
	int on_subitem_key = 0;

	if (!eaf)
		return (-1);
	if (!pfile)
		return (-1);
	if (!filename)
		return (-1);
	
	/* allocate the needed items and keys for parsing */
	esf_count_items(pfile, eaf, filename);
	
	/* do the actual parsing and filling in the data structures */
	if (eaf_find_file(eaf, filename) != 0)
		return (-1); /* file could not be found in eaf archive */
	
	fseek(eaf, 25, SEEK_CUR); /* skip the filename part of the file header (we know it) */
	fread(&temp, 4, 1, eaf); /* read in the filesize */ // 32-bit unsigned long int = 4
	filesize = ntohl(temp);
	
	while(bytes_read < (signed)filesize) {
		int bytes_left = filesize - bytes_read;
		
		if (bytes_left > LINE_LENGTH)
			bytes_left = LINE_LENGTH; /* used so we don't read beyond this file and into some other file in the archive */
		
		memset(line, 0, sizeof(char) * LINE_LENGTH);
		
		if (fgets(line, bytes_left, eaf) == NULL)
			bytes_read = filesize; /* bail out, we're done */
		
		bytes_read += strlen(line); /* keep track of the bytes we've read */
		if (bytes_left == 1)
			bytes_read++; /* fix for differences in strlen() */
		
		if (!strncmp(line, pfile->filter, strlen(pfile->filter))) {
			in_item = 1;
		} else if (!strncmp(line, "}", 1) && in_item) {
			/* end of non-subitem item */
			in_item = 0;
			on_item++;
			on_key = 0;
			in_subitem = 0;
			on_subitem_key = 0;
			on_subitem = 0;
		} else if (in_item) {
			enum _esf_line_data result = esf_check_line(line);
			
			if (result == SUBITEM_START) {
				in_subitem = 1;
			} else if (result == SUBITEM_END) {
				in_subitem = 0;
				on_subitem++;
				on_subitem_key = 0;
			} else if (result == KEY) {
				if (in_subitem) {
					assert(pfile->items[on_item].subitems[on_subitem].keys);
					esf_extract_information(line, &pfile->items[on_item].subitems[on_subitem].keys[on_subitem_key]);
					assert(pfile->items[on_item].subitems[on_subitem].keys);
					assert(pfile->items[on_item].subitems[on_subitem].keys[on_subitem_key].name);
					on_subitem_key++;
				} else {
					esf_extract_information(line, &pfile->items[on_item].keys[on_key]);
					on_key++;
				}
			}
		}
	}
	
	return (0);
}

static void esf_count_items(parsed_file *pfile, FILE *eaf, char *filename) {
	char line[LINE_LENGTH];
	int bytes_read = 0;
	unsigned long int temp, filesize = 0;
	unsigned char in_item = 0;
	int on_item = 0;
	int num_keys;
	int num_subitems;
	unsigned char in_subitem = 0;

	if (!eaf)
		return;
	if (!pfile)
		return;
	if (!filename)
		return;

	assert(pfile->num_items == 0);
	assert(pfile->filter);

	/* first, count the number of items and allocate */
	if (eaf_find_file(eaf, filename) != 0)
		return; /* file could not be found in eaf archive */

	fseek(eaf, 25, SEEK_CUR); /* skip the filename part of the file header (we know it) */
	fread(&temp, 4, 1, eaf); /* read in the filesize */
	filesize = ntohl(temp);

	while(bytes_read < (signed)filesize) {
		int bytes_left = filesize - bytes_read;

		if (bytes_left > LINE_LENGTH)
			bytes_left = LINE_LENGTH; /* used so we don't read beyond this file and into some other file in the archive */

		memset(line, 0, sizeof(char) * LINE_LENGTH);

		if (fgets(line, bytes_left, eaf) == NULL)
			bytes_read = filesize; /* bail out, we're done */

		bytes_read += strlen(line); /* keep track of the bytes we've read */
		if (bytes_left == 1)
			bytes_read++; /* fix for differences in strlen() */

		if (!strncmp(line, pfile->filter, strlen(pfile->filter))) {
			in_item = 1;
			pfile->num_items++;
		}
		if (!strncmp(line, "}", 1) && in_item)
			in_item = 0;
	}

	/* allocate the number of items we'll be needing */
	assert(pfile->items == NULL);

	if (pfile->num_items > 0) {
		pfile->items = calloc(pfile->num_items, sizeof(parsed_item));
		assert(pfile->items);
	} else {
		pfile->items = NULL;
	}

	/* parse the file and count number of keys per item, and allocate */
	if (eaf_find_file(eaf, filename) != 0)
		return; /* file could not be found in eaf archive */

	fseek(eaf, 25, SEEK_CUR); /* skip the filename part of the file header (we know it) */
	fread(&temp, 4, 1, eaf); /* read in the filesize */
	filesize = ntohl(temp);

	bytes_read = 0;

	while(bytes_read < (signed)filesize) {
		int bytes_left = filesize - bytes_read;

		if (bytes_left > LINE_LENGTH)
			bytes_left = LINE_LENGTH; /* used so we don't read beyond this file and into some other file in the archive */

		memset(line, 0, sizeof(char) * LINE_LENGTH);

		if (fgets(line, bytes_left, eaf) == NULL)
			bytes_read = filesize; /* bail out, we're done */

		bytes_read += strlen(line); /* keep track of the bytes we've read */
		if (bytes_left == 1)
			bytes_read++; /* fix for differences in strlen() */

		if (!strncmp(line, pfile->filter, strlen(pfile->filter))) {
			in_item = 1;
			num_keys = 0;
			num_subitems = 0;
		} else if (!strncmp(line, "}", 1) && in_item) {
			in_item = 0;
			if (num_keys > 0) {
				pfile->items[on_item].keys = calloc(num_keys, sizeof(parsed_key));
				assert(pfile->items[on_item].keys);
			} else {
				pfile->items[on_item].keys = NULL;
			}
			pfile->items[on_item].num_keys = num_keys;
			if (num_subitems > 0) {
				pfile->items[on_item].subitems = calloc(num_subitems, sizeof(parsed_item));
				assert(pfile->items[on_item].subitems);
			} else {
				pfile->items[on_item].subitems = NULL;
			}
			pfile->items[on_item].num_subitems = num_subitems;
			on_item++;
		} else if (in_item) {
			enum _esf_line_data result;

			result = esf_check_line(line);

			if (result == KEY) {
				if (!in_subitem)
					num_keys++;
			} else if (result == SUBITEM_START) {
				in_subitem = 1;
			} else if (result == SUBITEM_END) {
				num_subitems++;
				in_subitem = 0;
			}
		}
	}

	/* parse again to count the number of keys per subitem */
	if (eaf_find_file(eaf, filename) != 0)
		return; /* file could not be found in eaf archive */

	fseek(eaf, 25, SEEK_CUR); /* skip the filename part of the file header (we know it) */
	fread(&temp, 4, 1, eaf); /* read in the filesize */
	filesize = ntohl(temp);

	bytes_read = 0;
	num_subitems = 0;
	on_item = 0;

	while(bytes_read < (signed)filesize) {
		int bytes_left = filesize - bytes_read;

		if (bytes_left > LINE_LENGTH)
			bytes_left = LINE_LENGTH; /* used so we don't read beyond this file and into some other file in the archive */

		memset(line, 0, sizeof(char) * LINE_LENGTH);

		if (fgets(line, bytes_left, eaf) == NULL)
			bytes_read = filesize; /* bail out, we're done */

		bytes_read += strlen(line); /* keep track of the bytes we've read */
		if (bytes_left == 1)
			bytes_read++; /* fix for differences in strlen() */

		if (!strncmp(line, pfile->filter, strlen(pfile->filter))) {
			in_item = 1;
			num_keys = 0;
			in_subitem = 0;
		} else if (!strncmp(line, "}", 1) && in_item) {
			in_item = 0;
			in_subitem = 0;
			on_item++;
			num_subitems = 0;
		} else if (in_item) {
			enum _esf_line_data result;

			result = esf_check_line(line);

			if (result == KEY) {
				if (in_subitem)
					num_keys++; /* num_keys now used to count subitem keys only */
			} else if (result == SUBITEM_START) {
				in_subitem = 1;
			} else if (result == SUBITEM_END) {
				assert(pfile);
				assert(pfile->items);
				assert(pfile->items[on_item].subitems);

				pfile->items[on_item].subitems[num_subitems].num_keys = num_keys;
				if (num_keys > 0) {
					pfile->items[on_item].subitems[num_subitems].keys = calloc(num_keys, sizeof(parsed_key));
					assert(pfile->items[on_item].subitems[num_subitems].keys);
				} else {
					pfile->items[on_item].subitems[num_subitems].keys = NULL;
				}
				num_subitems++;
				in_subitem = 0;
				num_keys = 0;
			}
		}
	}
}

/* assumes line is a valid key line */
static void esf_extract_information(char *line, parsed_key *key) {
	char name[80] = {0};
	char value[550] = {0};
	int i;
	unsigned char on_name = 0;
	unsigned char on_value = 0;
	unsigned char passed_equal = 0;
	enum _esf_type type = ESF_NONE;
	int name_len = 0, value_len = 0;
	int ival = 0;
	float fval = 0.0f;

	if (!line) {
		printf("Error extracting information - bad line\n");
		return;
	}

	if (!key) {
		printf("Error extracting information - bad key\n");
		return;
	}

	for (i = 0; i < (signed)strlen(line); i++) {
		if (on_name) {
			if ((line[i] == '"') && (name[0] != 0)) {
				on_name = 0;
			} else {
				name[name_len] = line[i];
				name_len++;
			}
		}
		if (on_value) {
			if ((line[i] != '\n') && (line[i] != 0)) {
				value[value_len] = line[i];
				value_len++;
			}
		}
		if ((!on_name) && (!on_value) && (name[0] != 0)) {
			if (line[i] == '=') {
				passed_equal = 1;
			}
			if ((line[i] != ' ') && (line[i] != '=') && passed_equal) {
				on_value = 1;
				value[value_len] = line[i]; /* this is part of the value */
				value_len++;
			}
		}

		if ((!on_name) && (line[i] == '"') && (name[0] == 0)) {
			/* beginning of the name */
			on_name = 1;
		}
	}

	if (value[0] == '"') {
		char new_value[550] = {0};

		type = ESF_STRING;

		/* get rid of the quotes around a string */
		strncpy(new_value, &value[1], strlen(value) - 2);

		memset(value, 0, sizeof(char) * 550);

		strcpy(value, new_value); /* and now it doesn't have quotes */
	} else {
		/* if it has a decimal, it's a float, else, an integer */
		int i;

		for (i = 0; i < (signed)strlen(value); i++) {
			if (value[i] == '.') {
				type = ESF_FLOAT;
				break;
			}
		}

		if (type != ESF_FLOAT)
			type = ESF_INTEGER;
	}

	/* now, convert it to it's final type (string, leave it alone) */
	if (type == ESF_FLOAT) {
		fval = (float)atof(value);
	} else {
		ival = atoi(value);
	}

	/* set the key name */
	assert(key);
	key->name = (char *)malloc(sizeof(char) * (strlen(name) + 1));
	memset(key->name, 0, sizeof(char) * (strlen(name) + 1));
	strcpy(key->name, name);

	/* set the key type */
	key->type = type;

	/* finally, store the value */
	if (type == ESF_STRING) {
		char *string = (char *)malloc(sizeof(char) * (strlen(value) + 1)); /* this will be freed when handle is freed */
		memset(string, 0, sizeof(char) * (strlen(value) + 1));
		strcpy(string, value);
		key->value.cp = string;
	} else if (type == ESF_FLOAT) {
		key->value.f = fval;
	} else {
		/* must be int */
		key->value.i = ival;
	}
}

/* check a line and determine what it is, assume we are in an item */
static enum _esf_line_data esf_check_line(char *line) {
	int i;
	unsigned char has_equal = 0;
	unsigned char has_left_bracket = 0, has_right_bracket = 0;
	int num_quotes_on_left = 0;
	enum _esf_line_data_vars {LEFT_SIDE, RIGHT_SIDE} vars = LEFT_SIDE;

	if (!line)
		return (UNKNOWN);

	for (i = 0; i < (signed)strlen(line); i++) {
		if (line[i] == '"') {
			if (vars == LEFT_SIDE) {
				num_quotes_on_left++;
			}
		}
		if (line[i] == '=') {
			has_equal = 1;
			vars = RIGHT_SIDE;
		}
		if (line[i] == '{')
			has_left_bracket = 1;
		if (line[i] == '}')
			has_right_bracket = 1;
	}

	if (has_equal) {
		if ((num_quotes_on_left == 2) && (!has_left_bracket) && (!has_right_bracket)) {
			return (KEY);
		}
		if ((num_quotes_on_left == 0) && (has_left_bracket)) {
			return (SUBITEM_START);
		}
	} else {
		if ((num_quotes_on_left == 0) && (has_right_bracket)) {
			return (SUBITEM_END);
		}
	}

	return (UNKNOWN);
}
