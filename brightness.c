#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <gtk/gtk.h>

#define BACKLIGHT_PATH "/sys/class/backlight"
#define BRIGHTNESS_TOOLTIP_FORMAT_STR "Brightness: %d%%"
static GtkWidget *brightness_scale;
static GtkWidget *brightness_label;
static GtkWidget *brightness_revealer;
static GFile *brightness_file;
static GFileMonitor *brightness_monitor;
static int max_brightness;
static guint timeout_id;
static gboolean first_run = TRUE;

char *find_backlight_device() {
  DIR *dir;
  struct dirent *entry;
  static char device_path[256];

  dir = opendir(BACKLIGHT_PATH);
  if (dir == NULL) {
    perror("Cannot open backlight directory");
    return NULL;
  }

  while ((entry = readdir(dir)) != NULL) {
    printf("Found entry: %s\n", entry->d_name);
    printf("Entry type: %d\n", entry->d_type);
    if ((entry->d_type == DT_DIR || entry->d_type == DT_LNK) && strcmp(entry->d_name, ".") != 0 &&
        strcmp(entry->d_name, "..") != 0) {
      printf("Found backlight device: %s\n", device_path);
      snprintf(device_path, sizeof(device_path), "%s/%s", BACKLIGHT_PATH,
               entry->d_name);
      printf("Found backlight device: %s\n", device_path);
      closedir(dir);
      return device_path;
    }
  }

  closedir(dir);
  return NULL;
}

int read_value(const char *device_path, const char *file_name) {
  char path[512];
  FILE *file;
  int value;

  snprintf(path, sizeof(path), "%s/%s", device_path, file_name);
  file = fopen(path, "r");
  if (file == NULL) {
    perror("Cannot open file");
    return -1;
  }

  if (fscanf(file, "%d", &value) != 1) {
    fclose(file);
    return -1;
  }

  fclose(file);
  return value;
}

static gboolean revealer_unreveal_callback(gpointer data) {
  gtk_revealer_set_reveal_child(GTK_REVEALER(data), FALSE);
  timeout_id = 0;
  return FALSE;
}

static void update_brightness_widget() {
  char *contents;
  if (!g_file_load_contents(brightness_file, NULL, &contents, NULL, NULL, NULL)) {
    g_print("Cannot read brightness file\n");
    return;
  }
  int brightness = atoi(contents);
  printf("Brightness: %d\n", brightness);
  int percentage = ((double)brightness / max_brightness)*100;
  printf("Brightness Percentage: %i", percentage);
  gtk_range_set_value(GTK_RANGE(brightness_scale), percentage);
  if (!first_run)
    gtk_revealer_set_reveal_child(GTK_REVEALER(brightness_revealer), TRUE);
  if (timeout_id)
    g_source_remove(timeout_id);
  timeout_id = g_timeout_add(800, revealer_unreveal_callback, brightness_revealer);
  GString *str = g_string_new(NULL);
  char *tooltip_text = g_strdup_printf(BRIGHTNESS_TOOLTIP_FORMAT_STR, percentage);
  gtk_widget_set_tooltip_text(brightness_label, tooltip_text);
  g_free(tooltip_text);
  first_run = FALSE;
}

void init_brightness(GtkWidget *label_widget, GtkWidget *scale_widget, GtkWidget *revealer_widget) {
  brightness_scale = scale_widget;
  brightness_label = label_widget;
  brightness_revealer = revealer_widget;
  char *device_path = find_backlight_device();
  if (device_path == NULL) {
    g_print("Cannot find backlight device\n");
    return;
  }

  char *brightness_file_path = g_strconcat(device_path, "/brightness", NULL);
  brightness_file = g_file_new_for_path(brightness_file_path);
  brightness_monitor = g_file_monitor_file(brightness_file, G_FILE_MONITOR_NONE, NULL, NULL);
  g_free(brightness_file_path);

  char *max_brightness_file_path = g_strconcat(device_path, "/max_brightness", NULL);
  GFile *max_brightness_file = g_file_new_for_path(max_brightness_file_path);
  max_brightness = read_value(device_path, "max_brightness");
  g_free(max_brightness_file_path);

  g_signal_connect(brightness_monitor, "changed", G_CALLBACK(update_brightness_widget), NULL);

}
