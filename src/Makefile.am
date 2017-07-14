lib_LTLIBRARIES = libmodulemd-@API_VERSION@.la

libmodulemd_@API_VERSION@_la_SOURCES = \
	modulemd-version.h \
	modulemd.h \
	$(NULL)

libmodulemd_@API_VERSION@_la_CFLAGS = \
	$(MODULEMD_CFLAGS) \
	$(WARN_CFLAGS) \
	$(NULL)

libmodulemd_@API_VERSION@_la_LIBADD = \
	$(MODULEMD_LIBS) \
	$(NULL)

libmodulemd_@API_VERSION@_la_LDFLAGS = \
	$(WARN_LDFLAGS) \
	$(NULL)

-include $(top_srcdir)/git.mk
