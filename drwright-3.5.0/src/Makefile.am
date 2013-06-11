NULL =

EXTRA_DIST =

BUILT_SOURCES = \
	drw-resources.c \
	drw-resources.h \
	drw-cc-resources.c \
	drw-cc-resources.h \
	$(NULL)

# Typing break

pkglibexec_PROGRAMS = gnome-typing-monitor

gnome_typing_monitor_SOURCES =			\
	main.c					\
	drwright.c				\
	drwright.h				\
	drw-break-window.c			\
	drw-break-window.h			\
	drw-monitor.c				\
	drw-monitor.h				\
	drw-utils.c				\
	drw-utils.h				\
	drw-selection.c				\
	drw-selection.h				\
	drw-timer.c				\
	drw-timer.h \
	$(NULL)

nodist_gnome_typing_monitor_SOURCES = \
	drw-resources.c \
	drw-resources.h \
	$(NULL)

gnome_typing_monitor_CPPFLAGS = \
	-DGNOMELOCALEDIR="\"$(datadir)/locale\""	\
	-DBINDIR="\"$(bindir)\""			\
	$(AM_CPPFLAGS)
gnome_typing_monitor_CFLAGS = \
	$(TYPING_CFLAGS)	\
	$(AM_CFLAGS)
gnome_typing_monitor_LDADD = \
	$(TYPING_LIBS)		\
	$(SCREENSAVER_LIBS)	\
	$(LIBM) \
	$(INTLLIBS)

EXTRA_DIST += ocean-stripes.png

# Themeable application icon
icondir = $(datadir)/icons/hicolor/48x48/apps
dist_icon_DATA = typing-monitor.png
svgicondir = $(datadir)/icons/hicolor/scalable/apps
dist_svgicon_DATA = typing-monitor.svg
gtk_update_icon_cache = gtk-update-icon-cache -f -t $(datadir)/icons/hicolor
install-data-hook: update-icon-cache
uninstall-hook: update-icon-cache
update-icon-cache:
	@-if test -z "$(DESTDIR)"; then \
		echo "Updating Gtk icon cache."; \
		$(gtk_update_icon_cache); \
	else \
		echo "*** Icon cache not updated.  After (un)install, run this:"; \
		echo "***   $(gtk_update_icon_cache)"; \
	fi

# GSD plugin

gsd_plugin_LTLIBRARIES = \
	libtyping-break.la		\
	$(NULL)

libtyping_break_la_SOURCES = 		\
	gsd-typing-break-plugin.h	\
	gsd-typing-break-plugin.c	\
	gsd-typing-break-manager.h	\
	gsd-typing-break-manager.c	\
	gnome-settings-profile.h	\
	$(NULL)

libtyping_break_la_CPPFLAGS = \
	-DGNOME_SETTINGS_LOCALEDIR=\""$(datadir)/locale"\" \
	-DPKGLIBEXECDIR=\""$(pkglibexecdir)"\"	\
	$(AM_CPPFLAGS)

libtyping_break_la_CFLAGS = \
	$(GSD_PLUGIN_CFLAGS)	\
	$(AM_CFLAGS)

libtyping_break_la_LDFLAGS = \
	-export-dynamic \
	-export-symbols-regex '^register_gnome_settings_plugin' \
	-module \
	-avoid-version \
	-no-undefined \
	$(AM_LDFLAGS)

libtyping_break_la_LIBADD  = 		\
	$(GSD_PLUGIN_LIBS)	\
	$(INTLLIBS)

gsd_plugin_in_files = 		\
	typing-break.gnome-settings-plugin.in	\
	$(NULL)

gsd_plugin_DATA = $(gsd_plugin_in_files:.gnome-settings-plugin.in=.gnome-settings-plugin)

@GSD_INTLTOOL_PLUGIN_RULE@

# Control Centre panel

ccpanels_LTLIBRARIES = libdrwrightccp.la

libdrwrightccp_la_SOURCES = \
	drw-cc-module.c	\
	drw-cc-panel.c	\
	drw-cc-panel.h	\
	$(NULL)
nodist_libdrwrightccp_la_SOURCES = \
	drw-cc-resources.c \
	drw-cc-resources.h \
	$(NULL)
libdrwrightccp_la_CPPFLAGS = \
        -DLOCALEDIR="\"$(datadir)/locale\""	\
	$(AM_CPPFLAGS)
libdrwrightccp_la_CFLAGS = \
	$(CCPANEL_CFLAGS)	\
	$(AM_CFLAGS)
libdrwrightccp_la_LIBADD = \
	$(CCPANEL_LIBS)	\
	$(INTLLIBS)
libdrwrightccp_la_LDFLAGS = \
	-export-dynamic \
	-export-symbols-regex '^g_io_module_(load|unload)' \
	-avoid-version \
	-module \
	-no-undefined \
	$(AM_LDFLAGS)

desktopdir = $(datadir)/applications
desktop_in_files = gnome-typing-break-panel.desktop.in
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)

builder_in_files = drwright-panel.glade

%.ui: %.glade
	$(AM_V_GEN) $(GTK_BUILDER_CONVERT) $< $@ >/dev/null

@INTLTOOL_DESKTOP_RULE@

# Resources

drw-resources.h drw-resources.c: drwright.gresource.xml Makefile $(shell $(GLIB_COMPILE_RESOURCES) --generate-dependencies --sourcedir $(srcdir) $(srcdir)/drwright.gresource.xml)
	$(AM_V_GEN) $(GLIB_COMPILE_RESOURCES) --target $@ --sourcedir $(srcdir) --generate --c-name drw $<

# hack! https://bugzilla.gnome.org/show_bug.cgi?id=673101
drwright-cc.gresource.xml: drwright-panel.ui

drw-cc-resources.h drw-cc-resources.c: drwright-cc.gresource.xml Makefile $(shell $(GLIB_COMPILE_RESOURCES) --generate-dependencies --sourcedir $(srcdir) --sourcedir . $(srcdir)/drwright-cc.gresource.xml)
	$(AM_V_GEN) XMLLINT=$(XMLLINT) $(GLIB_COMPILE_RESOURCES) --target $@ --sourcedir $(srcdir) --sourcedir . --generate --c-name drw_cc $<

# GSettings

gsettings_schemas_in_files = org.gnome.settings-daemon.plugins.typing-break.gschema.xml.in
gsettings_SCHEMAS = $(gsettings_schemas_in_files:.gschema.xml.in=.gschema.xml)

# gsettings_convertdir = $(datadir)/GConf/gsettings
# dist_gsettings_convert_DATA = drwright.convert

@GSETTINGS_RULES@
@INTLTOOL_XML_NOMERGE_RULE@

# Clean

EXTRA_DIST += 			\
	$(builder_in_files)	\
	$(gsd_plugin_in_files)	\
	$(gsettings_schemas_in_files) 	\
	drwright.gresource.xml \
	drwright-cc.gresource.xml \
	$(NULL)

CLEANFILES = 			\
	$(builder_in_files:.glade=.ui) \
	$(desktop_DATA)		\
	$(gsettings_SCHEMAS)	\
	$(gsd_plugin_DATA)	\
	$(BUILT_SOURCES)	\
	$(NULL)

DISTCLEANFILES =		\
	$(builder_in_files:.glade=.ui) \
	$(desktop_DATA)		\
	$(gsd_plugin_DATA)	\
	$(BUILT_SOURCES)	\
	$(NULL)

# includes

-include $(top_srcdir)/git.mk
