#ifndef H_GUI_TEXT_ENTRY
#define H_GUI_TEXT_ENTRY

#include "gui/gui_session.h"
#include "includes.h"

/* window structure */
typedef struct {
  short int x, y, w, h, length;
  char *text;
  unsigned char update;
  SDL_Surface *trans_area;
  unsigned char visible;
  gui_session *session;
  unsigned char numeric_only;
  void (*callback) (char *text);
} gui_text_entry;

/* common window functions */
gui_text_entry *gui_create_text_entry(short int x, short int y, short int w, short int h, short int text_length, unsigned char numeric_only, gui_session *session);
int gui_destroy_text_entry(gui_text_entry *entry);
void gui_text_entry_set_focus(gui_session *session, gui_text_entry *text_entry);
void gui_show_text_entry(gui_text_entry *entry);
int gui_init_text_entry(void);
int gui_quit_text_entry(void);
void gui_text_entry_take_input(SDL_Event *event, gui_text_entry *entry);
char *gui_text_entry_get_text(gui_text_entry *entry);
void gui_text_entry_set_text(gui_text_entry *entry, char *text);
void gui_text_entry_set_callback(gui_text_entry *entry, void (*callback) (char *text));

#endif /* H_GUI_TEXT_ENTRY */
