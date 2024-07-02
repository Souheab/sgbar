#include <gtk/gtk.h>
#include <gio/gio.h>
#include "config.c"

GtkWidget *createtagbutton(int tagnum) {
  GtkWidget *button;
  GtkStyleContext *stylecontext;
  button = gtk_button_new_with_label(taglabel);
  stylecontext = gtk_widget_get_style_context(button);
  gtk_style_context_add_class(stylecontext, "tagbutton");
  g_signal_connect_swapped(button, "clicked", G_CALLBACK(tagbuttononclick), &tagnum);
  return button;
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
  gtk_window_move(GTK_WINDOW(window) , 10, 10);

  box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_container_add(GTK_CONTAINER(window), box);

  for (int i = 0; i < numtags; i++) {
    gtk_container_add(GTK_CONTAINER (box), createtagbutton(0));
  }

  gtk_window_set_type_hint(GTK_WINDOW(window), GDK_WINDOW_TYPE_HINT_DOCK);
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
  gtk_css_provider_load_from_file(cssprovider, cssfile , NULL);
  gtk_style_context_add_provider_for_screen(screen, GTK_STYLE_PROVIDER(cssprovider), GTK_STYLE_PROVIDER_PRIORITY_USER);

  app = gtk_application_new("com.souheab.sgbar", G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
  status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);

  return status;
}
