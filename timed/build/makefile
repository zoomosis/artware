CC=wcl386
CFLAGS=-zq -wx -zp1 -I\api -I\timed\utils
#COPT=-oneatx -5
SRC_DIR=..

!ifndef NDEBUG
CFLAGS+= -d2 -hw
!endif

# default to OS/2 build

!ifndef OS
OS=OS2
!endif

# NT is a synonym for WIN

!ifeq OS NT
OS=WIN
!endif

!ifeq OS WIN
TARGET=timednt.exe
CC=wcl386
CFLAGS+= -bt=nt -l=nt
CDEFS=-DWINNT
MSGAPI=..\..\api\ntapi.lib
!endif

!ifeq OS OS2
TARGET=timedp.exe
CC=wcl386
CFLAGS+= -bt=os2 -l=os2v2
CDEFS=-DOS2
MSGAPI=..\..\api\os2api.lib
!endif

!ifeq OS 386
TARGET=tim386.exe
CC=wcl386
CFLAGS+= -bt=dos -l=dos4g
#CDEFS=-D__DOS__
MSGAPI=..\..\api\386api.lib
!endif

CFLAGS+= $(CDEFS) $(COPT)

OBJS=areatags.obj &
	attach.obj &
	bmg.obj &
	checkcc.obj &
	choose.obj &
	select.obj &
	shel2dos.obj &
	showhdr.obj &
	showinfo.obj &
	showmail.obj &
	getmsg.obj &
	globals.obj &
	header.obj &
	help.obj &
	idlekey.obj &
	input.obj &
	list.obj &
	lock.obj &
	maint.obj &
	marklist.obj &
	mem.obj &
	message.obj &
	cleanidx.obj &
	clock.obj &
	config.obj &
	copymsg.obj &
	dirlist.obj &
	edit.obj &
	pickone.obj &
	register.obj &
	print.obj &
	printer.obj &
	quotos2.obj &
	readarea.obj &
	repedit.obj &
	reply.obj &
	request.obj &
	errorrpt.obj &
	fido.obj &
	find.obj &
	getbody.obj &
	getfmt.obj &
	timed.obj &
	tosslog.obj &
	txtbuild.obj &
	unrec.obj &
	nodeglue.obj &
	working.obj &
	wrap.obj &
	xfile.obj &
	picklist.obj &
	markchn.obj &
	macro.obj &
	charset.obj &
	filter.obj &
	v7.obj &
	fd.obj &
	nodelist.obj &
	nodebrow.obj &
	private.obj

UTILOBJS=video.obj &
	scrnutil.obj &

!ifeq OS 386
OBJS+= task386.obj
!endif
	
all: $(TARGET)

{$(SRC_DIR)\utils}.c.obj: ($UTILOBJS)
	$(CC) $(CFLAGS) -c $<

{$(SRC_DIR)}.c.obj: $(OBJS)
	$(CC) $(CFLAGS) -c $<

.c.obj:
	$(CC) $(CFLAGS) -c $<

$(TARGET): $(OBJS) $(UTILOBJS)
	$(CC) $(CFLAGS) -fe=$(TARGET) $(OBJS) $(UTILOBJS) $(MSGAPI)

clean:
	del *.obj
	del *.err

distclean: clean
	del $(TARGET)

install: $(TARGET)
	copy $(TARGET) $(BINDIR)
