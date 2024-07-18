#include <gtk/gtk.h>
#include <string.h>

typedef struct {
  GtkWidget *clock_widget;
  gchar *format_str;
} ClockUpdateArgs;


static void on_clock_button_clicked(GtkWidget *button, gpointer popup) {
  if (gtk_widget_get_visible(popup)) {
    gtk_widget_hide(popup);
  } else {
    GtkAllocation button_allocation;

    gint window_x, window_y;
    gint popup_width, popup_height;

    GtkWidget *main_window = gtk_widget_get_toplevel(button);

    gtk_widget_get_allocation(button, &button_allocation);
    gdk_window_get_origin(gtk_widget_get_window(main_window), &window_x,
                          &window_y);

    gtk_window_get_size(GTK_WINDOW(popup), &popup_width, &popup_height);
    // TODO: Don't hard code values
    int popup_x = 850;
    g_print("button_x %d", button_allocation.x);
    g_print("popup_wdith: %d\n", popup_width);
    int popup_y =
        window_y - button_allocation.y - button_allocation.height - 128;
    g_print("popup_y %d\n", popup_y);
    g_print("window_y %d\n", window_y);
    g_print("button_allocation.y %d\n", button_allocation.y);
    g_print("button_allocation.height %d\n", button_allocation.height);
    g_print("popup_height %d\n", popup_height);

    gtk_window_move(GTK_WINDOW(popup), popup_x, popup_y);
    gtk_widget_show_all(popup);
  }
}

static void format_ordinal_suffix(gchar* str, gint day) {
  gchar *pos = strstr(str, "%^");
  if (pos != NULL) {
    gchar *suffix;
    if (day >= 11 && day <= 13) {
      suffix = "th";
    } else {
      switch (day % 10) {
        case 1:
          suffix = "st";
          break;
        case 2:
          suffix = "nd";
          break;
        case 3:
          suffix = "rd";
          break;
        default:
          suffix = "th";
          break;
      }
    }
    memcpy(pos, suffix, 2);
  }
}

static gboolean update_clock_widget(gpointer user_data) {
  ClockUpdateArgs *args = (ClockUpdateArgs *)user_data;
  GtkWidget *clock_widget = args->clock_widget;
  gchar *format_str = args->format_str;
  time_t rawtime;
  struct tm *timeinfo;
  char buffer[50];

  time(&rawtime);
  timeinfo = localtime(&rawtime);

  strftime(buffer, sizeof(buffer), format_str, timeinfo);
  format_ordinal_suffix(buffer, timeinfo->tm_mday);
  gtk_button_set_label(GTK_BUTTON(clock_widget), buffer);

  return G_SOURCE_CONTINUE;
}

GtkWidget *create_popup_window(GtkWidget *parent_window) {
  GtkWidget *popup = gtk_window_new(GTK_WINDOW_POPUP);
  gtk_window_set_type_hint(GTK_WINDOW(popup), GDK_WINDOW_TYPE_HINT_POPUP_MENU);
  gtk_window_set_transient_for(GTK_WINDOW(popup), GTK_WINDOW(parent_window));

  GtkWidget *calendar = gtk_calendar_new();
  gtk_container_add(GTK_CONTAINER(popup), calendar);

  return popup;
}

GtkWidget *clock_widget_new(GtkWidget *parent_window, gchar* format_str) {
  GtkWidget *clock_widget = gtk_button_new();
  
  GtkWidget *popup = create_popup_window(parent_window);
  ClockUpdateArgs *args = g_new(ClockUpdateArgs, 1);
  args->clock_widget = clock_widget;
  args->format_str = format_str;

  g_signal_connect(clock_widget, "clicked", G_CALLBACK(on_clock_button_clicked), popup);

  GtkStyleContext *style = gtk_widget_get_style_context(clock_widget);
  gtk_style_context_add_class(style, "textclock");
  g_timeout_add_seconds(1, (GSourceFunc)update_clock_widget, args);
  return clock_widget;
}
