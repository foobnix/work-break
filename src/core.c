/*
 * core.c
 *
 *  Created on: May 28, 2013
 *      Author: ivan
 */
#include <gtk/gtk.h>
#include "core.h"
#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include <glib.h>

int cfg_working_time_sec = 55 * 60;
int cfg_rest_time_sec = 5 * 60;
int cfg_working_left_time = 0;
int is_debug = 1;

volatile int current_state = STATE_STOP;

int finish_time_sec = 0;

void *thread_timer();

void get_settings() {
    GKeyFile *settings = g_key_file_new();
    g_key_file_load_from_file(settings, "config.ini", G_KEY_FILE_KEEP_COMMENTS, NULL );

    int time = g_key_file_get_integer(settings, "MAIN", "cfg_working_time_sec", NULL );
    printf("cfg_working_time_sec %i  \n", time);
    if (time != 0) {
        cfg_working_time_sec = time;
    } else {
        g_key_file_set_integer(settings, "MAIN", "cfg_working_time_sec", cfg_working_time_sec);
    }

    time = g_key_file_get_integer(settings, "MAIN", "cfg_rest_time_sec", NULL );
    printf("cfg_rest_time_sec %i  \n", time);
    if (time != 0) {
        cfg_rest_time_sec = time;
    } else {
        g_key_file_set_integer(settings, "MAIN", "cfg_rest_time_sec", cfg_rest_time_sec);
    }

    gchar *data = g_key_file_to_data(settings, NULL, NULL );
    FILE *file = fopen("config.ini", "w");
    fputs(data, file);
    fclose(file);
    g_free(data);

}
void c_save_settings() {
    GKeyFile *settings = g_key_file_new();
    g_key_file_load_from_file(settings, "config.ini", G_KEY_FILE_KEEP_COMMENTS, NULL);

    g_key_file_set_integer(settings, "MAIN", "cfg_working_time_sec", cfg_working_time_sec);
    g_key_file_set_integer(settings, "MAIN", "cfg_rest_time_sec", cfg_rest_time_sec);

    gchar *data = g_key_file_to_data(settings, NULL, NULL );
    FILE *file = fopen("config.ini", "w");
    fputs(data, file);
    fclose(file);
    g_free(data);

    printf("save settins %i  \n", time);
}

int main(int argc, char *argv[]) {

    g_thread_init(NULL );
    gdk_threads_init();
    gdk_threads_enter();

    gtk_init(&argc, &argv);

    get_settings();

    gtk_timeout_add(1000, thread_timer, NULL );

    fullscreen_show_init();

    core_tray_icon_show();

    c_start_work();

    core_preferences_show();

    gtk_main();
    gdk_threads_leave();
}

void *thread_timer() {
    printf("Timer STATE  %i  \n", current_state);
    if (current_state == STATE_STOP) {
        printf("Timer stopped \n");
    }

    time_t current_time = time(NULL );
    time_t delta = finish_time_sec - current_time;

    struct tm *format = localtime(&delta);

    printf("Current local time and date: %s", asctime(format));
    int res = format->tm_min;

    printf("%02i:%02i \n", format->tm_min, format->tm_sec);

    char timer_str[100];

    snprintf(timer_str, 100, "%02i:%02i", format->tm_min, format->tm_sec);

    cfg_working_left_time = format->tm_min;

    tray_icon_update();
    update_timer_str(timer_str);
    f_set_time_label(timer_str);

    printf("delta %i \n", delta);

    if (delta <= 0) {
        if (current_state == STATE_WORKING) {
            c_take_brake();
        } else if (current_state == STATE_RESTING) {
            c_start_work();
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
    current_state = STATE_WORKING;
    time_t current_time = time(NULL );
    finish_time_sec = current_time + cfg_working_time_sec;

    f_hide();

}

void c_take_brake() {
    current_state = STATE_RESTING;
    time_t current_time = time(NULL );
    finish_time_sec = current_time + cfg_rest_time_sec;

    pref_hide();
    f_update_bg();
    f_show_all();
}

void core_about_show() {
    about_show_init();
}

void core_tray_icon_show() {
    tray_icon_show_init();
}
void core_preferences_show() {
    preferences_show_init();
}
