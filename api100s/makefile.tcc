#############################################################################
##                         MsgAPI "generic makefile"                       ##
#############################################################################

# If the LIB-making stuff doesn't work with NDMAKE and TLIB, take your
# MAKE.INI and edit out the ".RESPONSE_LIB:" line.

#
# Turbo C, TC++
#

MODEL=l
PREFIX=tc
COMP=d:\tcpp\bin\tcc
LIBIT=tlib
FLAGS=-c -O -Z -d -k- -f- -v- -p- -u  -C -m$(MODEL) -Id:\tcpp\include -Ld:\tcpp\lib
FLAGSOVL=-c -Z -O -C -d -k- -f- -v- -p- -u -m$(MODEL) -Id:\tcpp\include -Ld:\tcpp\lib
LIB_TRAIL=

ASM=tasm /t/mx

LIBNAME=$(PREFIX)_mapi$(MODEL).lib

$(LIBNAME):     1stchar.obj     msgapi.obj      api_sdm.obj     api_sq.obj   \
                sqasm.obj       cvtdate.obj     date2bin.obj    dosdate.obj  \
                fexist.obj      ffind.obj       flush.obj       sqasm.obj \
                months.obj      parsenn.obj     qksort.obj      strftim.obj  \
                stristr.obj     strocpy.obj     trail.obj       weekday.obj  \
                canlock.obj     flusha.obj      \
                api_jam.obj     jamcrc32.obj    api_hmb.obj

        $(LIBIT) $(LIBNAME),
        $(LIBIT) $(LIBNAME) -+1stchar -+msgapi -+api_sdm -+api_sq -+api_jam -+sqasm -+cvtdate -+date2bin -+dosdate -+fexist -+ffind $(LIB_TRAIL)
        $(LIBIT) $(LIBNAME) -+flush -+flusha -+months -+parsenn -+qksort -+strftim -+stristr -+strocpy -+trail -+weekday -+canlock $(LIB_TRAIL)
        $(LIBIT) $(LIBNAME) -+jamcrc32 -+api_hmb

1stchar.obj: 1stchar.c
        $(COMP) $(FLAGS) 1stchar.c

msgapi.obj: msgapi.c
        $(COMP) $(FLAGS) msgapi.c

api_sdm.obj: api_sdm.c
        $(COMP) $(FLAGSOVL) api_sdm.c

api_sq.obj: api_sq.c
        $(COMP) $(FLAGSOVL) api_sq.c

api_jam.obj: api_jam.c
        $(COMP) $(FLAGSOVL) api_jam.c

api_hmb.obj: api_hmb.c
        $(COMP) $(FLAGSOVL) api_hmb.c

jamcrc32.obj: jamcrc32.c
        $(COMP) $(FLAGS) jamcrc32.c

sqasm.obj: sqasm.asm
        $(ASM) sqasm.asm

cvtdate.obj: cvtdate.c
        $(COMP) $(FLAGS) cvtdate.c

date2bin.obj: date2bin.c
        $(COMP) $(FLAGS) date2bin.c

dosdate.obj: dosdate.c
        $(COMP) $(FLAGS) dosdate.c

fexist.obj: fexist.c
        $(COMP) $(FLAGS) fexist.c

ffind.obj: ffind.c
        $(COMP) $(FLAGS) ffind.c

flush.obj: flush.c
        $(COMP) $(FLAGS) flush.c

flusha.obj: flusha.asm
        $(ASM) flusha.asm

months.obj: months.c
        $(COMP) $(FLAGS) months.c

parsenn.obj: parsenn.c
        $(COMP) $(FLAGS) parsenn.c

qksort.obj: qksort.c
        $(COMP) $(FLAGS) qksort.c

strftim.obj: strftim.c
        $(COMP) $(FLAGS) strftim.c

stristr.obj: stristr.c
        $(COMP) $(FLAGS) stristr.c

strocpy.obj: strocpy.c
        $(COMP) $(FLAGS) strocpy.c

trail.obj: trail.c
        $(COMP) $(FLAGS) trail.c

weekday.obj: weekday.c
        $(COMP) $(FLAGS) weekday.c

canlock.obj: canlock.c
        $(COMP) $(FLAGSOVL) canlock.c

