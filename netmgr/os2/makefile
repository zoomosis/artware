# NetMgr makefile for 32-bit OS/2 build using Watcom

TARGET=..\netmgrp.exe

CC=wcl386
CFLAGS=-zq -bm -sg -d2 -wx -bt=os2 -l=os2v2 -k32k -I..\..\api
#COPT=-oneatx
SRC_DIR=..
MSGAPI=..\..\api\os2api.lib

OBJS=netmgr.obj config.obj wrap.obj txtbuild.obj &
	version7.obj fdnode.obj gigo.obj binkpack.obj command.obj &
	xfile.obj lastread.obj akamatch.obj external.obj mem.obj

{$(SRC_DIR)}.c.obj:
	$(CC) $(CFLAGS) $(CDEFS) $(COPT) -c $<

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(CDEFS) $(COPT) -fe=$@ $(OBJS) $(MSGAPI)

clean:  
	del *.obj
	del $(TARGET)
