SUBDIRS = src po

ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS}

DISTCHECK_CONFIGURE_FLAGS =

EXTRA_DIST = 				\
	AUTHORS 			\
	ChangeLog			\
	autogen.sh			\
	drwright.doap			\
	intltool-extract.in             \
	intltool-merge.in               \
	intltool-update.in

DISTCLEANFILES = 		\
	intltool-extract	\
	intltool-merge		\
	intltool-update		\
	$(desktop_files)

MAINTAINERCLEANFILES = \
	$(srcdir)/ChangeLog \
	$(srcdir)/INSTALL \
	$(srcdir)/aclocal.m4 \
	$(srcdir)/autoscan.log \
	$(srcdir)/compile \
	$(srcdir)/config.guess \
	$(srcdir)/config.h.in \
	$(srcdir)/config.sub \
	$(srcdir)/configure.scan \
	$(srcdir)/depcomp \
	$(srcdir)/install-sh \
	$(srcdir)/ltmain.sh \
	$(srcdir)/missing \
	$(srcdir)/mkinstalldirs \
	$(srcdir)/omf.make \
	$(srcdir)/xmldocs.make \
	$(srcdir)/gnome-doc-utils.make  \
	`find "$(srcdir)" -type f -name Makefile.in -print` \
	$(NULL)

# Build ChangeLog from GIT  history
ChangeLog:
	$(AM_V_GEN) if test -d $(top_srcdir)/.git; then \
		GIT_DIR="$(top_srcdir)/.git" git log --stat > $@; \
	fi

dist: ChangeLog

.PHONY: ChangeLog
-include $(top_srcdir)/git.mk
