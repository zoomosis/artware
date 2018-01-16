#############################################################################
##                         MsgAPI "generic makefile"                       ##
#############################################################################

# If the LIB-making stuff doesn't work with NDMAKE and TLIB, take your
# MAKE.INI and edit out the ".RESPONSE_LIB:" line.

#
# Turbo C, TC++
#

LIBIT=wlib.exe
LIBNAME=x:\api\ntapi.lib

#FLAGS=-5r -s -sg -mf -ol+rs -wx -bm -zp1 -bt=NT

FLAGS=-zq -d2 -mf -od -wx -bm -zp1 -bt=NT

.c : ..\

.c.obj : .AUTODEPEND
   wcc386 $(FLAGS) $[*


$(LIBNAME): 1stchar.obj msgapi.obj cvtdate.obj date2bin.obj dosdate.obj &
 fexist.obj flush.obj months.obj parsenn.obj qksort.obj strftim.obj &
 stristr.obj trail.obj weekday.obj canlock.obj api_jam.obj api_hmb.obj &
 jamcrc32.obj strlist.obj api_sq.obj api_sdm.obj common.obj sqsdm.obj

 $(LIBIT) $(LIBNAME) -+1stchar -+msgapi -+api_jam -+cvtdate -+date2bin -+dosdate -+fexist
 $(LIBIT) $(LIBNAME) -+flush -+months -+parsenn -+qksort -+strftim -+stristr -+trail -+weekday -+canlock
 $(LIBIT) $(LIBNAME) -+jamcrc32 -+strlist -+api_sq -+api_sdm -+common -+sqsdm -+api_hmb

