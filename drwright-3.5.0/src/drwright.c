/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2003-2005 Imendio HB
 * Copyright (C) 2002-2003 Richard Hult <richard@imendio.com>
 * Copyright (C) 2002 CodeFactory AB
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <config.h>
#include <string.h>
#include <math.h>
#include <gio/gio.h>
#include <glib/gi18n.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <libnotify/notify.h>

#include "drwright.h"
#include "drw-break-window.h"
#include "drw-monitor.h"
#include "drw-utils.h"
#include "drw-timer.h"

#define WARN_TIME_MAX        (3 * 60 /* s */)

#define I_(string) (g_intern_static_string (string))

typedef enum {
	STATE_START,
	STATE_RUNNING,
	STATE_WARN,
	STATE_BREAK_SETUP,
	STATE_BREAK,
	STATE_BREAK_DONE_SETUP,
	STATE_BREAK_DONE
} DrwState;

struct _DrWright {
        GSettings      *settings;

        GDBusProxy     *screensaver_proxy;

	/* Widgets. */
	GtkWidget      *break_window;
	GList          *secondary_break_windows;

	NotifyNotification *notification;
	gint            num_minutes_last_displayed;

	DrwMonitor     *monitor;

	DrwState        state;
	DrwTimer       *timer;
	DrwTimer       *idle_timer;

	gint            last_elapsed_time;
	gint            save_last_time;

	/* Time settings. */
	gint            type_time;
	gint            break_time;
	gint            warn_time;

	gboolean        enabled;
};

static void     activity_detected_cb           (DrwMonitor     *monitor,
						DrWright       *drwright);
static gboolean maybe_change_state             (DrWright       *drwright);
static void     break_window_done_cb           (GtkWidget      *window,
						DrWright       *dr);
static void     break_window_postpone_cb       (GtkWidget      *window,
						DrWright       *dr);
static void     break_window_lock_cb           (GtkWidget      *window,
						DrWright       *dr);
static void     break_window_destroy_cb        (GtkWidget      *window,
						DrWright       *dr);
static GList *  create_secondary_break_windows (void);

extern gboolean debug;

#define GNOME_SCREENSAVER_BUS_NAME      "org.gnome.ScreenSaver"
#define GNOME_SCREENSAVER_OBJECT_PATH   "/org/gnome/ScreenSaver"
#define GNOME_SCREENSAVER_INTERFACE     "org.gnome.ScreenSaver"

/* FIXMEchpe: make this async! */
static GDBusProxy *
get_screensaver_proxy (DrWright *dr)
{
        GDBusConnection *connection;
        GVariant *result;
        GError *error = NULL;

        if (dr->screensaver_proxy != NULL)
                return dr->screensaver_proxy;

        connection = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, NULL);
        if (connection == NULL)
                return NULL;

        dr->screensaver_proxy =
          g_dbus_proxy_new_sync (connection,
                                 G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES |
                                 G_DBUS_PROXY_FLAGS_DO_NOT_CONNECT_SIGNALS |
                                 G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START,
                                 NULL,
                                 GNOME_SCREENSAVER_BUS_NAME,
                                 GNOME_SCREENSAVER_OBJECT_PATH,
                                 GNOME_SCREENSAVER_INTERFACE,
                                 NULL,
                                 NULL);
        g_object_unref (connection);

        return dr->screensaver_proxy;
}

gboolean
drwright_can_lock_screen (DrWright *dr)
{
        return get_screensaver_proxy (dr) != NULL;
}

static void
notification_action_cb (NotifyNotification *notification,
                        const char *action,
                        DrWright *dr)
{
	GAppInfo *app_info;
	GdkAppLaunchContext *launch_context;

	g_assert (action != NULL);

	if (g_strcmp0 (action, "take-break") == 0) {
		if (dr->enabled) {
			dr->state = STATE_BREAK_SETUP;
			maybe_change_state (dr);
		}
	}
	else if (g_strcmp0 (action, "settings") == 0) {
		GError *error = NULL;

		launch_context = gdk_display_get_app_launch_context (gdk_display_get_default ());

		app_info = g_app_info_create_from_commandline (BINDIR "/gnome-control-center typing-break",
		                                               NULL,
		                                               G_APP_INFO_CREATE_SUPPORTS_STARTUP_NOTIFICATION,
		                                               &error);
		if (error) {
			g_warning ("%s", error->message);
			goto out;
		}

		g_app_info_launch (app_info, NULL, G_APP_LAUNCH_CONTEXT (launch_context), &error);
		if (error) {
			g_warning ("%s", error->message);
		}
out:
		g_clear_error (&error);
		g_object_unref (launch_context);
	}
	else {
		g_warning ("Unknown action: %s", action);
	}
}

