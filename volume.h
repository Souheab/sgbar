#ifndef _SGBAR_VOLUME_H_
#define _SGBAR_VOLUME_H_
#include <gtk/gtk.h>
void connect_widgets_pa(GtkWidget *label_arg, GtkWidget *scale_arg, GtkWidget *revealer_arg); 
void set_volume(int volume_percent);
gboolean is_volume_changing();
#endif
