# MSGAPI makefile for UNIX build

TARGET=../unixapi.a
CFLAGS=-g -Wall
LIB=ar
RANLIB=ranlib
SRC_DIR=..
VPATH=$(SRC_DIR)

.c.o:
	$(CC) $(CFLAGS) -o $*.o -c $<

OBJS=1stchar.o msgapi.o cvtdate.o date2bin.o dosdate.o fexist.o flush.o \
  months.o parsenn.o qksort.o strftim.o stristr.o trail.o weekday.o \
  canlock.o api_jam.o api_hmb.o jamcrc32.o strlist.o api_sq.o api_sdm.o \
  common.o sqsdm.o unix.o

$(TARGET): $(OBJS)
	$(AR) r $@ $?
	$(RANLIB) $@

clean:
	rm -f $(OBJS) $(TARGET)
