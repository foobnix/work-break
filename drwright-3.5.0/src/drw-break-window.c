/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2002      CodeFactory AB
 * Copyright (C) 2002-2003 Richard Hult <richard@imendio.com>

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
#include <unistd.h>
#include <math.h>
#include <glib/gi18n.h>
#include <gio/gio.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <canberra-gtk.h>

#include "drwright.h"
#include "drw-utils.h"
#include "drw-break-window.h"
#include "drw-timer.h"

struct _DrwBreakWindowPrivate {
	GtkWidget *clock_label;
	GtkWidget *break_label;
	GtkWidget *image;

	GtkWidget *postpone_entry;
	GtkWidget *postpone_button;

	DrwTimer  *timer;

	gint       break_time;
	gchar     *break_text;
	guint      clock_timeout_id;
	guint      postpone_timeout_id;
	guint      postpone_sensitize_id;
	guint      postpone_delay;
	gint       elapsed_idle_time;

        gboolean   enable_lock;

	GSettings *settings;
};

#define DRW_BREAK_WINDOW_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), DRW_TYPE_BREAK_WINDOW, DrwBreakWindowPrivate))

#define POSTPONE_DELAY (10 /* ms */)

#define POSTPONE_CANCEL 30

#define ELAPSED_IDLE_INTERVAL 30 /* s */

/* Signals */
enum {
	DONE,
	POSTPONE,
	LOCK,
	LAST_SIGNAL
};

enum {
        PROP_0,
        PROP_ENABLE_LOCK
};

static void         drw_break_window_class_init    (DrwBreakWindowClass *klass);
static void         drw_break_window_init          (DrwBreakWindow      *window);
static void         drw_break_window_constructed   (GObject             *object);
static void         drw_break_window_finalize      (GObject             *object);
static void         drw_break_window_dispose       (GObject             *object);
static gboolean     postpone_sensitize_cb          (DrwBreakWindow      *window);
static gboolean     clock_timeout_cb               (DrwBreakWindow      *window);
static void         postpone_clicked_cb            (GtkWidget           *button,
						    GtkWidget           *window);

G_DEFINE_TYPE (DrwBreakWindow, drw_break_window, GTK_TYPE_WINDOW)

static guint signals[LAST_SIGNAL];

static void
drw_break_window_set_property (GObject      *object,
                               guint         property_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
        DrwBreakWindow *window = DRW_BREAK_WINDOW (object);
        DrwBreakWindowPrivate *priv = window->priv;

        switch (property_id) {
                case PROP_ENABLE_LOCK:
                        priv->enable_lock = g_value_get_boolean (value);
                        break;
                default:
                        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
        }
}

static void
drw_break_window_class_init (DrwBreakWindowClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);

        object_class->set_property = drw_break_window_set_property;
        object_class->constructed = drw_break_window_constructed;
        object_class->finalize = drw_break_window_finalize;
        object_class->dispose = drw_break_window_dispose;

	signals[POSTPONE] =
		g_signal_new ("postpone",
			      G_TYPE_FROM_CLASS (klass),
			      G_SIGNAL_RUN_LAST,
			      0,
			      NULL, NULL,
			      g_cclosure_marshal_VOID__VOID,
			      G_TYPE_NONE, 0);

	signals[LOCK] =
		g_signal_new ("lock",
			      G_TYPE_FROM_CLASS (klass),
			      G_SIGNAL_RUN_LAST,
			      0,
			      NULL, NULL,
			      g_cclosure_marshal_VOID__VOID,
			      G_TYPE_NONE, 0);

	signals[DONE] =
	signals[DONE] =
		g_signal_new ("done",
			      G_TYPE_FROM_CLASS (klass),
			      G_SIGNAL_RUN_LAST,
			      0,
			      NULL, NULL,
			      g_cclosure_marshal_VOID__VOID,
			      G_TYPE_NONE, 0);

        g_object_class_install_property
                (object_class,
                 PROP_ENABLE_LOCK,
                 g_param_spec_boolean ("enable-lock", NULL, NULL,
                                       FALSE,
                                       G_PARAM_WRITABLE |
                                       G_PARAM_CONSTRUCT_ONLY |
                                       G_PARAM_STATIC_STRINGS));

	g_type_class_add_private (klass, sizeof (DrwBreakWindowPrivate));
}

static void
lock_clicked_cb (GtkWidget *button, DrwBreakWindow *window)
{
	g_signal_emit (window, signals[LOCK], 0, NULL);
}

static void
drw_break_window_init (DrwBreakWindow *window)
{
        window->priv = DRW_BREAK_WINDOW_GET_PRIVATE (window);
}

