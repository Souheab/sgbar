#include <gtk/gtk.h>

static gboolean update_clock_widget(gpointer user_data) {
  GtkWidget *clock_widget = (GtkWidget *) user_data;
  time_t rawtime;
  struct tm *timeinfo;
  char buffer[50];

  time(&rawtime);
  timeinfo = localtime(&rawtime);

  strftime(buffer, sizeof(buffer), "%H\n%M", timeinfo);
  gtk_label_set_text(GTK_LABEL(clock_widget), buffer);
  strftime(buffer, sizeof(buffer), "%H:%M:%S", timeinfo);
  gtk_widget_set_tooltip_text(clock_widget, buffer);

  return G_SOURCE_CONTINUE;
}

GtkWidget *clock_widget_new() {
  GtkWidget *clock_widget = gtk_label_new(NULL);
  g_timeout_add_seconds(1, (GSourceFunc)update_clock_widget, clock_widget);
  return clock_widget;
}
