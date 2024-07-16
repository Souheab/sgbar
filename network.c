#include <gtk/gtk.h>
#include <NetworkManager.h>

static NMClient *client;
static gboolean client_initialized = FALSE;

gchar *get_strength_icon_name(guint8 strength) {
  if (strength > 80)
    return "network-wireless-signal-excellent-symbolic";
  else if (strength > 55)
    return "network-wireless-signal-good-symbolic";
  else if (strength > 30)
    return "network-wireless-signal-ok-symbolic";
  else if (strength > 5)
    return "network-wireless-signal-weak-symbolic";
  else
    return "network-wireless-signal-none-symbolic";
}

static GtkWidget *new_wifi_ap_menu_item(const gchar *ssid, guint8 strength) {
  GtkWidget *item, *box, *label, *icon;
  gchar *icon_name = get_strength_icon_name(strength);

  box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  label = gtk_label_new(ssid);
  gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, 0);


  icon = gtk_image_new_from_icon_name(icon_name, GTK_ICON_SIZE_MENU);
  gtk_box_pack_end(GTK_BOX(box), icon, FALSE, FALSE, 0);

  item = gtk_menu_item_new();
  gtk_container_add(GTK_CONTAINER(item), box);

  return item;
}

static GtkWidget *new_wifi_menu() {
  GtkWidget *connected_network_item;
  GtkWidget *available_networks_item;
  GtkWidget *menu = gtk_menu_new();
  GtkWidget *connected_network = gtk_menu_item_new_with_label("disconnected");
  gtk_widget_set_sensitive(connected_network, FALSE);
  gboolean connected_network_found = FALSE;
  gboolean is_wifi_enabled = FALSE;
  NMDeviceState state;

  connected_network_item = gtk_menu_item_new_with_label("Connected Network");
  gtk_widget_set_sensitive(connected_network_item, FALSE);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), connected_network_item);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), connected_network);
  available_networks_item = gtk_menu_item_new_with_label("Available Networks");
  gtk_widget_set_sensitive(available_networks_item, FALSE);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), available_networks_item);


  const GPtrArray *devices = nm_client_get_devices(client);

  for (guint i = 0; i < devices->len; i++) {
    NMDevice *device = g_ptr_array_index(devices, i);
    if (NM_IS_DEVICE_WIFI(device)) {
      if (nm_device_get_state(device) == NM_DEVICE_STATE_ACTIVATED) {
        is_wifi_enabled = TRUE;
      } else {
        continue;
      }
      if (!connected_network_found) {
        NMAccessPoint *ap = nm_device_wifi_get_active_access_point(NM_DEVICE_WIFI(device));
        if (ap != NULL) {
          GBytes *ssid_bytes = nm_access_point_get_ssid(ap);
          if (ssid_bytes != NULL) {
            char *ssid = nm_utils_ssid_to_utf8(g_bytes_get_data(ssid_bytes, NULL), g_bytes_get_size(ssid_bytes));
            gtk_menu_item_set_label(GTK_MENU_ITEM(connected_network), ssid);
            gtk_widget_set_sensitive(connected_network, TRUE);
            connected_network_found = TRUE;
          }
        }
      }
      const GPtrArray *aps = nm_device_wifi_get_access_points(NM_DEVICE_WIFI(device));
      for (guint j = 0; j < aps->len; j++) {
        if (aps == NULL) {
          continue;
        }
        NMAccessPoint *ap = g_ptr_array_index(aps, j);
        if (ap == NULL) {
          continue;
        }
        GBytes *ssid_bytes = nm_access_point_get_ssid(ap);
        if (ssid_bytes == NULL) {
          continue;
        }
        char *ssid = nm_utils_ssid_to_utf8(g_bytes_get_data(ssid_bytes, NULL), g_bytes_get_size(ssid_bytes));
        GtkWidget *ap_item = new_wifi_ap_menu_item(ssid, nm_access_point_get_strength(ap));
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), ap_item);
      }
    }
  }

  if (!is_wifi_enabled) {
    gtk_widget_destroy(menu);
    menu = gtk_menu_new();
    GtkWidget *wifi_disabled_item = gtk_menu_item_new_with_label("Wi-Fi is disabled");
    gtk_widget_set_sensitive(wifi_disabled_item, FALSE);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), wifi_disabled_item);
  }

  return menu;
}

static void init_nm_client() {
  GError *error = NULL;
  client = nm_client_new(NULL, &error);
  if (error != NULL) {
    g_print("Error initializing nm_client: %s\n", error->message);
    g_error_free(error);
  }
}

static void wifi_button_on_click(GtkWidget *widget, gpointer data) {
  GtkWidget *menu = new_wifi_menu();
  gtk_widget_show_all(menu);
  g_signal_connect(menu, "deactivate", G_CALLBACK(gtk_widget_destroy), NULL);
  gtk_menu_popup_at_pointer(GTK_MENU(menu), NULL);
}

void update_wifi_button(GtkWidget *button, gpointer data){
  const GPtrArray *devices = nm_client_get_devices(client);
  gboolean is_connected = FALSE;
  guint8 strength = 0;

  for (guint i = 0; i < devices->len; i++) {
    NMDevice *device = g_ptr_array_index(devices, i);
    if (NM_IS_DEVICE_WIFI(device)) {
      if (nm_device_get_state(device) == NM_DEVICE_STATE_ACTIVATED) {
        NMAccessPoint *ap = nm_device_wifi_get_active_access_point(NM_DEVICE_WIFI(device));
        if (ap != NULL) {
          strength = nm_access_point_get_strength(ap);
          is_connected = TRUE;
        }
      }
    }
  }

  gchar *icon_name = get_strength_icon_name(strength);
  GtkWidget *image = gtk_image_new_from_icon_name(icon_name, GTK_ICON_SIZE_BUTTON);
  gtk_button_set_image(GTK_BUTTON(button), image);
}

GtkWidget *new_wifi_button() {
  if (!client_initialized) {
    init_nm_client();
  }
  client_initialized = TRUE;
  GtkWidget *wifi_button = gtk_button_new();
  update_wifi_button(wifi_button, NULL);
  g_signal_connect(wifi_button, "clicked", G_CALLBACK(wifi_button_on_click), NULL);
  return wifi_button;
}
