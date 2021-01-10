#include "alliances/alliances.h"
#include "includes.h"
#include "system/eaf.h"
#include "system/esf.h"
#include "system/path.h"

int num_alliances = 0;

struct _alliance alliances[MAX_ALLIANCES];

int load_alliances_eaf(FILE *eaf, char *filename) {
  parsed_file *alliances_esf = NULL;
  
  /* force load the first alliance (Independent) */
  alliances[num_alliances].name = (char *)malloc(sizeof(char) * (strlen("Independent") + 1));
  memset(alliances[num_alliances].name, 0, sizeof(char) * (strlen("Independent") + 1));
  strcpy(alliances[num_alliances].name, "Independent");
  alliances[num_alliances].logo = NULL;
  alliances[num_alliances].aggression = 0;
  num_alliances++;
  
  alliances_esf = esf_new_handle();
  assert(alliances_esf);
  
  esf_set_filter(alliances_esf, "alliance");
  
  if (esf_parse_file_eaf(eaf, alliances_esf, filename) != 0) {
    printf("Could not parse alliances.esf file!\n");
  } else {
    int i, j, k, l;
    
    for (i = 0; i < alliances_esf->num_items; i++) {
      for (j = 0; j < alliances_esf->items[i].num_keys; j++) {
	char *name = alliances_esf->items[i].keys[j].name;
	
	if (!strcmp(name, "Name")) {
	  char *text = alliances_esf->items[i].keys[j].value.cp;
	  
	  alliances[num_alliances].name = (char *)malloc(sizeof(char) * (strlen(text) + 1));
	  memset(alliances[num_alliances].name, 0, sizeof(char) * (strlen(text) + 1));
	  strcpy(alliances[num_alliances].name, text);
	} else if (!strcmp(name, "Logo")) {
	  char *text = alliances_esf->items[i].keys[j].value.cp;
	  
	  alliances[num_alliances].logo = (char *)malloc(sizeof(char) * (strlen(text) + 1));
	  memset(alliances[num_alliances].logo, 0, sizeof(char) * (strlen(text) + 1));
	  strcpy(alliances[num_alliances].logo, text);
	}
      }
      
      alliances[num_alliances].used_models = NULL; /* set to null in case we dont allocate later */
      alliances[num_alliances].num_used_models = 0;
      
      /* load the alliance ship models (what models of ships the alliance uses) */
      for (k = 0; k < alliances_esf->items[i].num_subitems; k++) {
	alliances[num_alliances].num_used_models = alliances_esf->items[i].num_subitems;
	alliances[num_alliances].used_models = calloc(alliances_esf->items[i].subitems[k].num_keys, sizeof(model_t *));
	for (l = 0; l < alliances_esf->items[i].subitems[k].num_keys; l++) {
	  char *name = alliances_esf->items[i].subitems[k].keys[l].name;
	  union _esf_value value = alliances_esf->items[i].subitems[k].keys[l].value;
	  
	  if (!strcmp(name, "Used Model")) {
	    model_t *model = NULL;
	    
	    model = get_model_pointer(value.cp);
	    if (!model)
	      printf("Couldn't find model \"%s\"\n", value.cp);
	    assert(model);
	    
	    alliances[num_alliances].used_models[l] = model;
	  }
	}
      }
      
      assert(alliances[num_alliances].name);
      assert(alliances[num_alliances].logo);
      alliances[num_alliances].aggression = (rand() % 5) + 1; /* make them angry enough to attack other alliances */
      
      num_alliances++;
    }
  }
  
  if (esf_close_handle(alliances_esf) != 0) {
    printf("Could not close parser handle!\n");
  }
  
  return (0);
}

void unload_alliances(void) {
  int i;
  
  for (i = 0; i < num_alliances; i++) {
    assert(alliances[i].name != NULL);
    free(alliances[i].name);
    alliances[i].name = NULL;
    if (alliances[i].logo)
      free(alliances[i].logo);
    alliances[i].logo = NULL;
    if (alliances[i].used_models)
      free(alliances[i].used_models); /* obviously don't free the models, the model system handles that, just free the memory used for the pointers to those models */
  }
  
  num_alliances = 0;
}

SDL_Surface *get_alliance_logo(char *alliance) {
  int i;
  
  for (i = 0; i < num_alliances; i++) {
    if (!strcmp(alliances[i].name, alliance)) {
      SDL_Surface *temp, *surface;
      
      temp = IMG_Load(apply_game_path(alliances[i].logo));
      if (temp == NULL) {
	fprintf(stdout, "Could not load logo for \"%s\" alliance.\n", alliance);
	exit(-1);
      }
      surface = SDL_DisplayFormatAlpha(temp);
      SDL_FreeSurface(temp);
      return (surface);
    }
  }
  
  /* alliance doesnt exist */
  return (NULL);
}

struct _alliance *get_alliance_pointer(char *alliance) {
  int i;
  
  if (!alliance)
    return (NULL);
  if (strlen(alliance) <= 0)
    return (NULL);
  
  for (i = 0; i < num_alliances; i++) {
    if (!strcmp(alliance, alliances[i].name))
      return (&alliances[i]);
  }
  
  return (NULL);
}

/* increases alliances aggression */
void alliance_ship_damaged(struct _alliance *alliance) {
  
  if (!alliance) {
#ifndef NDEBUG
    printf("Invalid alliance passed.\n");
#endif
    return;
  }
  
  alliance->aggression++;
  if (alliance->aggression > 10)
    alliance->aggression = 10;
}
