/*
 * core.c
 *
 *  Created on: May 28, 2013
 *      Author: ivan
 */
#include <gtk/gtk.h>
#include "core.h"
#include "mouse_xy.h"
#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include <glib.h>

int cfg_working_time_sec = 55 * 60;
int cfg_rest_time_sec = 5 * 60;
int cfg_working_left_time = 0;
int is_debug;
int is_with_tray = 0;
int is_mouse_event = 0;

volatile int current_state = STATE_STOP;
char* oldMouse = "0x:1869 y:1362 screen:0 window:73400327";
int absentTimer = 0;

int finish_time_sec = 0;

int *thread_timer();

void get_settings() {
	GKeyFile *settings = g_key_file_new();
	g_key_file_load_from_file(settings, "config.ini", G_KEY_FILE_KEEP_COMMENTS,
	NULL);

	int time = g_key_file_get_integer(settings, "MAIN", "cfg_working_time_sec",
	NULL);
	printf("cfg_working_time_sec %i  \n", time);
	if (time != 0) {
		cfg_working_time_sec = time;
	} else {
		g_key_file_set_integer(settings, "MAIN", "cfg_working_time_sec",
				cfg_working_time_sec);
	}

	time = g_key_file_get_integer(settings, "MAIN", "cfg_rest_time_sec", NULL);
	printf("cfg_rest_time_sec %i  \n", time);
	if (time != 0) {
		cfg_rest_time_sec = time;
	} else {
		g_key_file_set_integer(settings, "MAIN", "cfg_rest_time_sec",
				cfg_rest_time_sec);
	}

	is_debug = g_key_file_get_integer(settings, "MAIN", "is_debug", NULL);
    is_with_tray = g_key_file_get_integer(settings, "MAIN", "is_with_tray", NULL);
	printf("is_debug %i  \n", is_debug);
	printf("is_with_tray %i  \n", is_with_tray);

	gchar *data = g_key_file_to_data(settings, NULL, NULL);
	FILE *file = fopen("config.ini", "w");
	fputs(data, file);
	fclose(file);
	g_free(data);

}
void c_save_settings() {
	GKeyFile *settings = g_key_file_new();
	g_key_file_load_from_file(settings, "config.ini", G_KEY_FILE_KEEP_COMMENTS,
	NULL);

	g_key_file_set_integer(settings, "MAIN", "cfg_working_time_sec",
			cfg_working_time_sec);
	g_key_file_set_integer(settings, "MAIN", "cfg_rest_time_sec",
			cfg_rest_time_sec);

	gchar *data = g_key_file_to_data(settings, NULL, NULL);
	FILE *file = fopen("config.ini", "w");
	fputs(data, file);
	fclose(file);
	g_free(data);

	printf("save settins %i  \n", time);
}

static gboolean rmotion() {
	printf("root motion \n");
	is_mouse_event = 1;
	return FALSE;
}

int main(int argc, char *argv[]) {

	g_thread_init(NULL);
	gdk_threads_init();
	gdk_threads_enter();

	gtk_init(&argc, &argv);

	get_settings();

	gtk_timeout_add(1000, thread_timer, NULL);

	fullscreen_show_init();

	core_tray_icon_show();

	c_start_work();

	preferences_show_init();
	//pref_show_all();

	gtk_main();
	gdk_threads_leave();
}

int *thread_timer() {
	printf("Timer STATE  %i  \n", current_state);
	if (current_state == STATE_STOP) {
		printf("Timer stopped \n");
	}

	char * mouseXY = get_mouse_xy();
	int r = strcmp(mouseXY, oldMouse) == 0;
	printf("%i", r);
	printf("%s", mouseXY);

	if (strcmp(mouseXY, oldMouse) == 0) {
		printf("Mouse the same %s \n", mouseXY);
		absentTimer++;
	} else {
		printf("Mouse NOT the same %s \n", mouseXY);
		absentTimer = 0;
	}
	oldMouse = mouseXY;
	printf("Absent Timer %i \n", absentTimer);

	if(absentTimer > cfg_rest_time_sec){
		c_start_work();
	}


	time_t current_time = time(NULL);
	printf("current_time is %i \n", current_time);

	time_t delta = finish_time_sec - current_time;
	printf("delta %i \n", delta);

	//struct tm *format = localtime(&delta);
	struct tm *format = gmtime(&delta);
	if (format == 0) {
		printf("time format 0\n");
		int *res = 12;
		return res;
	}
	printf("format %i \n", format);
	printf("format %i \n", format->tm_min);
	//printf("Current local time and date: %s", asctime(format));
	int res = format->tm_min;

	printf("time %i %02i:%02i \n", format->tm_hour, format->tm_min,
			format->tm_sec);

	char timer_str[100];

	if (format->tm_hour > 0) {
		snprintf(timer_str, 100, "%i:%02i:%02i", format->tm_hour,
				format->tm_min, format->tm_sec);
	} else {
		snprintf(timer_str, 100, "%02i:%02i", format->tm_min, format->tm_sec);
	}

	cfg_working_left_time = format->tm_min;

	tray_icon_update(delta);
	update_timer_str(timer_str);
	f_set_time_label(timer_str);

	printf("delta %i \n", delta);
	printf("is_mouse_event %i \n", is_mouse_event);

	int temp = is_mouse_event;

	if (delta <= 0) {
		if (current_state == STATE_WORKING) {
			c_take_brake();
		} else if (current_state == STATE_RESTING) {
			current_state = STATE_WAITING;
			is_mouse_event = 0;
			finish_time_sec = current_time + 1;
		} else if (current_state == STATE_WAITING) {
			finish_time_sec = current_time + 1;
			printf("is_mouse_event %i \n", is_mouse_event);
			if (temp == 1) {
				printf("REady for work \n");
				c_start_work();
			}
		}
	}

}
void c_on_any_event() {
	printf("any event \n");
	if (current_state == STATE_RESTING) {
		c_start_work();
	}
}

void c_start_work() {
	absentTimer = 0;
	current_state = STATE_WORKING;
	time_t current_time = time(NULL);
	finish_time_sec = current_time + cfg_working_time_sec;
	//finish_time_sec = current_time + 10;
	f_hide();
	tray_show();
}
void c_postpone() {
	printf("c_postpone \n");
	current_state = STATE_WORKING;
	time_t current_time = time(NULL);
	finish_time_sec = current_time + 5 * 60;
	printf("finish_time_sec %i \n",finish_time_sec);
	f_hide();
	tray_show();
}

void c_start_long_work() {
	current_state = STATE_WORKING;
	time_t current_time = time(NULL);
	finish_time_sec = current_time + 2 * 60 * 60;
	f_hide();
	tray_show();
}

void c_take_brake() {
	printf("c_take_brake \n");
	current_state = STATE_RESTING;
	time_t current_time = time(NULL);
	finish_time_sec = current_time + cfg_rest_time_sec;
	//finish_time_sec = current_time + 3;
	
	printf("finish_time_sec %i \n", finish_time_sec);
		

	pref_hide();
	f_update_bg();
	f_show_all();
	//tray_hide();

}

void core_about_show() {
	about_show_init();
}

void core_tray_icon_show() {
	tray_icon_show_init();
}
void core_preferences_hide() {
	pref_hide();
}
void core_preferences_show() {
	pref_hide();
	preferences_show_init();
	pref_show_all();
}

void core_preferences_show_hide() {
	show_hide();
}
