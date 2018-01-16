#############################################################################
##                         MsgAPI "generic makefile"                       ##
#############################################################################

# If the LIB-making stuff doesn't work with NDMAKE and TLIB, take your
# MAKE.INI and edit out the ".RESPONSE_LIB:" line.

#
# Turbo C, TC++
#

#FLAGS=-ml -od -d2 -wo -wx -fh -zp1 -bt=DOS -id:\tc\msgapi;d:\tc\exec

FLAGS=-zq -ml -ol+rsf -wo -s -wx -zp1 -bt=DOS -ix:\api;x:\exec

.c : ..\
.c : x:\exec\

.asm : x:\exec\

.c.obj : .AUTODEPEND
   wcc $(FLAGS) $[*

.asm.obj :
    c:\bc\bin\tasm /zi/t/mx $[*


..\netmgr.exe: netmgr.obj register.obj config.obj mem.obj wrap.obj &
   txtbuild.obj version7.obj fdnode.obj gigo.obj binkpack.obj command.obj &
   akamatch.obj lastread.obj xfile.obj external.obj exec.obj checkpat.obj &
   spawn.obj
   wlink @ntmgr

