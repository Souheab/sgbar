#include "widgets.h"
#include <X11/Xatom.h>
#include <gdk/gdkx.h>
#include <gtk/gtk.h>
#include <limits.h>
// Contains X11 related functions
Display *display;
Window root_window;
Atom dwm_tag_mask_atom;
Atom utf8string;

void x_handle_events() {

  Atom actual_type;
  int actual_format;
  unsigned long nitems, bytes_after;
  unsigned char *prop_data = NULL;

  if (XGetWindowProperty(display, root_window, dwm_tag_mask_atom, 0, LONG_MAX,
                         False, utf8string, &actual_type, &actual_format,
                         &nitems, &bytes_after, &prop_data) == Success) {
    if (prop_data) {
      int tagmask = atoi((char*)prop_data);
      g_idle_add((GSourceFunc)update_tag_buttons, GINT_TO_POINTER(tagmask));
      XFree(prop_data);
    }
  }
}

static GdkFilterReturn x_event_filter(GdkXEvent *xevent, GdkEvent *event,
                                      gpointer data) {
  XEvent *x11_event = (XEvent *)xevent;

  if (x11_event->type == PropertyNotify) {
    XPropertyEvent *prop_event = (XPropertyEvent *)x11_event;
    if (prop_event->atom == dwm_tag_mask_atom) {
      x_handle_events();
      return GDK_FILTER_REMOVE;
    }
  }

  return GDK_FILTER_CONTINUE;
}

void setup_x_event_handling(GtkWidget *widget) {
  GdkScreen *screen = gtk_widget_get_screen(widget);
  GdkWindow *root_gdk_window = gdk_screen_get_root_window(screen);

  display = GDK_WINDOW_XDISPLAY(root_gdk_window);
  root_window = GDK_WINDOW_XID(root_gdk_window);

  if (display == NULL || root_window == 0) {
    return;
  }

  dwm_tag_mask_atom = XInternAtom(display, "DWM_TAG_MASK", False);
	utf8string = XInternAtom(display, "UTF8_STRING", False);

  XSelectInput(display, root_window, PropertyChangeMask);
  gdk_window_add_filter(root_gdk_window, x_event_filter, NULL);


  x_handle_events();
}
