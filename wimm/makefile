#############################################################################
##                         MsgAPI "generic makefile"                       ##
#############################################################################

# If the LIB-making stuff doesn't work with NDMAKE and TLIB, take your
# MAKE.INI and edit out the ".RESPONSE_LIB:" line.

#
# Turbo C, TC++
#

MODEL=l
PREFIX=
COMP=wcc386.exe
LIBIT=wlib.exe
#FLAGS=-mf -d2 -od -wx -fh -zp1 -ic:\tc\api;c:\timed\utils
FLAGS=-mf -4r -olrt -s -wx -fh -zp1 -ic:\tc\api;c:\timed\utils

wimm.exe: wimm.obj xfile.obj xmalloc.obj lastread.obj readcf2.obj scrnutil.obj video.obj

wimm.obj: wimm.c
   $(COMP) $(FLAGS) wimm.c

xfile.obj: xfile.c
   $(COMP) $(FLAGS) xfile.c

readcf2.obj: readcf2.c
   $(COMP) $(FLAGS) readcf2.c

lastread.obj: lastread.c
   $(COMP) $(FLAGS) lastread.c

xmalloc.obj: xmalloc.c
   $(COMP) $(FLAGS) xmalloc.c

scrnutil.obj: scrnutil.c
   $(COMP) $(FLAGS) scrnutil.c

video.obj: video.c
   $(COMP) $(FLAGS) video.c


