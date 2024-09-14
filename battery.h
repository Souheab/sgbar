#ifndef __BATTERY_H__
#define __BATTERY_H__

#include <gtk/gtk.h>
typedef struct {
    GFile *capacity_file;
    GFile *status_file;
    GFileMonitor *capacity_monitor;
    GFileMonitor *status_monitor;
    gint capacity;
    gchar status[20];
} BatteryModel;
typedef struct {
  GtkWidget *battery_box;
  GtkWidget *battery;
  GtkWidget *label;
} BatteryView;
typedef struct {
    BatteryModel *battery_model_singleton;
    BatteryView **battery_views;
    guint num_of_battery_views;
    guint num_of_battery_views_allocated;
} BatteryController;
BatteryController *get_battery_controller();
BatteryView *battery_controller_get_new_view(BatteryController *battery_controller);
#endif
