noinst_SCRIPTS =
if WINE_PLUGIN
  noinst_SCRIPTS += TrackIR.exe
endif #WINE_PLUGIN

if DARWIN
  LDFLAGS += -Wl,-no_arch_warnings
else
  LDFLAGS += -Wl,--no-warn-search-mismatch
endif

CC = winegcc

CXX = wineg++

SUFFIXES = .o .cpp .c .rc 64.o

.cpp.o :
	$(CXX) -c $(CXXFLAGS_PRE) $(CXXFLAGS) $(CPPFLAGS) -m32 -o $@ $<

.c.o :
	$(CC) -c $(CFLAGS_PRE) $(CFLAGS) $(CPPFLAGS) -m32 -o $@ $<

.cpp64.o :
	$(CXX) -c $(CXXFLAGS_PRE) $(CXXFLAGS) $(CPPFLAGS) -o $@ $<

.c64.o :
	$(CC) -c $(CFLAGS_PRE) $(CFLAGS) $(CPPFLAGS) -o $@ $<

.rc.o :
	wrc -o $@ $(RCFLAGS) $<

CXXFLAGS_PRE = -g -DHAVE_CONFIG_H -I../../.. -I. -I@srcdir@/../.. -I@top_builddir@
CFLAGS_PRE = -g -I../.. -I../../.. -DHAVE_CONFIG_H -I@srcdir@/../.. -I@top_builddir@
RCFLAGS = -I @srcdir@
#VPATH = .:../..:@srcdir@/../..:@top_builddir@:@srcdir@
vpath %.h @srcdir@/../..
vpath %.h @top_builddir@
vpath %.c @srcdir@
vpath %.c @srcdir@/../..

TrackIR.exe : main.o rest.o
	$(CXX) $(WINE_LIBS) $(LDFLAGS) -m32 -Wall -Wextra -g -o $@ $^

main.o : main.cpp resource.h

rest.o : rest.c rest.h

clean-local: clean-local-check
.PHONY: clean-local-check
clean-local-check:
	rm -f *.exe* *.dll* *.sh *.o

distclean-local: distclean-local-check
.PHONY: distclean-local-check
distclean-local-check:
	rm -f *.exe* *.dll* *.sh *.o

EXTRA_DIST = main.cpp resource.h rest.c rest.h

