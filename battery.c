#include <gtk/gtk.h>

#define BATTERY_PATH "/sys/class/power_supply/BAT0/"
#define BATTERY_CAPACITY_PATH BATTERY_PATH "/capacity"
#define BATTERY_STATUS_PATH BATTERY_PATH "/status"

GFile *capacity_file;
GFile *status_file;
GFileMonitor *capacity_monitor;
GFileMonitor *status_monitor;
GtkWidget *battery_widget;

static void update_battery_widget() {
  char *capacity_contents = NULL, *status_contents = NULL;
  gsize capacity_length, status_length;
  int capacity;
  char status[20];
  char buffer[50];

  if (g_file_load_contents(capacity_file, NULL, &capacity_contents,
                           &capacity_length, NULL, NULL)) {
    capacity = atoi(capacity_contents);
    g_free(capacity_contents);
  } else {
    capacity = -1; // Error reading capacity
  }

  if (g_file_load_contents(status_file, NULL, &status_contents, &status_length,
                           NULL, NULL)) {
    g_strlcpy(status, status_contents, sizeof(status));
    g_strchomp(status); // Remove trailing newline
    g_free(status_contents);
  } else {
    g_strlcpy(status, "Unknown", sizeof(status));
  }

  const char *icon_name;

  if (g_strcmp0(status, "Charging") == 0) {
    icon_name = "battery-good-charging-symbolic";
  } else if (capacity > 80) {
    icon_name = "battery-full-symbolic";
  } else if (capacity > 50) {
    icon_name = "battery-good-symbolic";
  } else if (capacity > 20) {
    icon_name = "battery-low-symbolic";
  } else {
    icon_name = "battery-empty-symbolic";
  }

  gtk_image_set_from_icon_name(GTK_IMAGE(battery_widget), icon_name,
                               GTK_ICON_SIZE_MENU);
}

void init_battery(GtkWidget *battery_widget_arg) {
  capacity_file = g_file_new_for_path(BATTERY_CAPACITY_PATH);
  status_file = g_file_new_for_path(BATTERY_STATUS_PATH);
  capacity_monitor =
      g_file_monitor_file(capacity_file, G_FILE_MONITOR_NONE, NULL, NULL);
  status_monitor =
      g_file_monitor_file(status_file, G_FILE_MONITOR_NONE, NULL, NULL);
  g_signal_connect(capacity_monitor, "changed",
                   G_CALLBACK(update_battery_widget), NULL);
  g_signal_connect(status_monitor, "changed", G_CALLBACK(update_battery_widget),
                   NULL);
  battery_widget = battery_widget_arg;
  update_battery_widget();
}
