/*
 * core.h
 *
 *  Created on: May 28, 2013
 *      Author: ivan
 */

#ifndef CORE_H_
#define CORE_H_

#define STATE_STOP 0
#define STATE_WORKING 1
#define STATE_RESTING 2

extern int cfg_working_time_sec;
extern int cfg_rest_time_sec;
extern int cfg_working_left_time;
extern volatile int current_state;

void core_tray_icon_show();
void core_preferences_show();
void core_fullscrean_show();
void core_about_show();

void c_start_work();
void c_take_brake();

#endif /* CORE_H_ */
