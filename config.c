#include <glib-2.0/glib.h>
#include <gtk/gtk.h>

void tagbuttononclick(GtkWidget *button, gpointer data) {
  g_print("tag clicked: %d\n", GPOINTER_TO_INT(data));
}
