#include "gui/gui.h"
#include "includes.h"
#include "system/font.h"
#include "system/video/video.h"

static int gui_text_entry_register_session(gui_session *session, gui_text_entry *entry);
static int gui_text_entry_has_focus(gui_text_entry *entry, gui_session *session);

gui_text_entry *gui_create_text_entry(short int x, short int y, short int w, short int h, short int text_length, unsigned char numeric_only, gui_session *session) {
  gui_text_entry *entry = (gui_text_entry *)malloc(sizeof(gui_text_entry));
  
  assert(entry);

  entry->x = x;
  entry->y = y;
  entry->w = w;
  entry->h = h;
  entry->callback = NULL;
  entry->session = session;
  entry->visible = 1;
  entry->update = 0;
  entry->length = text_length;
  entry->numeric_only = numeric_only; /* only allow #s to be entered? */
  entry->trans_area = NULL;
  entry->text = (char *)malloc(sizeof(char) * (text_length + 1));
  memset(entry->text, 0, sizeof(char) * (text_length + 1));
  
  if (gui_text_entry_register_session(session, entry) != 0) {
    free(entry->text);
    free(entry);
    return (NULL);
  }
  
  return (entry);
}

int gui_destroy_text_entry(gui_text_entry *entry) {
  assert(entry);
  
  if (entry->trans_area) {
    SDL_FreeSurface(entry->trans_area);
    entry->trans_area = NULL;
  }
  free(entry->text);
  free(entry);
  
  return (0);
}

void gui_show_text_entry(gui_text_entry *entry) {
  SDL_Rect rect;
  
  if (entry == NULL)
    return;
  
  if (!entry->visible)
    return;
  
  /* draw top bar */
  rect.x = entry->x;
  rect.y = entry->y;
  rect.w = entry->w;
  rect.h = 1;
  fill_rect(&rect, map_rgb(7, 100, 138));
  
  /* draw bottom bar */
  rect.x = entry->x;
  rect.y = entry->y + entry->h;
  rect.w = entry->w;
  rect.h = 1;
  fill_rect(&rect, map_rgb(7, 100, 138));
  
  /* draw left bar */
  rect.x = entry->x;
  rect.y = entry->y;
  rect.w = 1;
  rect.h = entry->h;
  fill_rect(&rect, map_rgb(7, 100, 138));
  
  /* draw right bar */
  rect.x = entry->x + entry->w;
  rect.y = entry->y;
  rect.w = 1;
  rect.h = entry->h;
  fill_rect(&rect, map_rgb(7, 100, 138));
  
  /* draw the transparent area */
  if (entry->trans_area == NULL) {
    SDL_Surface *area;
    SDL_Rect src, dest;
    
    area = SDL_CreateRGBSurface(SDL_SWSURFACE | SDL_SRCALPHA, entry->w - 2, entry->h - 2, screen->format->BitsPerPixel, screen->format->Rmask, screen->format->Gmask, screen->format->Bmask, screen->format->Amask);
    assert(area);
    rect.x = 0;
    rect.y = 0;
    rect.w = area->w;
    rect.h = area->h;
    SDL_FillRect(area, &rect, SDL_MapRGB(area->format, 0, 0, 0));
    SDL_SetAlpha(area, SDL_SRCALPHA, 93);
    
    rect.x = entry->x + 1;
    rect.y = entry->y + 1;
    rect.w = area->w;
    rect.h = area->h;
    blit_surface(area, NULL, &rect, 0);
    
    SDL_FreeSurface(area);
    
    /* now that the trans area is drawn, back up that area there */
    entry->trans_area = SDL_CreateRGBSurface(SDL_SWSURFACE | SDL_SRCALPHA, entry->w - 2, entry->h - 2, screen->format->BitsPerPixel, screen->format->Rmask, screen->format->Gmask, screen->format->Bmask, screen->format->Amask);
    src.x = entry->x + 1;
    src.y = entry->y + 1;
    src.w = entry->trans_area->w;
    src.h = entry->trans_area->h;
    dest.x = 0;
    dest.y = 0;
    dest.w = src.w;
    dest.h = src.h;
    
    SDL_BlitSurface(screen, &src, entry->trans_area, &dest);
  } else {
    rect.x = entry->x + 1;
    rect.y = entry->y + 1;
    rect.w = entry->trans_area->w;
    rect.h = entry->trans_area->h;
    
    blit_surface(entry->trans_area, NULL, &rect, 0);
  }
  
  /* finally, draw any text */
  if (strlen(entry->text) > 0) {
    int w, h, base;
    
    epiar_size_text(gui_font_normal, entry->text, &w, &h, &base);
    epiar_render_text(gui_font_normal, white, 0, AFONT_RENDER_BLENDED, screen, entry->x + 4, entry->y + (entry->h / 2) - (h/2) + base, entry->text );
    
    rect.x = entry->x + 4 + w + 2;
    rect.y = (entry->y + 1) + ((entry->h - 2) / 2) - (h/2) - 4;
    rect.w = 1;
    rect.h = entry->h - 8;
  } else {
    rect.x = entry->x + 2;
    rect.y = entry->y + (entry->h / 2) - 8;
    rect.w = 1;
    rect.h = entry->h - 8;
  }
  
  /* draw the cursor */
  if (gui_text_entry_has_focus(entry, entry->session))
    fill_rect(&rect, map_rgb(255, 255, 255));
}

