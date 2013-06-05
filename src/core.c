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

#ifdef __unix__
# include <unistd.h>
#elif defined _WIN32
# include <windows.h>
#define sleep(x) Sleep(1000 * x)
#endif

int cfg_working_time = 55;
int cfg_rest_time = 5;
int cfg_working_left_time = 0;

int current_state = STATE_RESTING;

int finish_time_sec = 0;

void *thread_timer();

int main(int argc, char *argv[]) {

    g_thread_init(NULL );
    gdk_threads_init();
    gdk_threads_enter();

    gtk_init(&argc, &argv);

    //gtk_timeout_add(1000, thread_timer_one, NULL);

    fullscreen_show_init();

    core_tray_icon_show();

    core_start_working();

    //core_preferences_show();

    pthread_t thraed1;
    pthread_create(&thraed1, NULL, thread_timer, NULL );
    gtk_main();
    gdk_threads_leave();
}

void *thread_timer_one() {
    printf("go \n");
}

void *thread_timer() {

    for (;;) {
        printf("timer %i \n", current_state);
        if (current_state == STATE_WORKING || current_state == STATE_RESTING) {
            printf("enable \n");
            time_t current_time = time(NULL );
            if (finish_time_sec == 0) {
                finish_time_sec = current_time + cfg_working_time * 60;
            }
            time_t delta = finish_time_sec - current_time;

            //time_t now = (time_t)100;
            struct tm *format = localtime(&delta);

            printf("Current local time and date: %s", asctime(format));
            int res = format->tm_min;

            printf("%02i:%02i \n", format->tm_min, format->tm_sec);

            char timer_str[100];

            snprintf(timer_str, 100, "%02i:%02i", format->tm_min, format->tm_sec);

            cfg_working_left_time = format->tm_min;
            tray_icon_update();

            gdk_threads_enter();
            update_timer_str(timer_str);
                gdk_threads_leave();

            printf("delta %i \n", delta);
            if (delta <= 0) {
                if (current_state == STATE_WORKING) {
                    finish_time_sec = current_time + cfg_rest_time * 60;
                    current_state = STATE_RESTING;
                    gdk_threads_enter();
                    f_update_bg();
                    f_show_all();
                    gdk_threads_leave();
                    printf("show FULL screen \n");
                } else if (current_state == STATE_RESTING) {
                    printf("hide FULL screen \n");
                    current_state = STATE_WORKING;
                    finish_time_sec = current_time + cfg_rest_time * 60;
                    gdk_threads_enter();
                    f_hide();
                    gdk_threads_leave();

                }
            }
            if (current_state == STATE_RESTING) {
                f_set_time_label(timer_str);
            }

        }

        sleep(1);
    }
}
void core_init() {
    core_start_working();
}

void core_start_working() {
    printf("core_start_working %i \n",current_state);
    if (current_state != STATE_WORKING ) {
        current_state = STATE_WORKING;
        printf("start working \n");
        time_t current_time = time(NULL );
        finish_time_sec = current_time + cfg_working_time * 60;
    } else {
        current_state = STATE_DISABLE;
        update_timer_str("00:00");
    }
    update_button_name();

}

void core_take_a_break() {
    current_state = STATE_RESTING;
    core_start_working();
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

void core_fullscrean_show() {
    gdk_threads_enter();
    fullscreen_show_init();
    gdk_threads_leave();
}

