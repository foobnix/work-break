/*
 * demo1.c
 *
 *  Created on: May 13, 2013
 *      Author: ivan
 */

#include <gtk/gtk.h>
#include "core.h"

GtkWidget *timer;
GtkWidget *run_state;
GtkSpinButton *spin_work;
GtkSpinButton *spin_rest;

void hide(GtkWidget *window) {
    gtk_widget_hide_all(window);
}

void update_timer_str(char *str) {
    gtk_label_set_text(GTK_LABEL(timer), str);
}

void update_button_name() {
    if (current_state == STATE_WORKING) {
        gtk_button_set_label(GTK_BUTTON(run_state), "Stop Working");
    } else if (current_state == STATE_DISABLE) {
        gtk_button_set_label(GTK_BUTTON(run_state), "Start Working");
    }
}

void preferences_show_init() {

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_position(window, GTK_WIN_POS_CENTER);
    gtk_window_set_title(GTK_WINDOW(window), "Type Braking");
    gtk_widget_set_size_request(window, -1, -1);
    gtk_window_set_resizable(window, FALSE);
    g_signal_connect(window, "destroy", G_CALLBACK(hide), NULL);

    GtkWidget *layout = gtk_vbox_new(FALSE, 0);
    GtkWidget *line1 = gtk_hbox_new(FALSE, 0);
    GtkWidget *line2 = gtk_hbox_new(FALSE, 0);

    GtkWidget *text1 = gtk_label_new("Work interval");
    gtk_widget_set_size_request(text1, 100, -1);
    GtkWidget *min1 = gtk_label_new("Mins");
    GtkWidget *min2 = gtk_label_new("Mins");

    spin_work = gtk_spin_button_new_with_range(5, 60, 1);
    gtk_spin_button_set_value(spin_work, cfg_working_time);

    gtk_box_pack_start(line1, text1, FALSE, FALSE, 0);
    gtk_box_pack_start(line1, spin_work, FALSE, FALSE, 0);
    gtk_box_pack_start(line1, min1, FALSE, FALSE, 0);

    GtkWidget *text2 = gtk_label_new("Break interval");
    gtk_widget_set_size_request(text2, 100, -1);

    spin_rest = gtk_spin_button_new_with_range(1, 15, 1);
    gtk_spin_button_set_value(spin_rest, cfg_rest_time);

    gtk_box_pack_start(line2, text2, FALSE, FALSE, 0);
    gtk_box_pack_start(line2, spin_rest, FALSE, FALSE, 0);
    gtk_box_pack_start(line2, min2, FALSE, FALSE, 0);

    gtk_box_pack_start(layout, line1, FALSE, TRUE, 0);
    gtk_box_pack_start(layout, line2, FALSE, TRUE, 0);

    timer = gtk_label_new("00:00");

    run_state = gtk_button_new_with_label("Start Work");
    GtkWidget *stop = gtk_button_new_with_label("Take a Break");

    update_button_name();

    gtk_box_pack_start(layout, timer, FALSE, TRUE, 0);
    gtk_box_pack_start(layout, run_state, FALSE, TRUE, 0);
    gtk_box_pack_start(layout, stop, FALSE, TRUE, 0);

    int time = (int) gtk_spin_button_get_value(spin_work);
    int rest_time = (int) gtk_spin_button_get_value(spin_rest);

    g_signal_connect(run_state, "clicked", G_CALLBACK(core_start_working), NULL);
    g_signal_connect(stop, "clicked", G_CALLBACK(core_take_a_break), NULL);

    gtk_container_add(GTK_CONTAINER(window), layout);

    gtk_widget_show_all(window);

}