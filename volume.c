#include <gtk/gtk.h>
#include <pulse/glib-mainloop.h>
#include <pulse/pulseaudio.h>

#define VOLUME_TOOLTIP_FORMAT_STR "Volume: %d%%"

static pa_context *context = NULL;
static pa_glib_mainloop *glib_mainloop = NULL;
static pa_mainloop_api *mainloop_api = NULL;
static gboolean volume_changing = FALSE;
static GtkWidget *volume_scale = NULL;
static GtkWidget *revealer = NULL;
static GtkWidget *label = NULL;
static guint timeout_id = 0;
static gboolean first_run = TRUE;
static int prev_volume = -1;

static void init_pa();

gboolean is_volume_changing() { return volume_changing; }

void connect_widgets_pa(GtkWidget *label_arg, GtkWidget *scale_arg, GtkWidget *revealer_arg) {
  label = label_arg;
  volume_scale = scale_arg;
  revealer = revealer_arg;
  init_pa();
};

static gboolean revealer_unreveal_callback(gpointer data) {
  gtk_revealer_set_reveal_child(GTK_REVEALER(data), FALSE);
  timeout_id = 0;
  return FALSE;
}

static void update_volume_scale(pa_volume_t volume) {
  gint volume_percent = volume * 100.0 / PA_VOLUME_NORM;
  volume_changing = TRUE;
  gtk_range_set_value(GTK_RANGE(volume_scale), volume_percent);
  if (!first_run)
    gtk_revealer_set_reveal_child(GTK_REVEALER(revealer), TRUE);
  if (timeout_id)
    g_source_remove(timeout_id);
  timeout_id = g_timeout_add(800, revealer_unreveal_callback, revealer);
  GString *str = g_string_new(NULL);
  char *tooltip_text = g_strdup_printf(VOLUME_TOOLTIP_FORMAT_STR, volume_percent);
  gtk_widget_set_tooltip_text(label, tooltip_text);
  g_free(tooltip_text);
  volume_changing = FALSE;
  first_run = FALSE;
}

void set_volume(int volume_percent) {
  if (volume_percent < 0 || volume_percent > 100) {
    g_print("Volume must be between 0 and 100\n");
    return;
  }

  pa_volume_t pa_volume =
      (pa_volume_t)((double)volume_percent / 100.0 * PA_VOLUME_NORM);
  pa_cvolume cvolume;
  pa_cvolume_set(&cvolume, 2, pa_volume); // Assuming stereo

  pa_operation *op =
      pa_context_set_sink_volume_by_index(context, 0, &cvolume, NULL, NULL);
  if (op) {
    pa_operation_unref(op);
  } else {
    g_print("Failed to set volume\n");
  }
}

static void sink_info_callback(pa_context *c, const pa_sink_info *i, int eol,
                               void *userdata) {
  if (eol > 0)
    return;

  if (i) {
    pa_volume_t volume = pa_cvolume_avg(&i->volume);
    if (prev_volume == volume) {
      return;
    }

    update_volume_scale(volume);
    prev_volume = volume;
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