static void
drw_break_window_constructed (GObject *object)
{
        DrwBreakWindow        *window = DRW_BREAK_WINDOW (object);
	DrwBreakWindowPrivate *priv = window->priv;
	GtkWidget             *vbox;
	GtkWidget             *hbox;
	GtkWidget             *align;
	GtkWidget             *monitor_box;
	gchar                 *str;
	GtkWidget             *outer_vbox;
	GtkWidget             *button_box;
	gboolean               allow_postpone;
	GtkWidget             *lock_button;

	gint                   root_monitor = 0;
	GdkScreen             *screen = NULL;
	GdkRectangle           monitor;
	gint                   right_padding;
	gint                   bottom_padding;


        priv->settings = g_settings_new (DRW_SETTINGS_SCHEMA_ID);

	priv->break_time = g_settings_get_int (priv->settings, "break-time");

        allow_postpone = g_settings_get_boolean (priv->settings, "allow-postpone");
        priv->postpone_delay = POSTPONE_DELAY;

	priv->elapsed_idle_time = 0;

	gtk_window_set_keep_above (GTK_WINDOW (window), TRUE);
	gtk_window_fullscreen (GTK_WINDOW (window));
	gtk_window_set_modal (GTK_WINDOW (window), TRUE);

	screen = gdk_screen_get_default ();
	gdk_screen_get_monitor_geometry (screen, root_monitor, &monitor);

	gtk_window_set_default_size (GTK_WINDOW (window),
				     gdk_screen_get_width (screen),
				     gdk_screen_get_height (screen));

	gtk_window_set_decorated (GTK_WINDOW (window), FALSE);
	gtk_widget_set_app_paintable (GTK_WIDGET (window), TRUE);
	drw_setup_background (GTK_WIDGET (window));

	align = gtk_alignment_new (0.5, 0.5, 0.0, 0.0);
	gtk_widget_show (align);

	outer_vbox = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (outer_vbox);

	right_padding = gdk_screen_get_width (screen) - monitor.width - monitor.x;
	bottom_padding = gdk_screen_get_height (screen) - monitor.height - monitor.y;

	monitor_box = gtk_alignment_new (0.5, 0.5, 1, 1);
	gtk_alignment_set_padding (GTK_ALIGNMENT (monitor_box),
				   monitor.y,
				   bottom_padding,
				   monitor.x,
				   right_padding);
	gtk_widget_show (monitor_box);

	gtk_container_add (GTK_CONTAINER (window), monitor_box);
	gtk_container_add (GTK_CONTAINER (monitor_box), outer_vbox);

	gtk_box_pack_start (GTK_BOX (outer_vbox), align, TRUE, TRUE, 0);

	if (allow_postpone || priv->enable_lock) {
		button_box = gtk_hbutton_box_new ();
                gtk_button_box_set_layout (GTK_BUTTON_BOX (button_box), GTK_BUTTONBOX_END);
                gtk_container_set_border_width (GTK_CONTAINER (button_box), 12);
                gtk_box_set_spacing (GTK_BOX (button_box), 12);

                gtk_box_pack_start (GTK_BOX (outer_vbox), button_box, FALSE, FALSE, 0);
                gtk_widget_show (button_box);
        }

        if (allow_postpone) {
		priv->postpone_button = gtk_button_new_with_mnemonic (_("_Postpone Break"));
                gtk_button_set_focus_on_click (GTK_BUTTON (priv->postpone_button), FALSE);
		gtk_widget_show (priv->postpone_button);

                if (priv->postpone_delay > 0) {
                        gtk_widget_set_sensitive (priv->postpone_button, FALSE);

                        priv->postpone_sensitize_id = g_timeout_add (priv->postpone_delay,
                                                                     (GSourceFunc) postpone_sensitize_cb,
                                                                     window);
                }

		g_signal_connect (priv->postpone_button,
				  "clicked",
				  G_CALLBACK (postpone_clicked_cb),
				  window);

		gtk_box_pack_end (GTK_BOX (button_box), priv->postpone_button, FALSE, TRUE, 0);

		priv->postpone_entry = gtk_entry_new ();
		gtk_entry_set_has_frame (GTK_ENTRY (priv->postpone_entry), FALSE);

		gtk_box_pack_end (GTK_BOX (button_box), priv->postpone_entry, FALSE, TRUE, 4);
	}

        if (priv->enable_lock) {
                lock_button = gtk_button_new_with_mnemonic (_("_Lock Screen"));
                gtk_widget_show (lock_button);
                gtk_box_pack_start (GTK_BOX (button_box), lock_button, FALSE, FALSE, 0);
                g_signal_connect (lock_button,
                                  "clicked",
                                  G_CALLBACK (lock_clicked_cb),
                                  window);
        }

	vbox = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox);

	gtk_container_add (GTK_CONTAINER (align), vbox);

	hbox = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, FALSE, 0);

	priv->image = gtk_image_new_from_stock (GTK_STOCK_STOP, GTK_ICON_SIZE_DIALOG);
	gtk_misc_set_alignment (GTK_MISC (priv->image), 1, 0.5);
	gtk_widget_show (priv->image);
	gtk_box_pack_start (GTK_BOX (hbox), priv->image, TRUE, TRUE, 8);

	priv->break_label = gtk_label_new (NULL);
	gtk_widget_show (priv->break_label);

	str = g_strdup_printf ("<span size=\"xx-large\" foreground=\"white\"><b>%s</b></span>",
			       _("Take a break!"));
	gtk_label_set_markup (GTK_LABEL (priv->break_label), str);
	g_free (str);

	gtk_box_pack_start (GTK_BOX (hbox), priv->break_label, FALSE, FALSE, 12);

	priv->clock_label = gtk_label_new (NULL);
	gtk_misc_set_alignment (GTK_MISC (priv->clock_label), 0.5, 0.5);
	gtk_widget_show (priv->clock_label);
	gtk_box_pack_start (GTK_BOX (vbox), priv->clock_label, TRUE, TRUE, 8);

	gtk_window_stick (GTK_WINDOW (window));

        gtk_window_set_focus (GTK_WINDOW (window), NULL);

	priv->timer = drw_timer_new ();

	/* Make sure we have a valid time label from the start. */
	clock_timeout_cb (window);

	priv->clock_timeout_id = g_timeout_add (1000,
						(GSourceFunc) clock_timeout_cb,
						window);
	ca_context_play (ca_gtk_context_get (), 0, CA_PROP_EVENT_ID, "desktop-screen-lock", NULL);
}

