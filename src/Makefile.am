
# This is the makefile for the source

# additional include paths necessary to compile the C++ programs
AM_CXXFLAGS = -I$(top_srcdir)/src @VEXDEP_CFLAGS@

###############################################################################
# THE PROGRAMS TO BUILD
###############################################################################

# the program to build (the names of the final binaries)
bin_PROGRAMS = vex

# list of sources for the 'vex' binary
vex_SOURCES =    \
			main.cpp \
			appdata.h \
			ui/callbacks.h \
			ui/callbacks.cpp \
			ui/interface.h \
			ui/interface.cpp \
			ui/uithread.h \
			ui/uithread.cpp \
			bt/bluetooth.h \
			bt/bluetooth.cpp \
			bt/btthread.h \
			bt/btthread.cpp

vex_LDFLAGS = @VEXDEP_LIBS@

###############################################################################
