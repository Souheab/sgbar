#include <gtk/gtk.h>

GtkWidget *new_container() {
  GtkWidget *box =  gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  GtkStyleContext *context = gtk_widget_get_style_context(box);
  gtk_style_context_add_class(context, "container");
  return box;
}
