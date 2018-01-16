#############################################################################
##                         MsgAPI "generic makefile"                       ##
#############################################################################

# If the LIB-making stuff doesn't work with NDMAKE and TLIB, take your
# MAKE.INI and edit out the ".RESPONSE_LIB:" line.

#
# Turbo C, TC++
#

FLAGS=-zq -mf -od -d2 -wx -bt=DOS4G -zp1 -ix:\api

.c : ..\

.c.obj : .AUTODEPEND
   wcc386 $(FLAGS) $[*

..\net386.exe: netmgr.obj register.obj config.obj wrap.obj &
   txtbuild.obj version7.obj fdnode.obj gigo.obj binkpack.obj command.obj &
   xfile.obj lastread.obj akamatch.obj external.obj mem.obj
   wlink @net386

