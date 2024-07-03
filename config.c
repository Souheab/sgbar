#include <glib-2.0/glib.h>
#include <gtk/gtk.h>

#define NUMTAGS 9
static const char *taglabel = "";
static const char *cssfilepath = "./style.css";

void tagbuttononclick(GtkWidget *button, gpointer data) {
  g_print("tag clicked: %d\n", GPOINTER_TO_INT(data));
}
