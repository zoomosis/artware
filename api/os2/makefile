# MSGAPI makefile for 32-bit OS/2 build using Watcom

TARGET=..\os2api.lib

CC=wcl386
CFLAGS=-zq -d2 -wx -zp1 -sg -bt=os2
#COPT=-oneatx
LIB=wlib
LFLAGS=-q
SRC_DIR=..

OBJS=1stchar.obj msgapi.obj cvtdate.obj date2bin.obj dosdate.obj &
  fexist.obj flush.obj months.obj parsenn.obj qksort.obj strftim.obj &
  stristr.obj trail.obj weekday.obj canlock.obj api_jam.obj api_hmb.obj &
  jamcrc32.obj strlist.obj api_sq.obj api_sdm.obj common.obj sqsdm.obj

{$(SRC_DIR)}.c.obj:
	$(CC) $(CFLAGS) $(CDEFS) $(COPT) -c $<
	$(LIB) $(LFLAGS) $(TARGET) -+$@

$(TARGET): $(OBJS)

clean:  
	del *.obj
	del $(TARGET)
