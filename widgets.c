#include <gtk/gtk.h>
#include "config.h"

// Contains widget constructors

GtkWidget *createtagbutton(int tagnum) {
  GtkWidget *button;
  GtkStyleContext *stylecontext;
  button = gtk_button_new_with_label(taglabel);
  stylecontext = gtk_widget_get_style_context(button);
  gtk_style_context_add_class(stylecontext, "tagbutton");
  g_signal_connect(button, "clicked", G_CALLBACK(tagbuttononclick), GINT_TO_POINTER(tagnum));
  return button;
}
