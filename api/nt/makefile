# MSGAPI makefile for 32-bit Windows NT build using OpenWatcom

TARGET=..\ntapi.lib

CC=wcl386
CFLAGS=-zq -d2 -wx -zp1 -sg -bm -bt=nt
#COPT=-oneatx
LIB=wlib
LFLAGS=-q
SRC_DIR=..

!ifdef __LINUX__
DEL=rm -f
!else
DEL=del
!endif

OBJS=1stchar.obj msgapi.obj cvtdate.obj date2bin.obj dosdate.obj &
  fexist.obj flush.obj months.obj parsenn.obj qksort.obj strftim.obj &
  stristr.obj trail.obj weekday.obj canlock.obj api_jam.obj api_hmb.obj &
  jamcrc32.obj strlist.obj api_sq.obj api_sdm.obj common.obj sqsdm.obj

{$(SRC_DIR)}.c.obj:
	$(CC) $(CFLAGS) $(CDEFS) $(COPT) -c -fo=$@ $<
	$(LIB) $(LFLAGS) $(TARGET) -+$@

$(TARGET): $(OBJS)

.IGNORE
clean:
	$(DEL) *.obj
	$(DEL) $(TARGET)
