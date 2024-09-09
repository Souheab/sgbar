#include <dirent.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "metric.h"
#include "config.h"
#include "brightness.h"

#define BACKLIGHT_PATH "/sys/class/backlight"
#define BRIGHTNESS_TOOLTIP_FORMAT_STR "Brightness: %d%%"

// Utility functions:

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

static void set_brightness(GFile *brightness_file, int value) {
  char *contents = g_strdup_printf("%d", value);
  const char *file_path = g_file_get_path(brightness_file);

  int fd = open(file_path, O_WRONLY);
  if (fd == -1) {
    g_warning("Failed to open file: %s (errno: %d, %s)", file_path, errno,
              strerror(errno));
    g_free(contents);
    return;
  }

  ssize_t bytes_written = write(fd, contents, strlen(contents));
  int close_result = close(fd);

  gboolean result = (bytes_written != -1 && close_result == 0);

  if (!result) {
    g_warning("Failed to write to file: %s (errno: %d, %s)", file_path, errno, strerror(errno));
  }

  g_free(contents);
}

// Model and controller stuff:

static BrightnessController *brightness_controller_singleton;
static gboolean brightness_controller_initialized = FALSE;

static void brightness_controller_on_brightness_model_updated(BrightnessController *brightness_controller);
static void update_brightness_view(Metric *view);
static void on_brightness_file_changed(GFileMonitor *monitor, GFile *file,
                           GFile *other_file,
                           GFileMonitorEvent event_type,
                           gpointer user_data);
static void on_brightness_scale_changed(GtkWidget *widget, gpointer data);

// Brightness Model functions:
static BrightnessModel *brightness_model_new() {
  BrightnessModel *brightness_model = g_new(BrightnessModel, 1);
  char *device_path = find_backlight_device();
  if (device_path == NULL) {
    g_print("Cannot find backlight device\n");
    return NULL;
  }

  GFileMonitor *brightness_monitor;
  GFile *brightness_file;
  
  char *brightness_file_path = g_strconcat(device_path, "/brightness", NULL);
  brightness_file = g_file_new_for_path(brightness_file_path);
  brightness_monitor = g_file_monitor_file(brightness_file, G_FILE_MONITOR_NONE, NULL, NULL);
  g_free(brightness_file_path);

  char *max_brightness_file_path = g_strconcat(device_path, "/max_brightness", NULL);
  GFile *max_brightness_file = g_file_new_for_path(max_brightness_file_path);
  int max_brightness = read_value(device_path, "max_brightness");

  brightness_model->brightness_monitor = brightness_monitor;
  brightness_model->brightness_file = brightness_file;
  brightness_model->brightness = read_value(device_path, "brightness");
  brightness_model->max_brightness = max_brightness;

  g_signal_connect(brightness_monitor, "changed", G_CALLBACK(on_brightness_file_changed), brightness_model);

  return brightness_model;
}

static void brightness_model_update_brightness_from_file(BrightnessModel *brightness_model) {
  g_print("Updated brightness from file\n");
  char *contents;
  if (!g_file_load_contents(brightness_model->brightness_file, NULL, &contents, NULL, NULL, NULL)) {
    g_print("Cannot read brightness file\n");
    return;
  }

  guint brightness = atoi(contents);
  int percentage = ((double)brightness / brightness_model->max_brightness) * 100;
  brightness_model->brightness = brightness;
  g_free(contents);
  brightness_controller_on_brightness_model_updated(brightness_controller_singleton);
}

static void brightness_model_set_brightness(BrightnessModel *brightness_model, guint brightness) {
  char *brightness_str = g_strdup_printf("%d", brightness);
  g_file_replace_contents(brightness_model->brightness_file, brightness_str, strlen(brightness_str), NULL, FALSE, G_FILE_CREATE_NONE, NULL, NULL, NULL);
  g_free(brightness_str);
}

static void brightness_model_set_brightness_as_percentage(BrightnessModel *brightness_model, guint brightness_percentage) {
  guint brightness = (brightness_percentage / 100.0) * brightness_model->max_brightness;
  char *brightness_str = g_strdup_printf("%d", brightness);
  g_file_replace_contents(brightness_model->brightness_file, brightness_str, strlen(brightness_str), NULL, FALSE, G_FILE_CREATE_NONE, NULL, NULL, NULL);
  g_free(brightness_str);
}

static BrightnessController *brightness_controller_new() {
  BrightnessController *brightness_controller = g_new(BrightnessController, 1);
  brightness_controller->brightness_model_singleton = brightness_model_new();
  brightness_controller->num_of_brightness_views = 0;
  brightness_controller->num_of_brightness_views_allocated = 1;
  brightness_controller->brightness_views = g_new(Metric *, 1);
  return brightness_controller;
}

BrightnessController *get_brightness_controller() {
  if (!brightness_controller_initialized) {
    brightness_controller_singleton = brightness_controller_new();
    brightness_controller_initialized = TRUE;
  }

  return brightness_controller_singleton;
}

static void brightness_controller_on_brightness_model_updated(BrightnessController *brightness_controller) {
  for (int i = 0; i < brightness_controller->num_of_brightness_views; i++) {
    Metric *view = brightness_controller->brightness_views[i];
    update_brightness_view(view);
  }
}

// View functions:
Metric *brightness_controller_get_new_view(BrightnessController *brightness_controller) {
  g_print("Creating new brightness view\n");
  GtkWidget *scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0, 100, 1);
  GtkWidget *revealer = gtk_revealer_new();
  GtkWidget *label = gtk_label_new(brightnessicon);

  Metric *metric = metric_new(label, scale, revealer);
  GtkStyleContext *style = gtk_widget_get_style_context(metric->metric_widget);
  gtk_style_context_add_class(style, "brightness-controller");

  if (brightness_controller->num_of_brightness_views >= brightness_controller->num_of_brightness_views_allocated) {
    brightness_controller->num_of_brightness_views_allocated *= 2;
    brightness_controller->brightness_views = g_realloc(brightness_controller->brightness_views,
                                                        brightness_controller->num_of_brightness_views_allocated * sizeof(GtkWidget *));
  }
  brightness_controller->brightness_views[brightness_controller->num_of_brightness_views] = metric;
  brightness_controller->num_of_brightness_views++;

  g_signal_connect(scale, "value-changed", G_CALLBACK(on_brightness_scale_changed), NULL);
  return metric;
}

void update_brightness_view(Metric *view) {
  BrightnessModel *model = brightness_controller_singleton->brightness_model_singleton;
  int percentage = ((double)model->brightness / model->max_brightness) * 100;
  gtk_range_set_value(GTK_RANGE(view->scale), percentage);
  gtk_label_set_text(GTK_LABEL(view->label), brightnessicon);
  char *tooltip_text = g_strdup_printf(BRIGHTNESS_TOOLTIP_FORMAT_STR, percentage);
  gtk_widget_set_tooltip_text(view->label, tooltip_text);
}

// Callback functions:
static void
on_brightness_file_changed(GFileMonitor *monitor, GFile *file,
                           GFile *other_file,
                           GFileMonitorEvent event_type,
                           gpointer user_data) {
  BrightnessModel *model = (BrightnessModel *) user_data;
  brightness_model_update_brightness_from_file(model);
}

static void on_brightness_scale_changed(GtkWidget *widget, gpointer data) {
  int value = gtk_range_get_value(GTK_RANGE(widget));
  g_print("Brightness modified with scale, setting to %d\n", value);
  brightness_model_set_brightness_as_percentage(brightness_controller_singleton->brightness_model_singleton, value);
}
