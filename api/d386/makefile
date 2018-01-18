# MSGAPI makefile for 32-bit DOS4GW build using Watcom & Borland TASM 

TARGET=..\386api.lib

CC=wcl386
#CFLAGS=-d2 -mf -od -wx -zp1 -bt=DOS4G
CFLAGS=-zq -5r -s -mf -ol+rs -wx -bm -zp1 -bt=DOS4G -D__DOS4GW__=1
#COPT=-oneatx
ASM=c:\bc\bin\tasm.exe
AFLAGS=/t/mx /d__FLAT__ 
LIB=wlib
LFLAGS=-q
SRC_DIR=..

OBJS=1stchar.obj msgapi.obj cvtdate.obj date2bin.obj dosdate.obj &
  fexist.obj flush.obj months.obj parsenn.obj qksort.obj strftim.obj &
  stristr.obj trail.obj weekday.obj canlock.obj api_jam.obj api_hmb.obj &
  jamcrc32.obj strlist.obj api_sq.obj api_sdm.obj common.obj sqsdm.obj &
  ffind.obj ffinda.obj

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