static void
show_warning_notification (DrWright *dr, gboolean show)
{
	NotifyNotification *notification;
	gint minutes, seconds;
	gchar *summary, *body;
	GError *error = NULL;

	if (show == FALSE) {
		if (dr->notification) {
			gboolean success = notify_notification_close (dr->notification, &error);
			if (success) {
				g_object_unref (dr->notification);
				dr->notification = NULL;
			}
			else {
				g_warning ("%s", error->message);
			}
			g_clear_error (&error);
		}
		dr->num_minutes_last_displayed = -1;
		return;
	}

	seconds = dr->type_time - drw_timer_elapsed (dr->timer) - dr->save_last_time;
	minutes = (seconds / 60) + 1;

	if (minutes == dr->num_minutes_last_displayed) {
		return;
	}
	dr->num_minutes_last_displayed = minutes;

	summary = _("Typing Break Reminder");

	if (minutes > 1) {
		body = g_strdup_printf (ngettext ("Approximately %d minute to the next break.",
		                                  "Approximately %d minutes to the next break.",
		                                  minutes),
		                        minutes);
	}
	else {
		body = g_strdup (_("Less than one minute to the next break."));
	}

	if (dr->notification) {
		notification = dr->notification;
		notify_notification_update (notification, summary, body, "typing-monitor");
	}
	else {
		notification = notify_notification_new (summary, body, "typing-monitor");
		notify_notification_set_hint (notification, "resident",
		                              g_variant_new_boolean (TRUE));
		notify_notification_add_action (notification,
		                                "settings", _("Settings…"),
		                                NOTIFY_ACTION_CALLBACK (notification_action_cb),
		                                dr, NULL);
		notify_notification_add_action (notification,
		                                "take-break", _("Take Break Now"),
		                                NOTIFY_ACTION_CALLBACK (notification_action_cb),
		                                dr, NULL);
		dr->notification = notification;
	}

	if (!notify_notification_show (notification, &error)) {
		g_warning ("%s", error->message);
	}

	g_clear_error (&error);
	g_free (body);
}

static void
setup_debug_values (DrWright *dr)
{
	dr->type_time = 300;
	dr->warn_time = 150;
	dr->break_time = 60;
}

static gboolean
grab_keyboard_on_window (GdkWindow *window,
			 guint32    activate_time)
{
	GdkGrabStatus status;

	status = gdk_keyboard_grab (window, TRUE, activate_time);
	if (status == GDK_GRAB_SUCCESS) {
		return TRUE;
	}

	return FALSE;
}

static gboolean
break_window_map_event_cb (GtkWidget *widget,
			   GdkEvent  *event,
			   DrWright  *dr)
{
	grab_keyboard_on_window (gtk_widget_get_window (dr->break_window), gtk_get_current_event_time ());

        return FALSE;
}

