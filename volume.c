#include <gtk/gtk.h>
#include <pulse/pulseaudio.h>
#include <pulse/glib-mainloop.h>

static pa_context *context = NULL;
static pa_glib_mainloop *glib_mainloop = NULL;
static pa_mainloop_api *mainloop_api = NULL;
static gboolean volume_changing = FALSE;
static GtkWidget *volume_scale = NULL;

static void init_pa();

void connect_scale_pa(GtkWidget *scale) {
  volume_scale = scale;
  init_pa();
};

static void update_volume_scale(pa_volume_t volume) {
  gdouble volume_percent = (gdouble)volume * 100.0 / PA_VOLUME_NORM;
  volume_changing = TRUE;
  gtk_range_set_value(GTK_RANGE(volume_scale), volume_percent);
  volume_changing = FALSE;
}

static void sink_info_callback(pa_context *c, const pa_sink_info *i, int eol,
                               void *userdata) {
  if (eol > 0)
    return;

  if (i) {
    pa_volume_t avg_volume = pa_cvolume_avg(&i->volume);
    update_volume_scale(avg_volume);
  }
}

static void subscribe_callback(pa_context *c, pa_subscription_event_type_t t,
                               uint32_t idx, void *userdata) {
  if ((t & PA_SUBSCRIPTION_EVENT_FACILITY_MASK) == PA_SUBSCRIPTION_EVENT_SINK) {
    pa_operation_unref(
        pa_context_get_sink_info_by_index(c, idx, sink_info_callback, NULL));
  }
}

static void context_state_callback(pa_context *c, void *userdata) {
  switch (pa_context_get_state(c)) {
  case PA_CONTEXT_READY:
    pa_operation_unref(
        pa_context_subscribe(c, PA_SUBSCRIPTION_MASK_SINK, NULL, NULL));
    pa_context_set_subscribe_callback(c, subscribe_callback, NULL);
    pa_operation_unref(
        pa_context_get_sink_info_by_index(c, 0, sink_info_callback, NULL));
    break;
  case PA_CONTEXT_FAILED:
    g_print("Connection failed\n");
    break;
  case PA_CONTEXT_TERMINATED:
    g_print("Connection terminated\n");
    break;
  default:
    break;
  }
}

static void init_pa() {
  glib_mainloop = pa_glib_mainloop_new(g_main_context_default());
  mainloop_api = pa_glib_mainloop_get_api(glib_mainloop);
  context = pa_context_new(mainloop_api, "sgbar");
  pa_context_connect(context, NULL, PA_CONTEXT_NOFLAGS, NULL);
  pa_context_set_state_callback(context, context_state_callback, NULL);
  pa_context_connect(context, NULL, 0, NULL);
}
