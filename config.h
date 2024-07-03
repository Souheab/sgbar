#ifndef __SGBAR_CONFIG_H__
#define __SGBAR_CONFIG_H__

#include <gtk/gtk.h>
#define NUMTAGS 9
static const char *taglabel = "";
static const char *cssfilepath = "./style.css";
void tagbuttononclick(GtkWidget *button, gpointer data);
#endif
