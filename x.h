#ifndef __SGBAR_X_H__
#define __SGBAR_X_H__
// Contains stuff related to working with X11
#include <X11/Xlib.h>
#include <gtk/gtk.h>
void setup_x_event_handling(GtkWidget *widget);
enum {
  DwmTags,
  DwmOccupiedTags,
  DwmSetTags,
  DwmLast
};
extern Atom dwm_atoms[DwmLast];
extern Atom utf8_string;
extern gint tagmask;
#endif
