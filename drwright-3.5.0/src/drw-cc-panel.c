/*
 * Copyright (C) 2010 Intel, Inc
 * Copyright © 2010 Christian Persch
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
 * Author: Thomas Wood <thomas.wood@intel.com>
 *         Christian Persch <chpe@gnome.org>
 */

#include <config.h>

#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <gio/gio.h>
#include <gtk/gtk.h>

#include "drw-cc-panel.h"

#define DRW_SETTINGS_SCHEMA_ID "org.gnome.settings-daemon.plugins.typing-break"

/* from libgnome-control-center */
#define CC_SHELL_PANEL_EXTENSION_POINT "control-center-1"

static GType drw_cc_panel_type;
static gpointer drw_cc_panel_parent_class;

#if 0
#define DRW_PANEL_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), DRW_TYPE_CC_PANEL, DrwCcPanelPrivate))

struct _DrwCcPanelPrivate
{
  gpointer dummy;
};
#endif

static int
spinbutton_input_cb (GtkWidget *button,
                     gpointer *ret,
                     gpointer user_data)
{
  const char *text;
  char *end;
  gint64 value, v;

  text = gtk_entry_get_text (GTK_ENTRY (button));

  value = 0;
  do {
    value *= 60;

    errno = 0;
    end = NULL;
    v = strtoll (text, &end, 10);
    if (errno ||
        !end || (*end != '\0' && *end != ':') ||
        v < 0 || v >= 60) {
      return GTK_INPUT_ERROR;
    }

    value += v;
    text = strchr (text, ':');
    if (text)
      text++;
  } while (text);

  * (gdouble *) ret = value;
  return TRUE;
}

static gboolean
spinbutton_output_cb (GtkSpinButton *button,
                      gpointer user_data)
{
  char buf[64];
  int value;
  int h, m, s;

  value = (int) (gtk_spin_button_get_value (button) + .5);

  h = value / 3600;
  value %= 3600;
  m = value / 60;
  s = value % 60;

  if (h > 0)
    g_snprintf (buf, sizeof (buf), "%u:%02u:%02u", h, m, s);
  else
    g_snprintf (buf, sizeof (buf), "%u:%02u", m, s);

  gtk_entry_set_text (GTK_ENTRY (button), buf);
  return TRUE;
}

static void
drw_cc_panel_get_property (GObject    *object,
                               guint       property_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  switch (property_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
drw_cc_panel_set_property (GObject      *object,
                               guint         property_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  switch (property_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
drw_cc_panel_dispose (GObject *object)
{
  G_OBJECT_CLASS (drw_cc_panel_parent_class)->dispose (object);
}

static void
drw_cc_panel_finalize (GObject *object)
{
  G_OBJECT_CLASS (drw_cc_panel_parent_class)->finalize (object);
}

static void
drw_cc_panel_class_init (DrwCcPanelClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  drw_cc_panel_parent_class = g_type_class_peek_parent (klass);

  /* g_type_class_add_private (klass, sizeof (DrwCcPanelPrivate)); */

  object_class->get_property = drw_cc_panel_get_property;
  object_class->set_property = drw_cc_panel_set_property;
  object_class->dispose = drw_cc_panel_dispose;
  object_class->finalize = drw_cc_panel_finalize;
}

static void
drw_cc_panel_class_finalize (DrwCcPanelClass *klass)
{
}

static void
drw_cc_panel_init (DrwCcPanel *self)
{
  GtkBuilder *builder;
  GError *error = NULL;
  GtkWidget *widget;
  GSettings *settings;

  /* priv = self->priv = DRW_CC_PANEL_GET_PRIVATE (self); */

  builder = gtk_builder_new ();
  gtk_builder_set_translation_domain (builder, GETTEXT_PACKAGE);

  if (!gtk_builder_add_from_resource (builder, "/org/gnome/drwright/drwright-panel.ui", &error))
    g_error ("%s", error->message);

  widget = (GtkWidget *) gtk_builder_get_object (builder, "typing-break-preferences-box");
  gtk_widget_reparent (widget, GTK_WIDGET (self));

  widget = (GtkWidget *) gtk_builder_get_object (builder, "typing-break-preferences-dialog");
  gtk_widget_destroy (widget);

  /* Now connect the settings */
  settings = g_settings_new (DRW_SETTINGS_SCHEMA_ID);

  g_settings_bind (settings, "enabled",
                   gtk_builder_get_object (builder, "break-enabled-checkbutton"),
                   "active",
                   G_SETTINGS_BIND_DEFAULT);
  g_settings_bind (settings, "enabled",
                   gtk_builder_get_object (builder, "inner-box"),
                   "sensitive",
                   G_SETTINGS_BIND_GET);
  widget = (GtkWidget *) gtk_builder_get_object (builder, "work-interval-spinbutton");
  gtk_entry_set_width_chars (GTK_ENTRY (widget), 8);
  gtk_entry_set_alignment (GTK_ENTRY (widget), 1.0);
  g_signal_connect (widget, "input",
                    G_CALLBACK (spinbutton_input_cb), NULL);
  g_signal_connect (widget, "output",
                    G_CALLBACK (spinbutton_output_cb), NULL);
  g_settings_bind (settings, "type-time",
                   gtk_spin_button_get_adjustment (GTK_SPIN_BUTTON (widget)),
                   "value",
                   G_SETTINGS_BIND_DEFAULT);
  widget = (GtkWidget *) gtk_builder_get_object (builder, "break-interval-spinbutton");
  gtk_entry_set_width_chars (GTK_ENTRY (widget), 8);
  gtk_entry_set_alignment (GTK_ENTRY (widget), 1.0);
  g_signal_connect (widget, "input",
                    G_CALLBACK (spinbutton_input_cb), NULL);
  g_signal_connect (widget, "output",
                    G_CALLBACK (spinbutton_output_cb), NULL);
  g_settings_bind (settings, "break-time",
                   gtk_spin_button_get_adjustment (GTK_SPIN_BUTTON (widget)),
                   "value",
                   G_SETTINGS_BIND_DEFAULT);
  g_settings_bind (settings, "allow-postpone",
                   gtk_builder_get_object (builder, "allow-postpone-checkbutton"),
                   "active",
                   G_SETTINGS_BIND_DEFAULT);

  g_object_unref (settings);
  g_object_unref (builder);
}

void
drw_cc_panel_register_type (GTypeModule *module)
{
  GType parent_type, g_define_type_id;
  GTypeQuery query;
  GTypeInfo type_info = {
    0,
    (GBaseInitFunc) NULL,
    (GBaseFinalizeFunc) NULL,
    (GClassInitFunc) drw_cc_panel_class_init,
    (GClassFinalizeFunc) drw_cc_panel_class_finalize,
    NULL,
    0,
    0,
    (GInstanceInitFunc) drw_cc_panel_init,
    NULL
  };

  parent_type = g_type_from_name ("CcPanel");
  if (parent_type == 0 || !g_type_is_a (parent_type, GTK_TYPE_WIDGET)) {
    drw_cc_panel_type = G_TYPE_INVALID;
    return;
  }

  g_type_query (parent_type, &query);
  type_info.class_size = query.class_size;
  type_info.instance_size = query.instance_size;

  g_define_type_id =
    drw_cc_panel_type =
      g_type_module_register_type (module,
                                   parent_type,
                                   "DrwCcPanel",
                                   &type_info,
                                   0);

  g_io_extension_point_implement (CC_SHELL_PANEL_EXTENSION_POINT,
                                  g_define_type_id,
                                  "typing-break", 0);
}

GType
drw_cc_panel_get_type (void)
{
  return drw_cc_panel_type;
}
