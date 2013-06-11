/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*-
 *
 * Copyright (C) 2007 William Jon McCann <mccann@jhu.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 */

#include "config.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#include <locale.h>

#include <glib.h>
#include <gio/gio.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <gtk/gtk.h>

#include "gnome-settings-profile.h"
#include "gsd-typing-break-manager.h"

#define DRW_SETTINGS_SCHEMA_ID "org.gnome.settings-daemon.plugins.typing-break"
#define I_(string) (g_intern_static_string (string))

#define GSD_TYPING_BREAK_MANAGER_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), GSD_TYPE_TYPING_BREAK_MANAGER, GsdTypingBreakManagerPrivate))

struct GsdTypingBreakManagerPrivate
{
        GSettings *settings;
        GPid  typing_monitor_pid;
        guint typing_monitor_idle_id;
        guint child_watch_id;
        guint setup_id;
};

static void     gsd_typing_break_manager_class_init  (GsdTypingBreakManagerClass *klass);
static void     gsd_typing_break_manager_init        (GsdTypingBreakManager      *typing_break_manager);

G_DEFINE_TYPE (GsdTypingBreakManager, gsd_typing_break_manager, G_TYPE_OBJECT)

static gpointer manager_object = NULL;

static gboolean
typing_break_timeout (GsdTypingBreakManager *manager)
{
        if (manager->priv->typing_monitor_pid > 0) {
                kill (manager->priv->typing_monitor_pid, SIGKILL);
        }

        manager->priv->typing_monitor_idle_id = 0;

        return FALSE;
}

static void
child_watch (GPid                   pid,
             int                    status,
             GsdTypingBreakManager *manager)
{
        if (pid == manager->priv->typing_monitor_pid) {
                manager->priv->typing_monitor_pid = 0;
                g_spawn_close_pid (pid);
        }
}

static void
setup_typing_break (GsdTypingBreakManager *manager,
                    gboolean               enabled)
{
        gnome_settings_profile_start (NULL);

        if (! enabled) {
                if (manager->priv->typing_monitor_pid != 0) {
                        manager->priv->typing_monitor_idle_id = g_timeout_add_seconds (3, (GSourceFunc) typing_break_timeout, manager);
                }
                return;
        }

        if (manager->priv->typing_monitor_idle_id != 0) {
                g_source_remove (manager->priv->typing_monitor_idle_id);
                manager->priv->typing_monitor_idle_id = 0;
        }

        if (manager->priv->typing_monitor_pid == 0) {
                GError  *error;
                char    *argv[] = { PKGLIBEXECDIR"/gnome-typing-monitor",  NULL };
                gboolean res;

                error = NULL;
                res = g_spawn_async ("/",
                                     argv,
                                     NULL,
                                     G_SPAWN_STDOUT_TO_DEV_NULL
                                     | G_SPAWN_STDERR_TO_DEV_NULL
                                     | G_SPAWN_DO_NOT_REAP_CHILD,
                                     NULL,
                                     NULL,
                                     &manager->priv->typing_monitor_pid,
                                     &error);
                if (! res) {
                        /* FIXME: put up a warning */
                        g_warning ("failed: %s\n", error->message);
                        g_error_free (error);
                        manager->priv->typing_monitor_pid = 0;
                        return;
                }

                manager->priv->child_watch_id = g_child_watch_add (manager->priv->typing_monitor_pid,
                                                                   (GChildWatchFunc)child_watch,
                                                                   manager);
        }

        gnome_settings_profile_end (NULL);
}

static void
typing_break_enabled_changed_cb (GSettings *settings,
                                 const char *key,
                                 GsdTypingBreakManager *manager)
{
        g_assert (key == I_("enabled"));

        setup_typing_break (manager, g_settings_get_boolean (settings, key));
}

static gboolean
really_setup_typing_break (GsdTypingBreakManager *manager)
{
        setup_typing_break (manager, TRUE);
        manager->priv->setup_id = 0;
        return FALSE;
}

gboolean
gsd_typing_break_manager_start (GsdTypingBreakManager *manager,
                                GError               **error)
{
        GsdTypingBreakManagerPrivate *priv = manager->priv;

        g_debug ("Starting typing_break manager");
        gnome_settings_profile_start (NULL);

        priv->settings = g_settings_new (DRW_SETTINGS_SCHEMA_ID);
        g_signal_connect (priv->settings, "changed::enabled",
                          G_CALLBACK (typing_break_enabled_changed_cb),
                          manager);

        if (g_settings_get_boolean (priv->settings, "enabled")) {
                priv->setup_id = g_timeout_add_seconds (3,
                                                        (GSourceFunc) really_setup_typing_break,
                                                        manager);
        }

        gnome_settings_profile_end (NULL);

        return TRUE;
}

void
gsd_typing_break_manager_stop (GsdTypingBreakManager *manager)
{
        GsdTypingBreakManagerPrivate *p = manager->priv;

        g_debug ("Stopping typing_break manager");

        if (p->setup_id != 0) {
                g_source_remove (p->setup_id);
                p->setup_id = 0;
        }

        if (p->child_watch_id != 0) {
                g_source_remove (p->child_watch_id);
                p->child_watch_id = 0;
        }

        if (p->typing_monitor_idle_id != 0) {
                g_source_remove (p->typing_monitor_idle_id);
                p->typing_monitor_idle_id = 0;
        }

        if (p->typing_monitor_pid > 0) {
                kill (p->typing_monitor_pid, SIGKILL);
                g_spawn_close_pid (p->typing_monitor_pid);
                p->typing_monitor_pid = 0;
        }

        if (p->settings != NULL) {
                g_signal_handlers_disconnect_by_func (p->settings,
                                                      G_CALLBACK (typing_break_enabled_changed_cb),
                                                      manager);
                g_object_unref (p->settings);
                p->settings = NULL;
        }
}

static void
gsd_typing_break_manager_class_init (GsdTypingBreakManagerClass *klass)
{
        GObjectClass   *object_class = G_OBJECT_CLASS (klass);

        g_type_class_add_private (klass, sizeof (GsdTypingBreakManagerPrivate));
}

static void
gsd_typing_break_manager_init (GsdTypingBreakManager *manager)
{
        manager->priv = GSD_TYPING_BREAK_MANAGER_GET_PRIVATE (manager);
}

GsdTypingBreakManager *
gsd_typing_break_manager_new (void)
{
        if (manager_object != NULL) {
                g_object_ref (manager_object);
        } else {
                manager_object = g_object_new (GSD_TYPE_TYPING_BREAK_MANAGER, NULL);
                g_object_add_weak_pointer (manager_object,
                                           (gpointer *) &manager_object);
        }

        return GSD_TYPING_BREAK_MANAGER (manager_object);
}
