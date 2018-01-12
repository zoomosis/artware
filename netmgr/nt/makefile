#############################################################################
##                         MsgAPI "generic makefile"                       ##
#############################################################################

# If the LIB-making stuff doesn't work with NDMAKE and TLIB, take your
# MAKE.INI and edit out the ".RESPONSE_LIB:" line.

#
# Turbo C, TC++
#


#FLAGS=-5r -mf -ol+rs -s -wx -fh -zp1 -bt=NT -ic:\tc\msgapi

FLAGS=-mf -od -d2 -wx -fh -bt=NT -zp1 -ic:\tc\msgapi

.c : ..\

.c.obj : .AUTODEPEND
   wcc386 $(FLAGS) $[*

..\netmgrnt.exe: netmgr.obj register.obj config.obj wrap.obj &
   txtbuild.obj version7.obj fdnode.obj gigo.obj binkpack.obj command.obj &
   xfile.obj lastread.obj akamatch.obj external.obj mem.obj
   wlink @netnt

