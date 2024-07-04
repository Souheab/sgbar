#include "config.h"
#include "volume.h"
#include "widgets.h"
#include <gtk/gtk.h>

// Contains custom widget stuff
GtkWidget *tagbuttons[NUMTAGS];
GtkWidget *volume;

GtkWidget *tag_button_new(int tagnum) {
  GtkWidget *button;
  GtkStyleContext *stylecontext;
  button = gtk_button_new_with_label(taglabel);
  stylecontext = gtk_widget_get_style_context(button);
  gtk_style_context_add_class(stylecontext, "tagbutton");
  g_signal_connect(button, "clicked", G_CALLBACK(tagbuttononclick),
                   GINT_TO_POINTER(tagnum));
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

static gboolean hide_revealer(gpointer data) {
  MetricData *metricdata = (MetricData *)data;
  gtk_revealer_set_reveal_child(GTK_REVEALER(metricdata->revealer), FALSE);
  metricdata->timeout_id = 0;
  return G_SOURCE_REMOVE;
}

void revealer_reveal_with_timeout(GtkWidget *revealer, MetricData *metricdata) {
  if (metricdata->timeout_id) {
    g_source_remove(metricdata->timeout_id);
    metricdata->timeout_id = 0;
  }

  gtk_revealer_set_reveal_child(GTK_REVEALER(metricdata->revealer), TRUE);
}

static void metric_on_enter(GtkWidget *widget, GdkEventCrossing *event, gpointer data) {
  g_print("hovering over metric\n");
  revealer_reveal_with_timeout(widget, data);
}

static void metric_on_leave(GtkWidget *widget, GdkEventCrossing *event,
                            gpointer data) {
  MetricData *metricdata = (MetricData *)data;
  g_print("leaving metric\n");

  if (metricdata->timeout_id == 0) {
    metricdata->timeout_id = g_timeout_add(500, hide_revealer, metricdata);
  }
}

static void metric_data_free(gpointer data, GClosure *closure) { g_free(data); }

GtkWidget *metric_new(const gchar *label_text) {
  GtkWidget *mainbox;
  GtkWidget *event_box;
  GtkWidget *scaleeventbox;
  GtkWidget *box;
  GtkWidget *label;
  GtkWidget *revealer;
  GtkWidget *scale;
  GtkStyleContext *stylecontext;
  MetricData *data;

  mainbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  event_box = gtk_event_box_new();

  gtk_container_add(GTK_CONTAINER(mainbox), event_box);

  stylecontext = gtk_widget_get_style_context(event_box);
  gtk_style_context_add_class(stylecontext, "metric");
  box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  stylecontext = gtk_widget_get_style_context(box);

  gtk_style_context_add_class(stylecontext, "metricbox");
  label = gtk_label_new(label_text);
  revealer = gtk_revealer_new();
  gtk_revealer_set_transition_type(GTK_REVEALER(revealer),
                                   GTK_REVEALER_TRANSITION_TYPE_SLIDE_RIGHT);
  gtk_revealer_set_transition_duration(GTK_REVEALER(revealer), 500);

  gtk_container_add(GTK_CONTAINER(box), label);
  gtk_container_add(GTK_CONTAINER(box), revealer);
  gtk_container_add(GTK_CONTAINER(event_box), box);

  box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0, 100, 1);
  connect_widgets_pa(scale, revealer);
  gtk_scale_set_draw_value(GTK_SCALE(scale), FALSE);
  gtk_widget_set_size_request(scale, 100, -1);

  scaleeventbox = gtk_event_box_new();
  gtk_container_add(GTK_CONTAINER(box), scale);
  gtk_container_add(GTK_CONTAINER(scaleeventbox), box);
  gtk_container_add(GTK_CONTAINER(revealer), scaleeventbox);

  data = g_new(MetricData, 1);
  data->revealer = revealer;
  data->timeout_id = 0;

  g_signal_connect_data(event_box, "enter-notify-event",
                        G_CALLBACK(metric_on_enter), data, metric_data_free, 0);
  g_signal_connect(event_box, "leave-notify-event", G_CALLBACK(metric_on_leave),
                   data);
  g_signal_connect(scaleeventbox, "enter-notify-event",
                   G_CALLBACK(metric_on_enter), data);
  g_signal_connect(scaleeventbox, "leave-notify-event",
                   G_CALLBACK(metric_on_leave), data);
  return mainbox;
}