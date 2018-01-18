# timEd makefile for 16-bit DOS build using Watcom

TARGET=..\timed.exe

CC=wcl
#CFLAGS=-ml -ol+rsf -s -wx -fh -bt=DOS -zp1 -id:\tc\msgapi;d:\timed\utils;d:\tc\exec;d:\database\cbt
CFLAGS=-zq -ml -d2 -odf -wo -wx -bt=DOS -zp1 -k8k -I..\..\api -I..\utils -I..\..\exec
#COPT=-oneatx
CDEFS=-D__DOS__=1
ASM=c:\bc\bin\tasm.exe
AFLAGS=/zi/t/mx
SRC_DIR=..
MSGAPI=..\..\api\dosapi.lib

OBJS=areatags.obj attach.obj bmg.obj checkcc.obj choose.obj &
 cleanidx.obj clock.obj config.obj copymsg.obj dirlist.obj edit.obj &
 errorrpt.obj find.obj getbody.obj getfmt.obj nodelist.obj &
 getmsg.obj globals.obj header.obj help.obj idlekey.obj input.obj &
 list.obj lock.obj maint.obj marklist.obj mem.obj message.obj &
 nodeglue.obj pickone.obj print.obj tasker.obj exec.obj checkpat.obj spawn.obj &
 quotos2.obj readarea.obj repedit.obj reply.obj request.obj &
 select.obj shel2dos.obj showhdr.obj showinfo.obj showmail.obj &
 timed.obj tosslog.obj txtbuild.obj unrec.obj printer.obj &
 working.obj wrap.obj xfile.obj video.obj scrnutil.obj picklist.obj &
 markchn.obj trapflag.obj macro.obj charset.obj filter.obj &
 v7.obj fd.obj nodelist.obj nodebrow.obj fido.obj &
 private.obj
  
{$(SRC_DIR)}.c.obj:
	$(CC) $(CFLAGS) $(CDEFS) $(COPT) -c $<

{$(SRC_DIR)}.asm.obj:
	$(ASM) $(AFLAGS) -c $<

{$(SRC_DIR)\utils}.c.obj:
	$(CC) $(CFLAGS) $(CDEFS) $(COPT) -c $<

{$(SRC_DIR)\..\exec}.c.obj:
	$(CC) $(CFLAGS) $(CDEFS) $(COPT) -c $<

{$(SRC_DIR)\..\exec}.asm.obj:
	$(ASM) $(AFLAGS) -c $<

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(CDEFS) $(COPT) -fe=$@ $(OBJS) $(MSGAPI)

clean:  
	del *.obj
	del $(TARGET)
