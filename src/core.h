/*
 * core.h
 *
 *  Created on: May 28, 2013
 *      Author: ivan
 */

#ifndef CORE_H_
#define CORE_H_

#define STATE_DISABLE 0
#define STATE_WORKING 1
#define STATE_RESTING 2

extern int cfg_working_time;
extern int cfg_rest_time;
extern int cfg_working_left_time;
extern int current_state;

void core_init();
void core_timer();
void core_tray_icon_show();
void core_preferences_show();
void core_fullscrean_show();
void core_about_show();
void core_take_a_break();
void core_start_working();
int core_get_minutes_to_break();

#endif /* CORE_H_ */
