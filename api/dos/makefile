# MSGAPI makefile for 16-bit DOS build using Watcom

TARGET=..\dosapi.lib

CC=wcl
CFLAGS=-zq -ml -d2 -wx -zp1 -bt=dos
#COPT=-oneatx
ASM=wasm
AFLAGS=-zq
LIB=wlib
LFLAGS=-q
SRC_DIR=..

OBJS=1stchar.obj msgapi.obj api_sdm.obj api_sq.obj cvtdate.obj &
  date2bin.obj dosdate.obj fexist.obj months.obj parsenn.obj &
  qksort.obj strftim.obj stristr.obj trail.obj weekday.obj flush.obj &
  canlock.obj flusha.obj api_jam.obj jamcrc32.obj api_hmb.obj &
  common.obj sqsdm.obj strlist.obj ffind.obj ffinda.obj

{$(SRC_DIR)}.c.obj:
	$(CC) $(CFLAGS) $(CDEFS) $(COPT) -c $<
	$(LIB) $(LFLAGS) $(TARGET) -+$@

{$(SRC_DIR)}.asm.obj:
    $(ASM) $(AFLAGS) $[*
	$(LIB) $(LFLAGS) $(TARGET) -+$@

$(TARGET): $(OBJS)

clean:  
	del *.obj
	del $(TARGET)
