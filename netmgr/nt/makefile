# NetMgr makefile for 32-bit Windows NT build using Watcom

TARGET=..\netmgrnt.exe

CC=wcl386
CFLAGS=-zq -bm -sg -d2 -wx -bt=nt -l=nt -k32k -I..\..\api
#COPT=-oneatx
SRC_DIR=..
MSGAPI=..\..\api\ntapi.lib

OBJS=netmgr.obj config.obj wrap.obj txtbuild.obj &
	version7.obj fdnode.obj gigo.obj binkpack.obj command.obj &
	xfile.obj lastread.obj akamatch.obj external.obj mem.obj

{$(SRC_DIR)}.c.obj:
	$(CC) $(CFLAGS) $(CDEFS) $(COPT) -c $<

$(TARGET): $(OBJS) $(MSGAPI)
	$(CC) $(CFLAGS) $(CDEFS) $(COPT) -fe=$@ $(OBJS) $(MSGAPI)

clean:  
	del *.obj
	del $(TARGET)
