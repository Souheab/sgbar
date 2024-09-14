#include "battery.h"
#include <gtk/gtk.h>

#define BATTERY_BASE_PATH "/sys/class/power_supply/"
#define BATTERY_TOOLTIP_FORMAT_STR "Battery: %d%% (%s)"
#define BATTERY_UPDATE_INTERVAL 1500

static BatteryController *battery_controller_singleton;
static gboolean battery_controller_initialized = FALSE;

static void on_battery_file_changed(GFileMonitor *monitor, GFile *file,
                                    GFile *other_file,
                                    GFileMonitorEvent event_type,
                                    gpointer user_data);

static void battery_model_update_from_files(BatteryModel *battery_model);
static void update_battery_view(BatteryView *view);
static void battery_controller_on_battery_model_updated(BatteryController *battery_controller);
static gboolean update_battery_timer_callback(gpointer user_data);

// HElpers:
static char* find_battery_path() {
    const char* battery_names[] = {"BAT0", "BAT1"};
    char* full_path = NULL;

    for (int i = 0; i < G_N_ELEMENTS(battery_names); i++) {
        full_path = g_build_filename(BATTERY_BASE_PATH, battery_names[i], NULL);
        if (g_file_test(full_path, G_FILE_TEST_EXISTS)) {
            return full_path;
        }
        g_free(full_path);
    }

    return NULL;
}


static BatteryModel *battery_model_new() {
    BatteryModel *battery_model = g_new(BatteryModel, 1);

    char* battery_path = find_battery_path();

    char *capacity_path = g_build_filename(battery_path, "capacity", NULL);
    char *status_path = g_build_filename(battery_path, "status", NULL);

    g_free(battery_path);

    battery_model->capacity_file = g_file_new_for_path(capacity_path);
    battery_model->status_file = g_file_new_for_path(status_path);
    battery_model->capacity_monitor = g_file_monitor_file(battery_model->capacity_file, G_FILE_MONITOR_NONE, NULL, NULL);
    battery_model->status_monitor = g_file_monitor_file(battery_model->status_file, G_FILE_MONITOR_NONE, NULL, NULL);

    g_signal_connect(battery_model->capacity_monitor, "changed", G_CALLBACK(on_battery_file_changed), battery_model);
    g_signal_connect(battery_model->status_monitor, "changed", G_CALLBACK(on_battery_file_changed), battery_model);

    battery_model_update_from_files(battery_model);

    g_timeout_add(BATTERY_UPDATE_INTERVAL, update_battery_timer_callback, battery_model);

    return battery_model;
}

static void battery_model_update_from_files(BatteryModel *battery_model) {
    gchar *capacity_contents = NULL, *status_contents = NULL;
    gsize capacity_length, status_length;

    if (g_file_load_contents(battery_model->capacity_file, NULL, &capacity_contents, &capacity_length, NULL, NULL)) {
      battery_model->capacity = atoi(capacity_contents);
      g_free(capacity_contents);
    } else {
      battery_model->capacity = -1; // Error reading capacity
    }

    if (g_file_load_contents(battery_model->status_file, NULL, &status_contents, &status_length, NULL, NULL)) {
      g_strlcpy(battery_model->status, status_contents, sizeof(battery_model->status));
      g_strchomp(battery_model->status); // Remove trailing newline
      g_free(status_contents);
    } else {
      g_strlcpy(battery_model->status, "Unknown", sizeof(battery_model->status));
    }

    battery_controller_on_battery_model_updated(battery_controller_singleton);
}

// Battery Controller functions:
static BatteryController *battery_controller_new() {
    BatteryController *battery_controller = g_new(BatteryController, 1);
    battery_controller_singleton = battery_controller;
    battery_controller->num_of_battery_views = 0;
    battery_controller->num_of_battery_views_allocated = 1;
    battery_controller->battery_views = g_new(BatteryView *, 1);
    battery_controller->battery_model_singleton = battery_model_new();
    return battery_controller;
}

BatteryController *get_battery_controller() {
    if (!battery_controller_initialized) {
        battery_controller_new();
        battery_controller_initialized = TRUE;
    }
    return battery_controller_singleton;
}


static void battery_controller_on_battery_model_updated(BatteryController *battery_controller) {
    for (int i = 0; i < battery_controller->num_of_battery_views; i++) {
        BatteryView *view = battery_controller->battery_views[i];
        update_battery_view(view);
    }
}

BatteryView *battery_controller_get_new_view(BatteryController *battery_controller) {
  GtkWidget *battery_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
  GtkWidget *battery = gtk_image_new();
  GtkWidget *label = gtk_label_new("");
  GtkStyleContext *style = gtk_widget_get_style_context(battery);
  gtk_style_context_add_class(style, "battery-indicator");
  style = gtk_widget_get_style_context(label);
  gtk_style_context_add_class(style, "battery-label");


  gtk_box_pack_start(GTK_BOX(battery_box), battery, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(battery_box), label, FALSE, FALSE, 0);

  BatteryView *battery_view = g_new(BatteryView, 1);
  battery_view->battery_box = battery_box;
  battery_view->battery = battery;
  battery_view->label = label;

  if (battery_controller->num_of_battery_views >= battery_controller->num_of_battery_views_allocated) {
    battery_controller->num_of_battery_views_allocated *= 2;
      battery_controller->battery_views = g_realloc(battery_controller->battery_views,
                                                    battery_controller->num_of_battery_views_allocated * sizeof(BatteryView *));
  }
  battery_controller->battery_views[battery_controller->num_of_battery_views] = battery_view;
  battery_controller->num_of_battery_views++;

  update_battery_view(battery_view);
  return battery_view;
}

// View functions:
static void update_battery_view(BatteryView *view) {
    BatteryModel *model = battery_controller_singleton->battery_model_singleton;
    
    const char *icon_name;
    if (g_strcmp0(model->status, "Charging") == 0) {
        icon_name = "battery-good-charging-symbolic";
    } else if (model->capacity > 80) {
        icon_name = "battery-full-symbolic";
    } else if (model->capacity > 50) {
        icon_name = "battery-good-symbolic";
    } else if (model->capacity > 20) {
        icon_name = "battery-low-symbolic";
    } else {
        icon_name = "battery-empty-symbolic";
    }

    gtk_image_set_from_icon_name(GTK_IMAGE(view->battery), icon_name, GTK_ICON_SIZE_MENU);

    char *label_text = g_strdup_printf("%d%%", model->capacity);
    gtk_label_set_text(GTK_LABEL(view->label), label_text);
    g_free(label_text);

    char *tooltip_text = g_strdup_printf(BATTERY_TOOLTIP_FORMAT_STR, model->capacity, model->status);
    gtk_widget_set_tooltip_text(view->battery_box, tooltip_text);
    g_free(tooltip_text);
}

// Callback functions
static void on_battery_file_changed(GFileMonitor *monitor, GFile *file,
                                    GFile *other_file,
                                    GFileMonitorEvent event_type,
                                    gpointer user_data) {
    BatteryModel *model = (BatteryModel *) user_data;
    battery_model_update_from_files(model);
}

static gboolean update_battery_timer_callback(gpointer user_data) {
  BatteryModel *model = (BatteryModel *)user_data;
  battery_model_update_from_files(model);
  return G_SOURCE_CONTINUE;
}


