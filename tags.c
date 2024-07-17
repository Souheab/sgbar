#include <gtk/gtk.h>
#include "config.h"

GtkWidget *tagbuttons[NUMTAGS];

static GtkWidget *tag_button_new(int tagnum) {
  GtkWidget *button;
  GtkStyleContext *stylecontext;
  button = gtk_button_new_with_label(taglabel);
  stylecontext = gtk_widget_get_style_context(button);
  gtk_style_context_add_class(stylecontext, "tagbutton");
  g_signal_connect(button, "clicked", G_CALLBACK(tagbuttononclick),
                   GINT_TO_POINTER(tagnum));
  return button;
}

static gboolean
istagselected(int tagnum, int tagmask) {
  if (tagnum >= NUMTAGS)
    return FALSE;

  return (tagmask & (1 << tagnum)) != 0;
}

gboolean update_active_tag_buttons(int tagmask) {
  for (int i = 0; i < NUMTAGS; i++) {
    GtkWidget *button = tagbuttons[i];
    GtkStyleContext *stylecontext = gtk_widget_get_style_context(button);
    if (istagselected(i, tagmask)) {
      gtk_style_context_add_class(stylecontext, "tagbutton-active");
    } else {
      gtk_style_context_remove_class(stylecontext, "tagbutton-active");
    }
  }
  return G_SOURCE_REMOVE;
}

gboolean update_occupied_tag_buttons(int tagmask) {
  printf("update_occupied_tag_buttons\n");
  for (int i = 0; i < NUMTAGS; i++) {
    GtkWidget *button = tagbuttons[i];
    GtkStyleContext *stylecontext = gtk_widget_get_style_context(button);
    if (istagselected(i, tagmask)) {
      gtk_style_context_add_class(stylecontext, "tagbutton-occupied");
    } else {
      gtk_style_context_remove_class(stylecontext, "tagbutton-occupied");
    }
  }
  return G_SOURCE_REMOVE;
}

GtkWidget *tags_box_new() {
  GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  for (int i = 0; i < NUMTAGS; i++) {
    tagbuttons[i] = tag_button_new(i);
    gtk_container_add(GTK_CONTAINER(box), tagbuttons[i]);
  }
  return box;
}
