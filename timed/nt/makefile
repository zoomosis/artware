#############################################################################
##                         MsgAPI "generic makefile"                       ##
#############################################################################

# If the LIB-making stuff doesn't work with NDMAKE and TLIB, take your
# MAKE.INI and edit out the ".RESPONSE_LIB:" line.

#
# Turbo C, TC++
#

#FLAGS=-5r -mf -s -ol+rs -wx -fh -zp1 -bt=NT -ic:\tc\msgapi;c:\timed\utils;c:\database\cbt

FLAGS=-zq -mf -od -d2 -wx -bt=NT -zp1 -ix:\api;x:\timed\utils;x:\database\cbt

.c : ..\
.c : ..\utils

.c.obj : .AUTODEPEND
   wcc386 $(FLAGS) $[*

..\timednt.exe : areatags.obj ATTACH.obj bmg.obj CHECKCC.obj choose.obj &
  SELECT.obj shel2dos.obj SHOWHDR.obj SHOWINFO.obj SHOWMAIL.obj &
  getmsg.obj globals.obj header.obj help.obj idlekey.obj input.obj &
  list.obj lock.obj maint.obj marklist.obj mem.obj message.obj &
  cleanidx.obj clock.obj CONFIG.obj COPYMSG.obj DIRLIST.obj EDIT.obj &
  PICKONE.obj register.obj print.obj printer.obj &
  quotos2.obj READAREA.obj repedit.obj REPLY.obj request.obj &
  errorrpt.obj fido.obj FIND.obj getbody.obj GETFMT.obj &
  timed.obj tosslog.obj TXTBUILD.obj unrec.obj nodeglue.obj &
  working.obj wrap.obj xfile.obj video.obj scrnutil.obj picklist.obj &
  markchn.obj macro.obj charset.obj filter.obj v7.obj fd.obj &
  nodelist.obj nodebrow.obj private.obj

  wlink @timnt

