#include <dirent.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "config.h"
#include "metric.h"

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
static gboolean brightness_changing = FALSE;

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
    if ((entry->d_type == DT_DIR || entry->d_type == DT_LNK) &&
        strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
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

static void set_brightness(int value) {
  brightness_changing = TRUE;
  char *contents = g_strdup_printf("%d", value);
  const char *file_path = g_file_get_path(brightness_file);

  int fd = open(file_path, O_WRONLY);
  if (fd == -1) {
    g_warning("Failed to open file: %s (errno: %d, %s)", file_path, errno,
              strerror(errno));
    g_free(contents);
    brightness_changing = FALSE;
    return;
  }

  ssize_t bytes_written = write(fd, contents, strlen(contents));
  int close_result = close(fd);

  gboolean result = (bytes_written != -1 && close_result == 0);

  if (!result) {
    g_warning("Failed to write to file: %s (errno: %d, %s)", file_path, errno,
              strerror(errno));
  }

  g_free(contents);
  brightness_changing = FALSE;
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
  printf("Updating brightness widget\n");
  char *contents;
  if (!g_file_load_contents(brightness_file, NULL, &contents, NULL, NULL,
                            NULL)) {
    g_print("Cannot read brightness file\n");
    return;
  }
  int brightness = atoi(contents);
  int percentage = ((double)brightness / max_brightness) * 100;
  gtk_range_set_value(GTK_RANGE(brightness_scale), percentage);
  if (!first_run)
    gtk_revealer_set_reveal_child(GTK_REVEALER(brightness_revealer), TRUE);
  if (timeout_id)
    g_source_remove(timeout_id);
  timeout_id =
      g_timeout_add(800, revealer_unreveal_callback, brightness_revealer);
  GString *str = g_string_new(NULL);
  char *tooltip_text =
      g_strdup_printf(BRIGHTNESS_TOOLTIP_FORMAT_STR, percentage);
  gtk_widget_set_tooltip_text(brightness_label, tooltip_text);
  g_free(tooltip_text);
  first_run = FALSE;
}

static void on_brightness_scale_changed(GtkWidget *widget, gpointer data) {
  if (brightness_changing)
    return;
  int value = gtk_range_get_value(GTK_RANGE(widget));
  int brightness = (value / 100.0) * max_brightness;
  set_brightness(brightness);
}

static void init_brightness(GtkWidget *label_widget, GtkWidget *scale_widget,
                     GtkWidget *revealer_widget) {
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
  brightness_monitor =
      g_file_monitor_file(brightness_file, G_FILE_MONITOR_NONE, NULL, NULL);
  g_free(brightness_file_path);

  char *max_brightness_file_path =
      g_strconcat(device_path, "/max_brightness", NULL);
  GFile *max_brightness_file = g_file_new_for_path(max_brightness_file_path);
  max_brightness = read_value(device_path, "max_brightness");
  g_free(max_brightness_file_path);

  update_brightness_widget();

  g_signal_connect(brightness_monitor, "changed",
                   G_CALLBACK(update_brightness_widget), NULL);
  g_signal_connect(brightness_scale, "value-changed",
                   G_CALLBACK(on_brightness_scale_changed), NULL);
}


GtkWidget *brightess_widget_new() {
  brightness_scale =
      gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0, 100, 1);
  brightness_revealer = gtk_revealer_new();
  brightness_label = gtk_label_new(brightnessicon);

  init_brightness(brightness_label, brightness_scale, brightness_revealer);

  return metric_new(brightness_label, brightness_scale, brightness_revealer);
};
