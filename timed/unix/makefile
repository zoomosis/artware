# timEd makefile for UNIX build

TARGET=../timedunx
CFLAGS=-Wfatal-errors -m32 -fstack-protector-all -W -Wall -g -I../../api -I../utils -funsigned-char
SRC_DIR=..
VPATH=$(SRC_DIR)

OBJS=areatags.o attach.o bmg.o checkcc.o choose.o select.o shel2dos.o \
  showhdr.o showinfo.o showmail.o getmsg.o globals.o header.o help.o \
  idlekeyunix.o input.o list.o lock.o maint.o marklist.o memunix.o message.o \
  cleanidx.o clock.o config.o copymsg.o dirlist.o edit.o pickone.o \
  print.o printer.o quotos2.o readarea.o repedit.o reply.o request.o \
  errorrpt.o fido.o find.o getbody.o getfmt.o timed.o tosslog.o \
  txtbuild.o unrec.o nodeglue.o working.o wrap.o videounix.o \
  scrnutilunix.o picklist.o markchn.o macro.o charset.o filter.o v7.o fd.o \
  nodelist.o nodebrow.o private.o fidonode.o fidouser.o patmat.o

.c.o:
	$(CC) $(CFLAGS) -o $*.o -c $<

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) ../../api/unixapi.a -lncurses

clean:
	rm -f $(OBJS) $(TARGET)
