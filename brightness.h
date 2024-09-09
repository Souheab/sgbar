#ifndef __BRIGHTNESS_H__
#define __BRIGHTNESS_H__
#include <gtk/gtk.h>
#include "metric.h"
typedef struct {
  GFile *brightness_file;
  GFileMonitor *brightness_monitor;
  guint brightness;
  guint max_brightness;
  guint timeout_id;
} BrightnessModel;
typedef struct {
  BrightnessModel *brightness_model_singleton;
  Metric **brightness_views;
  guint num_of_brightness_views;
  guint num_of_brightness_views_allocated;
} BrightnessController;
BrightnessController *get_brightness_controller();
Metric *brightness_controller_get_new_view(BrightnessController *brightness_controller);
#endif
