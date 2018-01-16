#############################################################################
##                         MsgAPI "generic makefile"                       ##
#############################################################################

# If the LIB-making stuff doesn't work with NDMAKE and TLIB, take your
# MAKE.INI and edit out the ".RESPONSE_LIB:" line.

#
# Turbo C, TC++
#

#FLAGS=-mf -bm -sg -od -d2 -wx -fh -bt=OS2 -zp1 -id:\tc\msgapi

FLAGS=-zq -5r -mf -sg -bm -ol+rs -s -wx -zp1 -bt=OS2 -ix:\api

.c : ..\

.c.obj : .AUTODEPEND
   wcc386 $(FLAGS) $[*

..\netmgrp.exe: netmgr.obj register.obj config.obj wrap.obj &
   txtbuild.obj version7.obj fdnode.obj gigo.obj binkpack.obj command.obj &
   xfile.obj lastread.obj akamatch.obj external.obj mem.obj
   wlink @ntmgrp

