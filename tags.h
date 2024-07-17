#ifndef __TAGS_H__
#define __TAGS_H__
#include <gtk/gtk.h>
GtkWidget *tags_box_new();
gboolean update_active_tag_buttons(int tagmask);
gboolean update_occupied_tag_buttons(int tagmask);
#endif
