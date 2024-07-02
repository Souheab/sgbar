#include <glib-2.0/glib.h>

static const int numtags = 9;
static const char *taglabel = "";
static const char *cssfilepath = "./style.css";

void tagbuttononclick(gpointer data) {
  g_print("data: %d", *((int*) data));
}