static void
drw_break_window_finalize (GObject *object)
{
	DrwBreakWindow        *window = DRW_BREAK_WINDOW (object);
	DrwBreakWindowPrivate *priv = window->priv;

	if (priv->clock_timeout_id != 0) {
		g_source_remove (priv->clock_timeout_id);
	}

	if (priv->postpone_timeout_id != 0) {
		g_source_remove (priv->postpone_timeout_id);
	}

	if (priv->postpone_sensitize_id != 0) {
		g_source_remove (priv->postpone_sensitize_id);
	}

        g_object_unref (priv->settings);

	G_OBJECT_CLASS (drw_break_window_parent_class)->finalize (object);
}

static void
drw_break_window_dispose (GObject *object)
{
	DrwBreakWindow        *window = DRW_BREAK_WINDOW (object);
	DrwBreakWindowPrivate *priv = window->priv;

	if (priv->timer) {
		drw_timer_destroy (priv->timer);
		priv->timer = NULL;
	}

	if (priv->clock_timeout_id != 0) {
		g_source_remove (priv->clock_timeout_id);
		priv->clock_timeout_id = 0;
	}

	if (priv->postpone_timeout_id != 0) {
		g_source_remove (priv->postpone_timeout_id);
		priv->postpone_timeout_id = 0;
	}

	if (priv->postpone_sensitize_id != 0) {
		g_source_remove (priv->postpone_sensitize_id);
	}

        G_OBJECT_CLASS (drw_break_window_parent_class)->dispose (object);
}

GtkWidget *
drw_break_window_new (gboolean enable_lock)
{
	GObject *object;

	object = g_object_new (DRW_TYPE_BREAK_WINDOW,
			       "type", GTK_WINDOW_POPUP,
			       "skip-taskbar-hint", TRUE,
			       "skip-pager-hint", TRUE,
			       "focus-on-map", TRUE,
                               "enable-lock", enable_lock,
			       NULL);

	return GTK_WIDGET (object);
}

void
drw_break_window_set_elapsed_idle_time (DrwBreakWindow *window, gint seconds)
{
	/* account for elapsed idle time in blocks of ELAPSED_IDLE_INTERVAL */
	window->priv->elapsed_idle_time =
		(seconds / ELAPSED_IDLE_INTERVAL) * ELAPSED_IDLE_INTERVAL;
}

static gboolean
postpone_sensitize_cb (DrwBreakWindow *window)
{
	DrwBreakWindowPrivate *priv;

	priv = window->priv;

	gtk_widget_set_sensitive (priv->postpone_button, TRUE);

	priv->postpone_sensitize_id = 0;
	return FALSE;
}

