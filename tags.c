#include "config.h"
#include "x.h"
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <X11/Xlib.h>

GtkWidget *tagbuttons[NUMTAGS];

void on_tag_button_right_click(GtkWidget *widget, GdkEventButton *event, gpointer data) {
  Display *dpy = GDK_DISPLAY_XDISPLAY(gdk_display_get_default());
  gint button_number = GPOINTER_TO_INT(data);
  g_print("butnum %d\n", button_number);

  if (dpy == NULL || tagmask == -1)
    return;

  Window root = DefaultRootWindow(dpy);
  gint bit_mask = 1 << button_number;
  tagmask |= bit_mask;
  char tagmask_str[32];
  snprintf(tagmask_str, sizeof(tagmask_str), "%d", tagmask);
  g_print("tagmask_updated %s \n", tagmask_str);
  XChangeProperty(dpy, root, dwm_atoms[DwmSetTags], utf8_string, 8, PropModeReplace, (unsigned char *)tagmask_str, strlen(tagmask_str));
  XFlush(dpy);
}

static GtkWidget *tag_button_new(gint tagnum) {
  GtkWidget *button;
  GtkStyleContext *stylecontext;
  button = gtk_button_new_with_label(taglabel);
  g_signal_connect(button, "button-press-event", G_CALLBACK(on_tag_button_right_click),  GINT_TO_POINTER(tagnum));
  stylecontext = gtk_widget_get_style_context(button);
  gtk_style_context_add_class(stylecontext, "tagbutton");
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
