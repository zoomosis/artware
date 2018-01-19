# NetMgr makefile for 32-bit DOS build using Watcom

TARGET=..\net386.exe

CC=wcl386
CFLAGS=-zq -sg -d2 -wx -bt=dos4g -l=dos4g -k32k -I..\..\api
#COPT=-oneatx
SRC_DIR=..
MSGAPI=..\..\api\386api.lib

OBJS=netmgr.obj register.obj config.obj wrap.obj txtbuild.obj &
	version7.obj fdnode.obj gigo.obj binkpack.obj command.obj &
	xfile.obj lastread.obj akamatch.obj external.obj mem.obj

{$(SRC_DIR)}.c.obj:
	$(CC) $(CFLAGS) $(CDEFS) $(COPT) -c $<

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(CDEFS) $(COPT) -fe=$@ $(OBJS) $(MSGAPI)

clean:  
	del *.obj
	del $(TARGET)