static gboolean
clock_timeout_cb (DrwBreakWindow *window)
{
	DrwBreakWindowPrivate *priv;
	gchar                 *txt;
	gint                   minutes;
	gint                   seconds;

	g_return_val_if_fail (DRW_IS_BREAK_WINDOW (window), FALSE);

	priv = window->priv;

	seconds = priv->break_time - drw_timer_elapsed (priv->timer) - priv->elapsed_idle_time;
	seconds = MAX (0, seconds);

	if (seconds == 0) {
		/* Zero this out so the finalizer doesn't try to remove the
		 * source, which would be done in the timeout callback ==
		 * no-no.
		 */
		priv->clock_timeout_id = 0;

		ca_context_play (ca_gtk_context_get (), 0, CA_PROP_EVENT_ID, "alarm-clock-elapsed", NULL);
		g_signal_emit (window, signals[DONE], 0, NULL);

		return FALSE;
	}

	minutes = seconds / 60;
	seconds -= minutes * 60;

	txt = g_strdup_printf ("<span size=\"25000\" foreground=\"white\"><b>%d:%02d</b></span>",
			       minutes,
			       seconds);
	gtk_label_set_markup (GTK_LABEL (priv->clock_label), txt);
	g_free (txt);

	return TRUE;
}

static void
postpone_entry_activate_cb (GtkWidget      *entry,
			  DrwBreakWindow *window)
{
        DrwBreakWindowPrivate *priv = window->priv;
	const gchar *str, *phrase;

	str = gtk_entry_get_text (GTK_ENTRY (entry));

        g_settings_get (priv->settings, "unlock-phrase", "&s", &phrase);

	if (!strcmp (str, phrase)) {
		g_signal_emit (window, signals[POSTPONE], 0, NULL);
		return;
	}

	gtk_entry_set_text (GTK_ENTRY (entry), "");
}

static gboolean
grab_on_window (GdkWindow *window,
		guint32    activate_time)
{
	if ((gdk_pointer_grab (window, TRUE,
			       GDK_BUTTON_PRESS_MASK |
			       GDK_BUTTON_RELEASE_MASK |
			       GDK_POINTER_MOTION_MASK,
			       NULL, NULL, activate_time) == 0)) {
		if (gdk_keyboard_grab (window, TRUE,
			       activate_time) == 0)
			return TRUE;
		else {
			gdk_pointer_ungrab (activate_time);
			return FALSE;
		}
	}

	return FALSE;
}

static gboolean
postpone_cancel_cb (DrwBreakWindow *window)
{
	DrwBreakWindowPrivate *priv;

	priv = window->priv;

	gtk_entry_set_text (GTK_ENTRY (priv->postpone_entry), "");
	gtk_widget_hide (priv->postpone_entry);

	priv->postpone_timeout_id = 0;

	return FALSE;
}

static gboolean
postpone_entry_key_press_event_cb (GtkEntry       *entry,
				   GdkEventKey    *event,
				   DrwBreakWindow *window)
{
	DrwBreakWindowPrivate *priv;

	priv = window->priv;

	if (event->keyval == GDK_KEY_Escape) {
		if (priv->postpone_timeout_id) {
			g_source_remove (priv->postpone_timeout_id);
		}

		postpone_cancel_cb (window);

		return TRUE;
	}

	g_source_remove (priv->postpone_timeout_id);

	priv->postpone_timeout_id = g_timeout_add_seconds (POSTPONE_CANCEL, (GSourceFunc) postpone_cancel_cb, window);

	return FALSE;
}

static void
postpone_clicked_cb (GtkWidget *button,
		     GtkWidget *window)
{
	DrwBreakWindow        *bw = DRW_BREAK_WINDOW (window);
	DrwBreakWindowPrivate *priv = bw->priv;
	const gchar           *phrase;

	/* Disable the phrase for now. */
	phrase = NULL; /* g_settings_get_string (priv->settings, "unlock-phrase", "&s", &phrase); */

	if (!phrase || !phrase[0]) {
		g_signal_emit (window, signals[POSTPONE], 0, NULL);
		return;
	}

	if (gtk_widget_get_visible (priv->postpone_entry)) {
		gtk_widget_activate (priv->postpone_entry);
		return;
	}

	gtk_widget_show (priv->postpone_entry);

	priv->postpone_timeout_id = g_timeout_add_seconds (POSTPONE_CANCEL, (GSourceFunc) postpone_cancel_cb, bw);

	grab_on_window (gtk_widget_get_window (priv->postpone_entry),  gtk_get_current_event_time ());

	gtk_widget_grab_focus (priv->postpone_entry);

	g_signal_connect (priv->postpone_entry,
			  "activate",
			  G_CALLBACK (postpone_entry_activate_cb),
			  bw);

	g_signal_connect (priv->postpone_entry,
			  "key_press_event",
			  G_CALLBACK (postpone_entry_key_press_event_cb),
			  bw);
}