static gboolean
maybe_change_state (DrWright *dr)
{
	gint elapsed_time;
	gint elapsed_idle_time;

	if (debug) {
		drw_timer_start (dr->idle_timer);
	}

	elapsed_time = drw_timer_elapsed (dr->timer) + dr->save_last_time;
	elapsed_idle_time = drw_timer_elapsed (dr->idle_timer);

	if (elapsed_time > dr->last_elapsed_time + dr->warn_time) {
		/* If the timeout is delayed by the amount of warning time, then
		 * we must have been suspended or stopped, so we just start
		 * over.
		 */
		dr->state = STATE_START;
	}

	switch (dr->state) {
	case STATE_START:
		if (dr->break_window) {
			gtk_widget_destroy (dr->break_window);
			dr->break_window = NULL;
		}

		dr->save_last_time = 0;

		drw_timer_start (dr->timer);
		drw_timer_start (dr->idle_timer);
		show_warning_notification (dr, FALSE);

		if (dr->enabled) {
			dr->state = STATE_RUNNING;
		}

		break;

	case STATE_RUNNING:
	case STATE_WARN:
		if (elapsed_idle_time >= dr->break_time) {
			dr->state = STATE_BREAK_DONE_SETUP;
 		} else if (elapsed_time >= dr->type_time) {
			dr->state = STATE_BREAK_SETUP;
		} else if (dr->state != STATE_WARN
			   && elapsed_time >= dr->type_time - dr->warn_time) {
			dr->state = STATE_WARN;
		}
		if (dr->state == STATE_WARN) {
			show_warning_notification (dr, TRUE);
		}
		break;

	case STATE_BREAK_SETUP:
		/* Don't allow more than one break window to coexist, can happen
		 * if a break is manually enforced.
		 */
		if (dr->break_window) {
			dr->state = STATE_BREAK;
			break;
		}

		show_warning_notification (dr, FALSE);

		drw_timer_start (dr->timer);

		dr->break_window = drw_break_window_new (drwright_can_lock_screen (dr));

		drw_break_window_set_elapsed_idle_time (DRW_BREAK_WINDOW (dr->break_window),
		                                        elapsed_idle_time);

		g_signal_connect (dr->break_window, "map_event",
				  G_CALLBACK (break_window_map_event_cb),
				  dr);

		g_signal_connect (dr->break_window,
				  "done",
				  G_CALLBACK (break_window_done_cb),
				  dr);

		g_signal_connect (dr->break_window,
				  "postpone",
				  G_CALLBACK (break_window_postpone_cb),
				  dr);

		g_signal_connect (dr->break_window,
				  "lock",
				  G_CALLBACK (break_window_lock_cb),
				  dr);

		g_signal_connect (dr->break_window,
				  "destroy",
				  G_CALLBACK (break_window_destroy_cb),
				  dr);

		dr->secondary_break_windows = create_secondary_break_windows ();

		gtk_widget_show (dr->break_window);

		dr->save_last_time = elapsed_time;
		dr->state = STATE_BREAK;
		break;

	case STATE_BREAK:
		if (elapsed_time - dr->save_last_time >= dr->break_time) {
			dr->state = STATE_BREAK_DONE_SETUP;
		}
		break;

	case STATE_BREAK_DONE_SETUP:
		dr->state = STATE_BREAK_DONE;
		break;

	case STATE_BREAK_DONE:
		dr->state = STATE_START;
		if (dr->break_window) {
			gtk_widget_destroy (dr->break_window);
			dr->break_window = NULL;
		}
		break;
	}

	dr->last_elapsed_time = elapsed_time;

	return TRUE;
}

static void
activity_detected_cb (DrwMonitor *monitor,
		      DrWright   *dr)
{
	drw_timer_start (dr->idle_timer);
}

static void
settings_change_cb (GSettings  *settings,
                    const char *key,
                    DrWright   *dr)
{
        dr->state = STATE_START;

        if (!key || key == I_("type-time")) {
                dr->type_time = g_settings_get_int (settings, "type-time");
                dr->warn_time = MIN (dr->type_time / 10, WARN_TIME_MAX);
	}
	if (!key || key == I_("break-time")) {
                dr->break_time = g_settings_get_int (settings, "break-time");
	}
	if (!key || key == I_("enabled")) {
                dr->enabled = g_settings_get_boolean (settings, "enabled");
	}

	maybe_change_state (dr);
}

static void
break_window_done_cb (GtkWidget *window,
		      DrWright  *dr)
{
	gtk_widget_destroy (dr->break_window);

	dr->state = STATE_BREAK_DONE_SETUP;
	dr->break_window = NULL;

	maybe_change_state (dr);
}

