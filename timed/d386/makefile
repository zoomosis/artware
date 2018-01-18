# timEd makefile for 32-bit DOS build using Watcom

TARGET=..\tim386.exe

CC=wcl386
#CFLAGS=-mf -od -d2 -wx -fh -bt=DOS4G -zp1 -id:\tc\api;d:\timed\utils;d:\database\cbt
CFLAGS=-zq -5r -mf -ol+rs -s -wx -zp1 -bt=dos4g -l=dos4g -I..\..\api -I..\utils
#COPT=-oneatx
CDEFS=-D__DOS__=1
SRC_DIR=..
MSGAPI=..\..\api\386api.lib

OBJS=areatags.obj attach.obj bmg.obj checkcc.obj choose.obj &
  select.obj shel2dos.obj showhdr.obj showinfo.obj showmail.obj &
  getmsg.obj globals.obj header.obj help.obj key386.obj input.obj &
  list.obj lock.obj maint.obj marklist.obj mem.obj message.obj &
  cleanidx.obj clock.obj config.obj copymsg.obj dirlist.obj edit.obj &
  pickone.obj register.obj print.obj printer.obj &
  quotos2.obj readarea.obj repedit.obj reply.obj request.obj &
  errorrpt.obj fido.obj find.obj getbody.obj getfmt.obj &
  timed.obj tosslog.obj txtbuild.obj unrec.obj nodeglue.obj &
  working.obj wrap.obj xfile.obj video.obj scrnutil.obj picklist.obj &
  markchn.obj macro.obj charset.obj filter.obj v7.obj fd.obj &
  nodelist.obj nodebrow.obj task386.obj private.obj
  
{$(SRC_DIR)}.c.obj:
	$(CC) $(CFLAGS) $(CDEFS) $(COPT) -c $<

{$(SRC_DIR)\utils}.c.obj:
	$(CC) $(CFLAGS) $(CDEFS) $(COPT) -c $<

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(CDEFS) $(COPT) -fe=$@ $(OBJS) $(MSGAPI)

clean:  
	del *.obj
	del $(TARGET)