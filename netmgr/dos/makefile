# NetMgr makefile for 16-bit DOS build using Watcom

TARGET=..\netmgr.exe

CC=wcl
CFLAGS=-zq -ml -sg -d2 -wx -bt=dos -l=dos -k8k -I..\..\api -I..\..\exec
#COPT=-oneatx
SRC_DIR=..
MSGAPI=..\..\api\dosapi.lib

OBJS=netmgr.obj register.obj config.obj mem.obj wrap.obj &
	txtbuild.obj version7.obj fdnode.obj gigo.obj binkpack.obj command.obj &
	akamatch.obj lastread.obj xfile.obj external.obj exec.obj checkpat.obj &
	spawn.obj

{$(SRC_DIR)}.c.obj:
	$(CC) $(CFLAGS) $(CDEFS) $(COPT) -c $<

{\exec}.c.obj:
	$(CC) $(CFLAGS) $(CDEFS) $(COPT) -c $<

{\exec}.asm.obj:
    c:\bc\bin\tasm /zi/t/mx $[*

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(CDEFS) $(COPT) -fe=$@ $(OBJS) $(MSGAPI)

clean:  
	del *.obj
	del $(TARGET)
