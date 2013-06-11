#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include "core.h"
#include <unistd.h>

int cfg_working_left_time;

GtkStatusIcon *trayIcon;

static void tray_icon_popup(GtkStatusIcon *status_icon, guint button, guint32 activate_time, gpointer popUpMenu) {
    printf("poup \n");
    gtk_menu_popup(GTK_MENU(popUpMenu), NULL, NULL, gtk_status_icon_position_menu, status_icon, button, activate_time);
}

void tray_icon_update() {
    gchar *res = g_strdup_printf("%i minutes left before rest", cfg_working_left_time);
    gtk_status_icon_set_tooltip(trayIcon, res);
}

void on_mouse_move() {
    printf("on mouse move \n");
}
void try_quit(){
    c_save_settings();
    gtk_main_quit();
}
void tray_icon_show_init() {

    char fname[] = "systray.png";

    if( access( fname, F_OK ) != -1 ) {
        trayIcon = gtk_status_icon_new_from_file(fname);
    } else {
        trayIcon = gtk_status_icon_new_from_icon_name(GTK_STOCK_EXECUTE);
    }

    GtkWidget *menu = gtk_menu_new();

    GtkWidget *pref = gtk_image_menu_item_new_from_stock(GTK_STOCK_PREFERENCES, NULL );
    GtkWidget *line = gtk_separator_menu_item_new();

    GtkWidget *about = gtk_image_menu_item_new_from_stock(GTK_STOCK_QUIT, NULL );
    GtkWidget *take_break = gtk_menu_item_new_with_label("Take A Break");

    g_signal_connect(G_OBJECT (pref), "activate", G_CALLBACK (core_preferences_show), NULL);
    g_signal_connect(G_OBJECT (about), "activate", G_CALLBACK (try_quit), NULL);
    g_signal_connect(G_OBJECT (take_break), "activate", G_CALLBACK (c_take_brake), NULL);

    gtk_menu_shell_append(GTK_MENU_SHELL (menu), pref);
    gtk_menu_shell_append(GTK_MENU_SHELL (menu), about);
    gtk_menu_shell_append(GTK_MENU_SHELL (menu), line);
    gtk_menu_shell_append(GTK_MENU_SHELL (menu), take_break);

    gtk_widget_show_all(menu);

    g_signal_connect(GTK_STATUS_ICON (trayIcon), "activate", GTK_SIGNAL_FUNC (core_preferences_show), NULL);
    g_signal_connect(GTK_STATUS_ICON (trayIcon), "popup-menu", GTK_SIGNAL_FUNC (tray_icon_popup), menu);

    gtk_widget_show_all(trayIcon);

}

