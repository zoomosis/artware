# timEd makefile for 32-bit Windows NT build using Watcom

!ifdef __LINUX__

TARGET=../timednt.exe
CFLAGS=-zq -bm -sg -wx -bt=nt -l=nt -k32k -I../../api -I..
MSGAPI=../../api/ntapi.lib
DEL=rm -f

!else

TARGET=..\timednt.exe
CFLAGS=-zq -bm -sg -wx -bt=nt -l=nt -k32k -I..\..\api -I..
MSGAPI=..\..\api\ntapi.lib
DEL=del

!endif

CC=wcl386
#COPT=-oneatx
SRC_DIR=..

OBJS=areatags.obj attach.obj bmg.obj checkcc.obj choose.obj &
  select.obj shel2dos.obj showhdr.obj showinfo.obj showmail.obj &
  getmsg.obj globals.obj header.obj help.obj idlekey.obj input.obj &
  list.obj lock.obj maint.obj marklist.obj mem.obj message.obj &
  cleanidx.obj clock.obj config.obj copymsg.obj dirlist.obj edit.obj &
  pickone.obj print.obj printer.obj quotos2.obj readarea.obj &
  repedit.obj reply.obj request.obj errorrpt.obj fido.obj find.obj &
  getbody.obj getfmt.obj timed.obj tosslog.obj txtbuild.obj unrec.obj &
  nodeglue.obj working.obj wrap.obj video.obj scrnutil.obj picklist.obj &
  markchn.obj macro.obj charset.obj filter.obj v7.obj fd.obj nodelist.obj &
  nodebrow.obj private.obj fidonode.obj fidouser.obj patmat.obj

{$(SRC_DIR)}.c.obj:
	$(CC) $(CFLAGS) $(CDEFS) $(COPT) -fo=$@ -c $<

$(TARGET): $(OBJS) $(MSGAPI)
	$(CC) $(CFLAGS) $(CDEFS) $(COPT) -fe=$@ $(OBJS) $(MSGAPI)

.IGNORE
clean:
	$(DEL) *.obj
	$(DEL) $(TARGET)