static void
break_window_postpone_cb (GtkWidget *window,
			  DrWright  *dr)
{
	gint elapsed_time;

	gtk_widget_destroy (dr->break_window);

	dr->state = STATE_RUNNING;
	dr->break_window = NULL;

	elapsed_time = drw_timer_elapsed (dr->timer);

	if (elapsed_time + dr->save_last_time >= dr->type_time) {
		/* Typing time has expired, but break was postponed.
		 * We'll warn again in (elapsed * sqrt (typing_time))^2 */
		gfloat postpone_time = (((float) elapsed_time) / dr->break_time)
					* sqrt (dr->type_time);
		postpone_time *= postpone_time;
		dr->save_last_time = dr->type_time - MAX (dr->warn_time, (gint) postpone_time);
	}

	drw_timer_start (dr->timer);
	maybe_change_state (dr);
}

static void
screensaver_locked_cb (GDBusProxy *proxy,
                       GAsyncResult *result,
                       DrWright *dr)
{
  GVariant *variant;

  variant = g_dbus_proxy_call_finish (proxy, result, NULL);
  if (variant == NULL)
          return;

  g_variant_unref (variant);

  /* We keep the break window. If the user unlocks before the break is up,
   * it'll still prevent typing; otherwise the timeout will have killed
   * it in the meantime.
   *
   * FIXME: make sure we don't go to 'break' again while the screen is
   * locked.
   */
}

static void
break_window_lock_cb (GtkWidget *window,
		      DrWright  *dr)
{
        GDBusProxy *proxy;

        proxy = get_screensaver_proxy (dr);
        if (proxy == NULL)
                return;

        /* ungrab the keyboard so the screensaver can start */
	gdk_keyboard_ungrab (GDK_CURRENT_TIME);

        g_dbus_proxy_call (proxy,
                           "Lock",
                           g_variant_new ("()"),
                           G_DBUS_CALL_FLAGS_NONE,
                           -1,
                           NULL,
                           (GAsyncReadyCallback) screensaver_locked_cb,
                           dr);
}

static void
break_window_destroy_cb (GtkWidget *window,
			 DrWright  *dr)
{
	GList *l;

	for (l = dr->secondary_break_windows; l; l = l->next) {
		gtk_widget_destroy (l->data);
	}

	g_list_free (dr->secondary_break_windows);
	dr->secondary_break_windows = NULL;
}

static GList *
create_secondary_break_windows (void)
{
	GdkDisplay *display;
	GdkScreen  *screen;
	GtkWidget  *window;
	gint        i;
	GList      *windows = NULL;

	display = gdk_display_get_default ();

	for (i = 0; i < gdk_display_get_n_screens (display); i++) {
		screen = gdk_display_get_screen (display, i);

		if (screen == gdk_screen_get_default ()) {
			/* Handled by DrwBreakWindow. */
			continue;
		}

		window = gtk_window_new (GTK_WINDOW_POPUP);

		windows = g_list_prepend (windows, window);

		gtk_window_set_screen (GTK_WINDOW (window), screen);

		gtk_window_set_default_size (GTK_WINDOW (window),
					     gdk_screen_get_width (screen),
					     gdk_screen_get_height (screen));

		gtk_widget_set_app_paintable (GTK_WIDGET (window), TRUE);
		drw_setup_background (GTK_WIDGET (window));
		gtk_window_stick (GTK_WINDOW (window));
		gtk_widget_show (window);
	}

	return windows;
}

DrWright *
drwright_new (void)
{
	DrWright *dr = g_new0 (DrWright, 1);

	if (debug) {
		setup_debug_values (dr);
	}

	if (!notify_is_initted ()) {
		notify_init (g_get_application_name ());
	}

	dr->timer = drw_timer_new ();
	dr->idle_timer = drw_timer_new ();

	dr->state = STATE_START;

	dr->monitor = drw_monitor_new ();

	g_signal_connect (dr->monitor,
			  "activity",
			  G_CALLBACK (activity_detected_cb),
			  dr);

	g_timeout_add_seconds (1,
			       (GSourceFunc) maybe_change_state,
			       dr);

        dr->settings = g_settings_new (DRW_SETTINGS_SCHEMA_ID);
        settings_change_cb (dr->settings, NULL, dr);
        g_signal_connect (dr->settings, "changed",
                          G_CALLBACK (settings_change_cb), dr);

	return dr;
}
