#ifndef __SGBAR_X_H__
#define __SGBAR_X_H__
// Contains stuff related to working with X11
#include <X11/Xlib.h>
#include <gtk/gtk.h>
static GdkFilterReturn x_event_filter(GdkXEvent *xevent, GdkEvent *event, gpointer data); 
void setup_x_event_handling(GtkWidget *widget);
#endif
