#############################################################################
##                         MsgAPI "generic makefile"                       ##
#############################################################################

# If the LIB-making stuff doesn't work with NDMAKE and TLIB, take your
# MAKE.INI and edit out the ".RESPONSE_LIB:" line.

#
# Turbo C, TC++
#

#LIBIT=glib
#LIBNAME=linapi.lib

#FLAGS=-d2 -mf -od -wx -bm -fh -zp1 -bt=OS2

#FLAGS=-g -Wall

.c.o:
	gcc -g -Wall -fpack-struct -o $*.o -c $<

OBJS = 1stchar.o msgapi.o cvtdate.o date2bin.o dosdate.o \
	fexist.o flush.o months.o parsenn.o qksort.o strftim.o \
	stristr.o trail.o weekday.o canlock.o api_jam.o api_hmb.o \
	jamcrc32.o strlist.o api_sq.o api_sdm.o common.o sqsdm.o \
	lincomp.o

linapi.a : 	${OBJS}
	ar r $@ $?
	ranlib $@

all: linapi.a
