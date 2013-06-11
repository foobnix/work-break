/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2002 CodeFactory AB
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
#include <stdlib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "drw-selection.h"
#include "drwright.h"

gboolean debug = FALSE;

int main(int argc, char *argv[]) {
	DrWright *drwright;
	DrwSelection *selection;
	const GOptionEntry options[] = { { "debug", 'd', 0, G_OPTION_ARG_NONE,
			&debug, N_("Enable debugging code"), NULL }, { NULL } };
	GOptionContext *option_context;
	GError *error = NULL;
	gboolean retval;

	bindtextdomain(GETTEXT_PACKAGE, GNOMELOCALEDIR);
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
	textdomain(GETTEXT_PACKAGE);

	option_context = g_option_context_new(NULL );
	g_option_context_set_translation_domain(option_context, GETTEXT_PACKAGE);
	g_option_context_add_main_entries(option_context, options, GETTEXT_PACKAGE);
	g_option_context_add_group(option_context, gtk_get_option_group(TRUE));

	retval = g_option_context_parse(option_context, &argc, &argv, &error);
	g_option_context_free(option_context);
	if (!retval) {
		g_print("%s\n", error->message);
		g_error_free(error);
		exit(1);
	}

	g_set_application_name(_("Typing Monitor"));
	gtk_window_set_default_icon_name("typing-monitor");

	selection = drw_selection_start();
	if (!drw_selection_is_master(selection)) {
		g_message("The typing monitor is already running, exiting.");
		return 0;
	}

	drwright = drwright_new();

	gtk_main();

	return 0;
}
