#include "widgets.h"
#include <X11/Xatom.h>
#include <gdk/gdkx.h>
#include <gtk/gtk.h>
// Contains X11 related functions
Display *display;
Window root_window;
Atom net_current_desktop_atom;

void x_get_display_prop() {

  Atom actual_type;
  int actual_format;
  unsigned long nitems, bytes_after;
  unsigned char *prop_data = NULL;

  if (XGetWindowProperty(display, root_window, net_current_desktop_atom, 0, 1,
                         False, XA_CARDINAL, &actual_type, &actual_format,
                         &nitems, &bytes_after, &prop_data) == Success) {
    if (prop_data) {
      long desktop = *(long *)prop_data;
      g_idle_add((GSourceFunc)update_tag_buttons, GINT_TO_POINTER(desktop));
      XFree(prop_data);
    }
  }
}

static GdkFilterReturn x_event_filter(GdkXEvent *xevent, GdkEvent *event,
                                      gpointer data) {
  XEvent *x11_event = (XEvent *)xevent;

  if (x11_event->type == PropertyNotify) {
    XPropertyEvent *prop_event = (XPropertyEvent *)x11_event;
    if (prop_event->atom == net_current_desktop_atom) {
      x_get_display_prop();
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

  net_current_desktop_atom =
      XInternAtom(display, "_NET_CURRENT_DESKTOP", False);

  gdk_window_add_filter(root_gdk_window, x_event_filter, NULL);

  XSelectInput(display, root_window, PropertyChangeMask);

  x_get_display_prop();
}
