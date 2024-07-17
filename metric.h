#ifndef __METRIC_H__
#define __METRIC_H__
#include <gtk/gtk.h>
typedef struct {
  GtkWidget *label;
  GtkWidget *scale;
  GtkWidget *revealer;
  GtkWidget *metric_widget;
} Metric;
Metric *metric_new(GtkWidget *label, GtkWidget *scale, GtkWidget *revealer);
#endif
