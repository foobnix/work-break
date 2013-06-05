/*
 * about.c
 *
 *  Created on: May 28, 2013
 *      Author: ivan
 */

#include <gtk/gtk.h>

void about_hide(GtkWidget *widget, gpointer data) {
    GtkWidget *window = (GtkWidget *) data;
    gtk_widget_hide_all(window);
}

void about_show_init() {
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_position(window, GTK_WIN_POS_CENTER);

    GtkWidget *text1 = gtk_label_new("About Type Breaker");

    GtkWidget *close = gtk_button_new_with_label("Close");

    GtkWidget *layout = gtk_vbox_new(FALSE, 0);

    gtk_box_pack_start(layout, text1, FALSE, TRUE, 0);
    gtk_box_pack_start(layout, close, FALSE, TRUE, 0);

    g_signal_connect(close, "clicked", G_CALLBACK(about_hide), G_OBJECT(window));

    gtk_container_add(GTK_CONTAINER(window), layout);

    gtk_widget_show_all(window);

}
