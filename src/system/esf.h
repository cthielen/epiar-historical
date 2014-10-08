#ifndef H_ESF
#define H_ESF

#include "includes.h"

enum _esf_type {ESF_NONE, ESF_INTEGER, ESF_STRING, ESF_FLOAT};

union _esf_value {
	int i;
	char *cp;
	float f;
};

/* a single "key" = value */
typedef struct _parsed_key {
	char *name;
	enum _esf_type type;
	union _esf_value value;
} parsed_key;

/* one "weapon = {}" */
typedef struct _parsed_item {
	int num_keys;
	parsed_key *keys;
	int num_subitems;
	struct _parsed_item *subitems;
} parsed_item;

typedef struct _parsed_file {
	char *filter; /* like "weapon" or "ammo" */
	int num_items;
	parsed_item *items;
} parsed_file;

parsed_file *esf_new_handle(void); /* get an empty handle */
int esf_close_handle(parsed_file *pfile); /* release a handle */
int esf_set_filter(parsed_file *pfile, char *filter); /* set the filter for a file */
int esf_parse_file_eaf(FILE *eaf, parsed_file *pfile, char *filename); /* parse a file located in an eaf */

#endif /* H_ESF */
