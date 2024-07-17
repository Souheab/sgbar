#include "config.h"
#include "network.h"
#include "volume.h"
#include "battery.h"
#include "brightness.h"
#include "x.h"
#include "clock.h"
#include "tags.h"
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <gdk/gdkx.h>
#include <gio/gio.h>
#include <gtk/gtk.h>

static void activate(GtkApplication *app, gpointer user_data) {
  GtkWidget *window;
  GtkWidget *tags_box;
  GtkWidget *main_box;
  GtkWidget *left_box;
  GtkWidget *right_box;

  // Widgets:
  GtkWidget *tags_box_widget;
  GtkWidget *wifi_button;
  GtkWidget *volume;
  GtkWidget *brightness;
  GtkWidget *battery;
  GtkWidget *clock_widget;

  // TODO: Figure out size based on current monitor
  // Maybe also create window for each monitor
  int window_width = 1920;
  int window_height = 40;

  window = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(window), "sgbar");
  gtk_window_set_default_size(GTK_WINDOW(window), window_width, window_height);
  g_signal_connect(window, "realize", G_CALLBACK(setup_x_event_handling), NULL);

  main_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  left_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  right_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

  gtk_box_pack_start(GTK_BOX(main_box), left_box, TRUE, TRUE, 0);
  gtk_box_pack_end(GTK_BOX(main_box), right_box, TRUE, TRUE, 0);

  tags_box = tags_box_new();
  gtk_box_pack_start(GTK_BOX(left_box), tags_box, TRUE, TRUE, 0);

  volume = volume_widget_new();
  gtk_box_pack_end(GTK_BOX(right_box), volume, FALSE, TRUE, 0);

  brightness = brightness_widget_new();
  gtk_box_pack_end(GTK_BOX(right_box), brightness, FALSE, TRUE, 0);

  battery = battery_widget_new();
  gtk_box_pack_end(GTK_BOX(right_box), battery, FALSE, TRUE, 0);

  wifi_button = wifi_button_new();
  gtk_box_pack_end(GTK_BOX(right_box), wifi_button, FALSE, TRUE, 0);


  clock_widget = clock_widget_new();
  gtk_box_pack_end(GTK_BOX(right_box), clock_widget, FALSE, TRUE, 0);

  gtk_container_add(GTK_CONTAINER(window), main_box);

  gtk_window_set_type_hint(GTK_WINDOW(window), GDK_WINDOW_TYPE_HINT_DOCK);
  gtk_widget_show_all(window);

  GdkWindow *gdk_window = gtk_widget_get_window(window);

  if (gdk_window != NULL) {
    gdk_window_set_override_redirect(gdk_window, TRUE);
    gdk_window_show(gdk_window);
    gdk_window_move(gdk_window, 0, 0);
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
