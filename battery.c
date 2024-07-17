#include <gtk/gtk.h>

#define BATTERY_PATH "/sys/class/power_supply/BAT0/"
#define BATTERY_CAPACITY_PATH BATTERY_PATH "capacity"
#define BATTERY_STATUS_PATH BATTERY_PATH "status"
#define BATTERY_TOOLTIP_FORMAT_STR "Battery: %d%% (%s)"

static GFile *capacity_file;
static GFile *status_file;
static GFileMonitor *capacity_monitor;
static GFileMonitor *status_monitor;
static gboolean battery_module_initialized = FALSE;

static gboolean update_battery_widget(gpointer data) {
  GtkWidget *battery_widget = (GtkWidget *) data;
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

  char* tooltip_text = g_strdup_printf(BATTERY_TOOLTIP_FORMAT_STR, capacity, status);
  gtk_widget_set_tooltip_text(battery_widget, tooltip_text);
  return G_SOURCE_CONTINUE;
}

static void init_battery() {
  capacity_file = g_file_new_for_path(BATTERY_CAPACITY_PATH);
  status_file = g_file_new_for_path(BATTERY_STATUS_PATH);
  g_print(BATTERY_STATUS_PATH"\n");
  capacity_monitor =
      g_file_monitor_file(capacity_file, G_FILE_MONITOR_NONE, NULL, NULL);
  status_monitor =
      g_file_monitor_file(status_file, G_FILE_MONITOR_NONE, NULL, NULL);

  if (!capacity_monitor || !status_monitor) {
    g_print("Failed to set up file monitors\n");
    return;
  }

  char *capacity_contents = NULL, *status_contents = NULL;
  gsize capacity_length, status_length;
  int capacity;
  char status[20];
  char buffer[50];

  g_file_load_contents(status_file, NULL, &capacity_contents,
                       &capacity_length, NULL, NULL);
  g_print("capacity_contents: %s\n", capacity_contents);
  battery_module_initialized = TRUE;
}

GtkWidget *battery_widget_new() {
  GtkWidget *battery = gtk_image_new();
  if (!battery_module_initialized)
    init_battery();

  update_battery_widget(battery);

  // Unfortunately from what i see we can't seem to monitor battery maybe since it's managed by the kernel
  // TODO: Update, maybe we can acpi to monitor
  g_timeout_add_seconds(5, (GSourceFunc)update_battery_widget, battery);

  return battery;
}
