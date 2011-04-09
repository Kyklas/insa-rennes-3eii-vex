
# This is the makefile for the source

# additional include paths necessary to compile the C++ programs
# AM_CFLAGS = -I$(top_srcdir)/src @VEXDEP_CFLAGS@
CFLAGS = @VEXDEP_CFLAGS@

###############################################################################
# THE PROGRAMS TO BUILD
###############################################################################

# the program to build (the names of the final binaries)
bin_PROGRAMS = vex

# list of sources for the 'vex' binary
vex_SOURCES =    \
			main.c \
			appdata.h \
			communication.h \
			ui/callbacks.h \
			ui/callbacks.c \
			ui/interface.h \
			ui/interface.c \
			ui/uithread.h \
			ui/uithread.c \
			bt/bluetooth.h \
			bt/bluetooth.c \
			bt/btthread.h \
			bt/btthread.c

vex_LDFLAGS = @VEXDEP_LIBS@

###############################################################################
