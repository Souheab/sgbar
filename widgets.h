#ifndef __SGBAR_WIDGETS_H__
#define __SGBAR_WIDGETS_H__

#include "config.h"
#include <gtk/gtk.h>
typedef struct {
  GtkWidget *revealer;
  guint timeout_id;
} MetricData;
extern GtkWidget *tagbuttons[NUMTAGS];
extern GtkWidget *volume;
GtkWidget *tag_button_new(int tagnum);
GtkWidget *metric_new(const gchar *label_text);
void update_tag_buttons(int currenttag);
#endif
