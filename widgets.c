#include "widgets.h"
#include "config.h"
#include "volume.h"
#include <gtk/gtk.h>

// Contains custom widget stuff
GtkWidget *tagbuttons[NUMTAGS];
GtkWidget *volume;
GtkWidget *brightness;
GtkWidget *battery;

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

static void on_scale_value_changed(GtkRange *range, gpointer data) {
  if (is_volume_changing())
    return;

  gdouble value = gtk_range_get_value(range);
  set_volume((int)value);
}

static gboolean hide_revealer(gpointer data) {
  MetricData *metricdata = (MetricData *)data;
  gtk_revealer_set_reveal_child(GTK_REVEALER(metricdata->revealer), FALSE);
  metricdata->timeout_id = 0;
  return G_SOURCE_REMOVE;
}

GtkWidget *volume_widget_new() {
  GtkWidget *scale =
      gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0, 100, 1);
  GtkWidget *revealer = gtk_revealer_new();
  GtkWidget *label = gtk_label_new(volumeicon);

  connect_widgets_pa(label, scale, revealer);
  g_signal_connect(scale, "value-changed", G_CALLBACK(on_scale_value_changed),
                   NULL);

  return metric_new(label, scale, revealer);
}

GtkWidget *brightess_widget_new() {
  GtkWidget *scale =
      gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0, 100, 1);
  GtkWidget *revealer = gtk_revealer_new();
  GtkWidget *label = gtk_label_new(brightnessicon);

  return metric_new(label, scale, revealer);
};

GtkWidget *battery_widget_new() {
  GtkWidget *battery = gtk_image_new();

}

static void metric_on_enter(GtkWidget *widget, GdkEventCrossing *event,
                            gpointer data) {

  MetricData *metricdata = (MetricData *)data;

  if (metricdata->timeout_id) {
    g_source_remove(metricdata->timeout_id);
    metricdata->timeout_id = 0;
  }

  gtk_revealer_set_reveal_child(GTK_REVEALER(metricdata->revealer), TRUE);
}

static void metric_on_leave(GtkWidget *widget, GdkEventCrossing *event,
                            gpointer data) {
  MetricData *metricdata = (MetricData *)data;

  if (metricdata->timeout_id == 0) {
    metricdata->timeout_id = g_timeout_add(500, hide_revealer, metricdata);
  }
}

static void metric_data_free(gpointer data, GClosure *closure) { g_free(data); }

GtkWidget *metric_new(GtkWidget *label, GtkWidget *scale, GtkWidget *revealer) {
  GtkWidget *mainbox;
  GtkWidget *event_box;
  GtkWidget *scaleeventbox;
  GtkWidget *box;
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
  gtk_revealer_set_transition_type(GTK_REVEALER(revealer),
                                   GTK_REVEALER_TRANSITION_TYPE_SLIDE_RIGHT);
  gtk_revealer_set_transition_duration(GTK_REVEALER(revealer), 500);

  gtk_container_add(GTK_CONTAINER(box), label);
  gtk_container_add(GTK_CONTAINER(box), revealer);
  gtk_container_add(GTK_CONTAINER(event_box), box);

  box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

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
