#include <gtk/gtk.h>
#include "config.c"

GtkWidget *createtagbutton(int tagnum) {
  GtkWidget *button;
  button = gtk_button_new_with_label(taglabel);
  return button;
}

static void activate(GtkApplication *app, gpointer user_data) {
  GtkWidget *window;
  GtkWidget *button_box;
  // TODO: Figure out size per monitor
  int window_width = 1920;
  int window_height = 40;

  window = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(window), "sgbar");
  gtk_window_set_default_size(GTK_WINDOW(window), window_width, window_height);
  gtk_window_move(GTK_WINDOW(window) , 10, 10);

  button_box = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
  gtk_button_box_set_layout(GTK_BUTTON_BOX(button_box), GTK_BUTTONBOX_START);
  gtk_container_add(GTK_CONTAINER(window), button_box);

  for (int i = 0; i < numtags; i++) {
    gtk_container_add(GTK_CONTAINER (button_box), createtagbutton(0));
  }


  gtk_window_set_type_hint(GTK_WINDOW(window), GDK_WINDOW_TYPE_HINT_DOCK);
  gtk_widget_show_all(window);
}

int main(int argc, char **argv) {
  GtkApplication *app;
  int status;

  app = gtk_application_new("com.souheab.sgbar", G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
  status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);

  return status;
}
