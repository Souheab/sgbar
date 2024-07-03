#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <gdk/gdkx.h>
#include <gio/gio.h>
#include <gtk/gtk.h>
#include "config.h"
#include "widgets.h"
#include "x.h"

GtkWidget *tagbuttons[NUMTAGS];

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
  g_signal_connect(window, "realize", G_CALLBACK(setup_x_event_handling), NULL);

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
