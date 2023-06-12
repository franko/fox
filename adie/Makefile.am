## Process this file with automake to produce Makefile.in

AM_CXXFLAGS = @X_CFLAGS@
INCLUDES = -I$(top_srcdir)/include -I$(top_builddir)/include
RESWRAP  = $(top_builddir)/utils/reswrap$(EXEEXT)
SUFFIXES = .gif .bmp
LDADD = $(top_builddir)/lib/libFOX-1.7.la -lm


bin_PROGRAMS = adie

bin_SCRIPTS = Adie.stx

man_MANS = adie.1

ICONS  = \
$(top_srcdir)/adie/adie_gif.gif \
$(top_srcdir)/adie/backward_gif.gif \
$(top_srcdir)/adie/big_gif.gif \
$(top_srcdir)/adie/bookdel_gif.gif \
$(top_srcdir)/adie/booknext_gif.gif \
$(top_srcdir)/adie/bookprev_gif.gif \
$(top_srcdir)/adie/bookset_gif.gif \
$(top_srcdir)/adie/browser.gif \
$(top_srcdir)/adie/capitalize.gif \
$(top_srcdir)/adie/close_gif.gif \
$(top_srcdir)/adie/colors_gif.gif \
$(top_srcdir)/adie/command_bmp.bmp \
$(top_srcdir)/adie/config_gif.gif \
$(top_srcdir)/adie/copy_gif.gif \
$(top_srcdir)/adie/cut_gif.gif \
$(top_srcdir)/adie/delete_gif.gif \
$(top_srcdir)/adie/delimit_gif.gif \
$(top_srcdir)/adie/filehidden.gif \
$(top_srcdir)/adie/fileshown.gif \
$(top_srcdir)/adie/fonts_gif.gif \
$(top_srcdir)/adie/forward_gif.gif \
$(top_srcdir)/adie/goto_gif.gif \
$(top_srcdir)/adie/help_gif.gif \
$(top_srcdir)/adie/indent_gif.gif \
$(top_srcdir)/adie/info_gif.gif \
$(top_srcdir)/adie/lang_gif.gif \
$(top_srcdir)/adie/logger.gif \
$(top_srcdir)/adie/lowercase.gif \
$(top_srcdir)/adie/miscellaneous_gif.gif \
$(top_srcdir)/adie/new_gif.gif \
$(top_srcdir)/adie/nobrowser.gif \
$(top_srcdir)/adie/nologger.gif \
$(top_srcdir)/adie/open_gif.gif \
$(top_srcdir)/adie/palette_gif.gif \
$(top_srcdir)/adie/pattern_gif.gif \
$(top_srcdir)/adie/paste_gif.gif \
$(top_srcdir)/adie/print_gif.gif \
$(top_srcdir)/adie/quit_gif.gif \
$(top_srcdir)/adie/redo_gif.gif \
$(top_srcdir)/adie/reload_gif.gif \
$(top_srcdir)/adie/replace_gif.gif \
$(top_srcdir)/adie/saveall_gif.gif \
$(top_srcdir)/adie/saveas_gif.gif \
$(top_srcdir)/adie/saveto_gif.gif \
$(top_srcdir)/adie/save_gif.gif \
$(top_srcdir)/adie/search_gif.gif \
$(top_srcdir)/adie/searchicon_gif.gif \
$(top_srcdir)/adie/searchnext_gif.gif \
$(top_srcdir)/adie/searchprev_gif.gif \
$(top_srcdir)/adie/shiftleft_gif.gif \
$(top_srcdir)/adie/shiftright_gif.gif \
$(top_srcdir)/adie/small_gif.gif \
$(top_srcdir)/adie/styles_gif.gif \
$(top_srcdir)/adie/syntax_gif.gif \
$(top_srcdir)/adie/undo_gif.gif \
$(top_srcdir)/adie/uppercase.gif


adie_SOURCES = \
Adie.h \
Adie.cpp \
Commands.cpp \
Commands.h \
FindInFiles.h \
FindInFiles.cpp \
help.h \
help.cpp \
HelpWindow.h \
HelpWindow.cpp \
icons.h \
icons.cpp \
main.cpp \
Modeline.h \
Modeline.cpp \
Preferences.h \
Preferences.cpp \
ShellCommand.h \
ShellCommand.cpp \
ShellDialog.h \
ShellDialog.cpp \
Syntax.h \
Syntax.cpp \
SyntaxParser.h \
SyntaxParser.cpp \
TextWindow.h \
TextWindow.cpp

BUILT_SOURCES = \
icons.h \
icons.cpp \
help.h \
help.cpp

icons.h: $(ICONS)
	$(RESWRAP) -h -o $@ $^


icons.cpp: $(ICONS)
	$(RESWRAP) -i icons.h -o $@ $^

help.h: help.txt
	$(RESWRAP) -h -a -o $@ $^

help.cpp: help.txt
	$(RESWRAP) -i help.h -a -o $@ $^

CLEANFILES = icons.h icons.cpp help.h help.cpp


EXTRA_DIST = $(man_MANS) \
Adie.stx \
LICENSE \
help.txt \
$(ICONS)
