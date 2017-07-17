lib_LTLIBRARIES = libmodulemd-@API_VERSION@.la

libmodulemd_@API_VERSION@_la_SOURCES = \
	modulemd-version.h \
	modulemd.h \
	modulemd.c \
	modulemd-modulemetadata.h \
	modulemd-modulemetadata.c \
	$(NULL)

libmodulemd_@API_VERSION@_la_CFLAGS = \
	$(GOBJECT_CFLAGS) \
	$(WARN_CFLAGS) \
	$(NULL)

libmodulemd_@API_VERSION@_la_LIBADD = \
	$(GOBJECT_LIBS) \
	$(NULL)

libmodulemd_@API_VERSION@_la_LDFLAGS = \
	$(WARN_LDFLAGS) \
	$(NULL)

-include $(top_srcdir)/git.mk

CLEANFILES =

-include $(INTROSPECTION_MAKEFILE)
INTROSPECTION_GIRS =
INTROSPECTION_SCANNER_ARGS = --add-include-path=$(srcdir) --warn-all
INTROSPECTION_COMPILER_ARGS = --includedir=$(srcdir)

if HAVE_INTROSPECTION
introspection_sources = modulemd.h \
			modulemd.c \
			modulemd-modulemetadata.h \
			modulemd-modulemetadata.c

modulemd-@API_VERSION@.gir: libmodulemd-@API_VERSION@.la
modulemd_@API_VERSION@_gir_INCLUDES = GObject-2.0
modulemd_@API_VERSION@_gir_CFLAGS = $(INCLUDES)
modulemd_@API_VERSION@_gir_LIBS = libmodulemd-@API_VERSION@.la
modulemd_@API_VERSION@_gir_FILES = $(introspection_sources)
INTROSPECTION_GIRS += modulemd-@API_VERSION@.gir

girdir = $(datadir)/gir-1.0
gir_DATA = $(INTROSPECTION_GIRS)

typelibdir = $(libdir)/girepository-1.0
typelib_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

CLEANFILES += $(gir_DATA) $(typelib_DATA)
endif
