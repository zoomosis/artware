# MSGAPI makefile for 32-bit OS/2 build using OpenWatcom

!ifdef __LINUX__
TARGET=../os2api.lib
!else
TARGET=..\os2api.lib
!endif

CC=wcl386
CFLAGS=-zq -d2 -wx -zp1 -sg -bt=os2
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
