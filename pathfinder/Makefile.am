## Process this file with automake to produce Makefile.in

AM_CXXFLAGS = @X_CFLAGS@
INCLUDES = -I$(top_srcdir)/include -I$(top_builddir)/include
RESWRAP  = $(top_builddir)/utils/reswrap$(EXEEXT)
SUFFIXES = .gif .bmp
LDADD	 = $(top_builddir)/lib/libFOX-1.7.la -lm

bin_PROGRAMS	= PathFinder

man_MANS = PathFinder.1

ICONS = $(top_srcdir)/pathfinder/addbook.bmp \
$(top_srcdir)/pathfinder/bigicons.bmp \
$(top_srcdir)/pathfinder/closepanel.gif \
$(top_srcdir)/pathfinder/clrbook.gif \
$(top_srcdir)/pathfinder/copy.bmp \
$(top_srcdir)/pathfinder/config_gif.gif \
$(top_srcdir)/pathfinder/copyit.gif \
$(top_srcdir)/pathfinder/cut.bmp \
$(top_srcdir)/pathfinder/delbook.bmp \
$(top_srcdir)/pathfinder/deleteit.bmp \
$(top_srcdir)/pathfinder/desktop.bmp \
$(top_srcdir)/pathfinder/details.bmp \
$(top_srcdir)/pathfinder/dirup.bmp \
$(top_srcdir)/pathfinder/enter.gif \
$(top_srcdir)/pathfinder/execute.bmp \
$(top_srcdir)/pathfinder/file_gif.gif \
$(top_srcdir)/pathfinder/fileshown.gif \
$(top_srcdir)/pathfinder/filehidden.gif \
$(top_srcdir)/pathfinder/foldernew.gif \
$(top_srcdir)/pathfinder/foxbig.gif \
$(top_srcdir)/pathfinder/foxmini.gif \
$(top_srcdir)/pathfinder/goback.bmp \
$(top_srcdir)/pathfinder/goforw.bmp \
$(top_srcdir)/pathfinder/gotodir.bmp \
$(top_srcdir)/pathfinder/home.gif \
$(top_srcdir)/pathfinder/hosts.bmp \
$(top_srcdir)/pathfinder/iconpath.gif \
$(top_srcdir)/pathfinder/linkit.bmp \
$(top_srcdir)/pathfinder/location.gif \
$(top_srcdir)/pathfinder/maphost.bmp \
$(top_srcdir)/pathfinder/miscellaneous_gif.gif \
$(top_srcdir)/pathfinder/moveit.gif \
$(top_srcdir)/pathfinder/paste.bmp \
$(top_srcdir)/pathfinder/pattern_gif.gif \
$(top_srcdir)/pathfinder/properties.bmp \
$(top_srcdir)/pathfinder/quit_gif.gif \
$(top_srcdir)/pathfinder/newfolder.bmp \
$(top_srcdir)/pathfinder/renameit.gif \
$(top_srcdir)/pathfinder/rotateleft.gif \
$(top_srcdir)/pathfinder/rotateright.gif \
$(top_srcdir)/pathfinder/run_bmp.bmp \
$(top_srcdir)/pathfinder/setbook.gif \
$(top_srcdir)/pathfinder/setdir.gif \
$(top_srcdir)/pathfinder/sorting.bmp \
$(top_srcdir)/pathfinder/smallicons.bmp \
$(top_srcdir)/pathfinder/unmaphost.bmp \
$(top_srcdir)/pathfinder/warningicon_gif.gif \
$(top_srcdir)/pathfinder/work.gif

PathFinder_SOURCES =  \
icons.h \
icons.cpp \
CopyDialog.h \
CopyDialog.cpp \
PathFinder.cpp \
PathFinder.h \
Preferences.cpp \
Preferences.h \
PropertyDialog.cpp \
PropertyDialog.h

BUILT_SOURCES =	icons.h icons.cpp

icons.h: $(ICONS)
	$(RESWRAP) -h -o $@ $^

icons.cpp: $(ICONS)
	$(RESWRAP) -i icons.h -o $@ $^

CLEANFILES	= icons.h icons.cpp

EXTRA_DIST = \
$(man_MANS) \
LICENSE \
$(ICONS)

