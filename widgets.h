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
extern GtkWidget *brightness;
extern GtkWidget *battery;
extern GtkWidget *time_widget;
GtkWidget *tag_button_new(int tagnum);
GtkWidget *metric_new(GtkWidget* label, GtkWidget* scale, GtkWidget* revealer);
GtkWidget *tags_box_new();
GtkWidget *volume_widget_new();    
GtkWidget *brightess_widget_new();
GtkWidget *battery_widget_new();
void init_time_widget();
void update_tag_buttons(int currenttag);
void x_get_display_prop();
#endif