int gui_init_text_entry(void) {
  return (0); /* doesn't load anything */
}

int gui_quit_text_entry(void) {
  return (0); /* doesn't load anything */
}

static int gui_text_entry_register_session(gui_session *session, gui_text_entry *entry) {
  int i, slot = -1;
  
  if (session == NULL)
    return (-1);
  if (entry == NULL)
    return (-1);
  
  for (i = 0; i < MAX_SESSION_CHILDREN; i++) {
    if (session->child_type[i] == GUI_NONE) {
      slot = i;
      break;
    }
  }
  
  if (slot == -1)
    return (-1);
  
  session->children[slot] = entry;
  session->child_type[slot] = GUI_TEXT_ENTRY;
  
  return (0);
}

void gui_text_entry_set_focus(gui_session *session, gui_text_entry *text_entry) {
  int i;

  if (session == NULL)
    return;
  if (text_entry == NULL)
    return;
  
  /* set focus and set all other entries to update (to erase the cursor from another entry) */
  for (i = 0; i < MAX_SESSION_CHILDREN; i++) {
    if (session->child_type[i] == GUI_TEXT_ENTRY) {
      gui_text_entry *te = session->children[i];
      
      if (te == text_entry)
	session->keyboard_focus = i;
      
      te->update = 1;
    }
  }
}

void gui_text_entry_take_input(SDL_Event *event, gui_text_entry *entry) {
  unsigned char capitalize = 0;
  char key = 0;
  char key_name[20] = {0};
  SDLKey skey = event->key.keysym.sym;
  static SDLKey last_key = 0;
  static Uint32 last_press = 0; /* used in key repeats */
  
  if (event == NULL)
    return;
  if (entry == NULL)
    return;
  
  if (!entry->visible)
    return;
  
  sprintf(key_name, "%s", SDL_GetKeyName(skey));
  
  if (event->type != SDL_KEYDOWN)
    return;

#ifndef WIN32
#warning KEY REPEAT RATE HARD CODED TO 450 MS
#endif
  if (skey == last_key) {
    if ((last_press + 450) > SDL_GetTicks())
      return; /* not time to repeat */
  } else {
    last_key = skey;
    last_press = SDL_GetTicks();
  }
  
  if (entry->numeric_only) {
    if ((skey >= SDLK_0) && (skey <= SDLK_9))
      key = key_name[0]; /* it's a valid number */
  } else {
    if (((skey >= SDLK_a) && (skey <= SDLK_z)) || ((skey >= SDLK_0) && (skey <= SDLK_9)))
      key = key_name[0]; /* it's a valid letter or number */
  }
  
  if (((signed)strlen(entry->text) >= entry->length) && (event->key.keysym.sym != SDLK_BACKSPACE))
    return; /* already filled */
  
  /* if we have a key mod, and it's a lowercase letter, allow uppercasing */
  if (((event->key.keysym.mod == KMOD_CAPS) || (event->key.keysym.mod == KMOD_LSHIFT) || (event->key.keysym.mod == KMOD_RSHIFT)) && ((key >= 97) && (key <= 122)))
    capitalize = 1;
  
  /* set key to something if it's a special key (space, backspace, etc.), unless it's numeric mode */
  if (skey == SDLK_BACKSPACE)
    key = 8; /* ascii for backspace */
  
  if (!entry->numeric_only) {
    if (skey == SDLK_SPACE)
      key = ' '; /* i dont know ascii for the spacebar */
  }
  
  if (key == 0)
    return; /* dunno what to tell ya kid ... */
  
  /* check for special keys */
  if (key == 8) {
    if (strlen(entry->text) > 0)
      entry->text[strlen(entry->text) - 1] = 0;
  } else {
    /* nothing special, must be a letter */
    if (capitalize) {
      char new_key = key - 32;
      strncat(entry->text, &new_key, 1);
    } else {
      strncat(entry->text, &key, 1);
    }
  }

  if (entry->callback)
    entry->callback(entry->text);
  
  entry->update = 1;
}

char *gui_text_entry_get_text(gui_text_entry *entry) {
  char *text = NULL;
  
  if (entry == NULL)
    return (NULL);

  text = (char *)malloc(sizeof(char) * (strlen(entry->text) + 1));
  memset(text, 0, sizeof(char) * (strlen(entry->text) + 1));
  strcpy(text, entry->text);
  
  return (text);
}

static int gui_text_entry_has_focus(gui_text_entry *entry, gui_session *session) {
  int i;
  
  if (entry == NULL)
    return (0);
  if (session == NULL)
    return (0);
  
  for (i = 0; i < MAX_SESSION_CHILDREN; i++) {
    if (session->child_type[i] == GUI_TEXT_ENTRY) {
      gui_text_entry *te = (gui_text_entry *)session->children[i];
      
      if ((te == entry) && (session->keyboard_focus == i))
	return (1);
    }
  }
  
  return (0);
}

void gui_text_entry_set_text(gui_text_entry *entry, char *text) {
  if (entry == NULL)
    return;
  if (text == NULL)
    return;
  
  memset(entry->text, 0, sizeof(char) * (entry->length + 1));
  strcpy(entry->text, text);
}

void gui_text_entry_set_callback(gui_text_entry *entry, void (*callback) (char *text)) {
  assert(entry);
  entry->callback = callback;
}
