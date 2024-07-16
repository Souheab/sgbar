#ifndef __SGBAR_WIDGETS_H__
#define __SGBAR_WIDGETS_H__

#include "config.h"
#include <gtk/gtk.h>
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
gboolean update_active_tag_buttons(int tagmask);
gboolean update_occupied_tag_buttons(int tagmask);
#endif
