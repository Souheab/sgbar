#include <gtk/gtk.h>
#include "config.h"

// Contains custom widget stuff
GtkWidget *tagbuttons[NUMTAGS];
GtkWidget *volume;

GtkWidget *tag_button_new(int tagnum) {
  GtkWidget *button;
  GtkStyleContext *stylecontext;
  button = gtk_button_new_with_label(taglabel);
  stylecontext = gtk_widget_get_style_context(button);
  gtk_style_context_add_class(stylecontext, "tagbutton");
  g_signal_connect(button, "clicked", G_CALLBACK(tagbuttononclick), GINT_TO_POINTER(tagnum));
  return button;
}

void update_tag_buttons(int currenttag) {
  g_print("running update_tag_buttons\n");
  for (int i = 0; i < NUMTAGS; i++) {
    GtkWidget *button = tagbuttons[i];
    GtkStyleContext *stylecontext = gtk_widget_get_style_context(button);
    if (i == currenttag) {
      gtk_style_context_add_class(stylecontext, "tagbutton-active");
    } else {
      gtk_style_context_remove_class(stylecontext, "tagbutton-active");
    }
  }
}

GtkWidget *metric_new(const gchar* label_text) {
  GtkWidget *metric_box;
  GtkWidget *label;

  metric_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  label = gtk_label_new(label_text);
  gtk_container_add(GTK_CONTAINER(metric_box), label);

  return metric_box;
}
