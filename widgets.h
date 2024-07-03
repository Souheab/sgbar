#ifndef __SGBAR_WIDGETS_H__
#define __SGBAR_WIDGETS_H__

#include <gtk/gtk.h>
#include "config.h"
extern GtkWidget *tagbuttons[NUMTAGS];
extern GtkWidget *volume;
GtkWidget *tag_button_new(int tagnum);
GtkWidget *metric_new(const gchar* label_text);
#endif
