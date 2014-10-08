#ifndef H_GUI
#define H_GUI

#include "gui/gui_session.h"
#include "gui/gui_text_entry.h"
#include "gui/gui_window.h"
#include "gui/gui_button.h"
#include "gui/gui_image.h"
#include "gui/gui_checkbox.h"
#include "gui/gui_frame.h"
#include "gui/gui_commondlg.h"
#include "gui/gui_label.h"
#include "gui/gui_scrollbar.h"
#include "gui/gui_textbox.h"
#include "gui/gui_tab.h"
#include "gui/gui_keybox.h"
#include "gui/gui_listbox.h"
#include "gui/gui_btab.h"
#include "includes.h"
#include "system/font.h"

extern afont *gui_font_normal, *gui_font_bold;

/* common gui functions */
int gui_init(void);
int gui_quit(void);
void gui_main(gui_session *session);

#endif /* H_GUI */
