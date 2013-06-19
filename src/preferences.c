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
GtkWidget *window;

int cfg_working_time_sec;
int cfg_rest_time_sec;

void update_timer_str(char *str) {
	gtk_label_set_text(GTK_LABEL(timer), str);
}

void update_time() {
	cfg_working_time_sec = gtk_spin_button_get_value(spin_work) * 60;
	cfg_rest_time_sec = gtk_spin_button_get_value(spin_rest) * 60;
	c_save_settings();
}

void in_start() {
	update_time();
	c_start_work();
}
void in_break() {
	update_time();
	c_take_brake();
}
gboolean pref_hide(){
    printf("my destroy \n");
	gtk_widget_hide_all(window);
	return FALSE;
}
void pref_show_all(){
    printf("my show \n");
    gtk_widget_show_all(window);
}
void in_2h_work(){
    c_start_long_work();
}

void preferences_show_init() {

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_position(window, GTK_WIN_POS_CENTER);
	gtk_window_set_title(GTK_WINDOW(window), "Type Braking");
	gtk_widget_set_size_request(window, -1, -1);
	gtk_window_set_resizable(window, FALSE);
	g_signal_connect(window, "destroy", G_CALLBACK(pref_hide),
			window);

	GtkWidget *layout = gtk_vbox_new(FALSE, 0);
	GtkWidget *line1 = gtk_hbox_new(FALSE, 0);
	GtkWidget *line2 = gtk_hbox_new(FALSE, 0);

	GtkWidget *text1 = gtk_label_new("Work interval");
	gtk_widget_set_size_request(text1, 100, -1);
	GtkWidget *min1 = gtk_label_new("Mins");
	GtkWidget *min2 = gtk_label_new("Mins");

	spin_work = gtk_spin_button_new_with_range(5, 60, 1);
	if (cfg_working_time_sec > 60) {
		gtk_spin_button_set_value(spin_work, cfg_working_time_sec / 60);
	} else {
		gtk_spin_button_set_value(spin_work, cfg_working_time_sec);
	}

	gtk_box_pack_start(line1, text1, FALSE, FALSE, 0);
	gtk_box_pack_start(line1, spin_work, FALSE, FALSE, 0);
	gtk_box_pack_start(line1, min1, FALSE, FALSE, 0);

	GtkWidget *text2 = gtk_label_new("Break interval");
	gtk_widget_set_size_request(text2, 100, -1);

	spin_rest = gtk_spin_button_new_with_range(1, 15, 1);

	if (cfg_rest_time_sec > 60) {
		gtk_spin_button_set_value(spin_rest, cfg_rest_time_sec / 60);
	} else {
		gtk_spin_button_set_value(spin_rest, cfg_rest_time_sec);
	}

	gtk_box_pack_start(line2, text2, FALSE, FALSE, 0);
	gtk_box_pack_start(line2, spin_rest, FALSE, FALSE, 0);
	gtk_box_pack_start(line2, min2, FALSE, FALSE, 0);

	gtk_box_pack_start(layout, line1, FALSE, TRUE, 0);
	gtk_box_pack_start(layout, line2, FALSE, TRUE, 0);

	timer = gtk_label_new("00:00");

	run_state = gtk_button_new_with_label("Start Work");


	GtkWidget *long_work = gtk_button_new_with_label("Working for 2h");

	GtkWidget *layout_work = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(layout_work, run_state, TRUE, TRUE, 0);
	gtk_box_pack_start(layout_work, long_work, TRUE, TRUE, 0);


	GtkWidget *take_break = gtk_button_new_with_label("Take a Break");

	gtk_button_set_label(GTK_BUTTON(run_state), "Start Working");

	gtk_box_pack_start(layout, timer, FALSE, TRUE, 0);
	gtk_box_pack_start(layout, layout_work, TRUE, TRUE, 0);
	gtk_box_pack_start(layout, take_break, FALSE, TRUE, 0);

	int time = (int) gtk_spin_button_get_value(spin_work);
	int rest_time = (int) gtk_spin_button_get_value(spin_rest);

	g_signal_connect(run_state, "clicked", G_CALLBACK(in_start), NULL);
	g_signal_connect(take_break, "clicked", G_CALLBACK(in_break), NULL);
	g_signal_connect(long_work, "clicked", G_CALLBACK(in_2h_work), NULL);

	gtk_container_add(GTK_CONTAINER(window), layout);

	//gtk_widget_show_all(window);

}
