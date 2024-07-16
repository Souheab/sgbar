#include <gtk/gtk.h>
typedef struct {
  GtkWidget *revealer;
  guint timeout_id;
} MetricData;

static gboolean hide_revealer(gpointer data) {
  MetricData *metricdata = (MetricData *)data;
  gtk_revealer_set_reveal_child(GTK_REVEALER(metricdata->revealer), FALSE);
  metricdata->timeout_id = 0;
  return G_SOURCE_REMOVE;
}

static void metric_data_free(gpointer data, GClosure *closure) { g_free(data); }

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
