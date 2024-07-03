#include "config.c"
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <gdk/gdkx.h>
#include <gio/gio.h>
#include <gtk/gtk.h>

Display *display;
Window root_window;
Atom net_current_desktop_atom;
GtkWidget *tagbuttons[NUMTAGS];

static GdkFilterReturn x11_event_filter(GdkXEvent *xevent, GdkEvent *event, gpointer data);

GtkWidget *createtagbutton(int tagnum) {
  GtkWidget *button;
  GtkStyleContext *stylecontext;
  button = gtk_button_new_with_label(taglabel);
  stylecontext = gtk_widget_get_style_context(button);
  gtk_style_context_add_class(stylecontext, "tagbutton");
  g_signal_connect(button, "clicked", G_CALLBACK(tagbuttononclick),
                   GINT_TO_POINTER(tagnum));
  return button;
}

void update_tag_buttons(int currenttag) {
  g_print("running update_tag_buttons\n");
  for (int i = 0; i < NUMTAGS; i++) {
    GtkWidget *button = tagbuttons[i];
    GtkStyleContext *stylecontext = gtk_widget_get_style_context(button);
    if (i == currenttag) {
      gtk_style_context_add_class(stylecontext, "tagbutton-active");
    } else {
      gtk_style_context_remove_class(stylecontext, "tagbutton-active");
    }
  }
}

static void setup_x11_event_handling(GtkWidget *widget) {
  GdkScreen *screen = gtk_widget_get_screen(widget);
  GdkWindow *root_gdk_window = gdk_screen_get_root_window(screen);

  display = GDK_WINDOW_XDISPLAY(root_gdk_window);
  root_window = GDK_WINDOW_XID(root_gdk_window);

  if (display == NULL || root_window == 0) {
    return;
  }

  net_current_desktop_atom =
      XInternAtom(display, "_NET_CURRENT_DESKTOP", False);

  gdk_window_add_filter(root_gdk_window, x11_event_filter, NULL);

  XSelectInput(display, root_window, PropertyChangeMask);
}

static GdkFilterReturn x11_event_filter(GdkXEvent *xevent, GdkEvent *event,
gpointer data) {
  XEvent *x11_event = (XEvent *)xevent;

  if (x11_event->type == PropertyNotify) {
    XPropertyEvent *prop_event = (XPropertyEvent *)x11_event;
    if (prop_event->atom == net_current_desktop_atom) {
      Atom actual_type;
      int actual_format;
      unsigned long nitems, bytes_after;
      unsigned char *prop_data = NULL;

      if (XGetWindowProperty(display, root_window, net_current_desktop_atom, 0,
                             1, False, XA_CARDINAL, &actual_type,
                             &actual_format, &nitems, &bytes_after,
                             &prop_data) == Success) {
        if (prop_data) {
          long desktop = *(long *)prop_data;
          g_idle_add((GSourceFunc)update_tag_buttons, GINT_TO_POINTER(desktop));
          g_print("Current desktop: %lu\n", *((unsigned long *)prop_data));
          XFree(prop_data);
        }
      }
      return GDK_FILTER_REMOVE;
    }
  }

  return GDK_FILTER_CONTINUE;
}

static void activate(GtkApplication *app, gpointer user_data) {
  GtkWidget *window;
  GtkWidget *box;
  // TODO: Figure out size based on current monitor
  // Maybe also create window for each monitor
  int window_width = 1920;
  int window_height = 40;

  window = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(window), "sgbar");
  gtk_window_set_default_size(GTK_WINDOW(window), window_width, window_height);
  g_signal_connect(window, "realize", G_CALLBACK(setup_x11_event_handling), NULL);


  box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_container_add(GTK_CONTAINER(window), box);

  for (int i = 0; i < NUMTAGS; i++) {
    GtkWidget *button = createtagbutton(i);
    tagbuttons[i] = button;
    gtk_container_add(GTK_CONTAINER(box), button);
  }

  gtk_window_set_type_hint(GTK_WINDOW(window), GDK_WINDOW_TYPE_HINT_DOCK);
  gtk_widget_show_all(window);

  GdkWindow *gdk_window = gtk_widget_get_window(window);

  if (gdk_window) {
    gdk_window_set_override_redirect(gdk_window, TRUE);
    gdk_window_show(gdk_window);
    gdk_window_move(gdk_window, 0, 30);
  }
  gtk_widget_show_all(window);
}

int main(int argc, char **argv) {
  GtkApplication *app;
  GdkScreen *screen;
  GtkCssProvider *cssprovider = gtk_css_provider_new();
  GFile *cssfile;
  int status;

  gtk_init(&argc, &argv);
  screen = gdk_screen_get_default();
  if (screen == NULL) {
    g_error("Failed to get screen");
  }
  cssfile = g_file_new_for_path(cssfilepath);
  gtk_css_provider_load_from_file(cssprovider, cssfile, NULL);
  gtk_style_context_add_provider_for_screen(screen,
                                            GTK_STYLE_PROVIDER(cssprovider),
                                            GTK_STYLE_PROVIDER_PRIORITY_USER);

  app = gtk_application_new("com.souheab.sgbar", G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
  status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);

  return status;
}
