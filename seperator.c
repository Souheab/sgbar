#include <gtk/gtk.h>

GtkWidget *separator_new(gint spacing) {
  GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  GtkWidget *separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);

  gtk_widget_set_vexpand(separator, TRUE);

  GtkWidget *top_spacer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  GtkWidget *bottom_spacer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

  gtk_box_pack_start(GTK_BOX(box), top_spacer, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(box), separator, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(box), bottom_spacer, TRUE, TRUE, 0);

  gtk_box_set_child_packing(GTK_BOX(box), top_spacer, FALSE, FALSE, 0, GTK_PACK_START);
  gtk_box_set_child_packing(GTK_BOX(box), separator, TRUE, TRUE, 0, GTK_PACK_START);
  gtk_box_set_child_packing(GTK_BOX(box), bottom_spacer, FALSE, FALSE, 0, GTK_PACK_START);

  gtk_widget_set_size_request(top_spacer, -1, spacing);
  gtk_widget_set_size_request(bottom_spacer, -1, spacing);

  return box;
}
