# timEd makefile for 16-bit DOS build using Watcom

!ifdef __LINUX__

TARGET=../timed.exe
CFLAGS=-zq -zc -ml -wx -bt=dos -l=dos -k8k -I../../api -I.. -I../../exec
MSGAPI=../../api/dosapi.lib
DEL=rm

!else

TARGET=..\timed.exe
CFLAGS=-zq -zc -ml -wx -bt=dos -l=dos -k8k -I..\..\api -I.. -I..\..\exec
MSGAPI=..\..\api\dosapi.lib
ASM=c:\bc\bin\tasm.exe
AFLAGS=/zi/t/mx
DEL=del

!endif

CC=wcl
#COPT=-oneatx
CDEFS=-D__DOS__=1
SRC_DIR=..

OBJS=areatags.obj attach.obj bmg.obj checkcc.obj choose.obj &
  cleanidx.obj clock.obj config.obj copymsg.obj dirlist.obj edit.obj &
  errorrpt.obj find.obj getbody.obj getfmt.obj nodelist.obj &
  getmsg.obj globals.obj header.obj help.obj idlekey.obj input.obj &
  list.obj lock.obj maint.obj marklist.obj mem.obj message.obj &
  nodeglue.obj pickone.obj print.obj tasker.obj exec.obj checkpat.obj spawn.obj &
  quotos2.obj readarea.obj repedit.obj reply.obj request.obj &
  select.obj shel2dos.obj showhdr.obj showinfo.obj showmail.obj &
  timed.obj tosslog.obj txtbuild.obj unrec.obj printer.obj &
  working.obj wrap.obj video.obj scrnutil.obj picklist.obj &
  markchn.obj trapflag.obj macro.obj charset.obj filter.obj &
  v7.obj fd.obj nodelist.obj nodebrow.obj fido.obj private.obj &
  fidonode.obj fidouser.obj patmat.obj
  
{$(SRC_DIR)}.asm.obj:
!ifdef __LINUX__
        echo "Error: Building from within Linux is not fully supported; Borland TASM (tasm.exe) is required."
!else
	$(ASM) $(AFLAGS) -c $<
!endif

{$(SRC_DIR)}.c.obj:
	$(CC) $(CFLAGS) $(CDEFS) $(COPT) -fo=$@ -c $<

{$(SRC_DIR)\utils}.c.obj:
	$(CC) $(CFLAGS) $(CDEFS) $(COPT) -fo=$@ -c $<

{$(SRC_DIR)\..\exec}.c.obj:
	$(CC) $(CFLAGS) $(CDEFS) $(COPT) -fo=$@ -c $<

{$(SRC_DIR)\..\exec}.asm.obj:
	$(ASM) $(AFLAGS) -fo=$@ -c $<

$(TARGET): $(OBJS) $(MSGAPI)
	$(CC) $(CFLAGS) $(CDEFS) $(COPT) -fe=$@ $(OBJS) $(MSGAPI)

.IGNORE
clean:
	$(DEL) *.obj
	$(DEL) $(TARGET)
