echo off
if %1. == . goto help
if exist %1con goto goon
echo *** Destination path not found!
goto help
:goon
echo getlang e c  from exec.c        to %1exec.c
getlang e c <exec.c >%1exec.c
if errorlevel 1 goto err
echo getlang e p  from exec.pas      to %1exec.pas
getlang e p <exec.pas >%1exec.pas
if errorlevel 1 goto err
echo getlang e c  from exec.h        to %1exec.h
getlang e c <exec.h >%1exec.h
if errorlevel 1 goto err
echo getlang e a  from checkpat.asm  to %1checkpat.asm
getlang e a <checkpat.asm >%1checkpat.asm
if errorlevel 1 goto err
echo getlang e c  from checkpat.h    to %1checkpat.h
getlang e c <checkpat.h >%1checkpat.h
if errorlevel 1 goto err
echo getlang e p  from checkpat.pas  to %1checkpat.pas
getlang e p <checkpat.pas >%1checkpat.pas
if errorlevel 1 goto err
echo getlang e c  from extest.c      to %1extest.c
getlang e c <extest.c >%1extest.c
if errorlevel 1 goto err
echo getlang e p  from extest.pas    to %1extest.pas
getlang e p <extest.pas >%1extest.pas
if errorlevel 1 goto err
echo getlang e a  from spawn.asm     to %1spawn.asm
getlang e a <spawn.asm >%1spawn.asm
if errorlevel 1 goto err
echo getlang e c  from compat.h      to %1compat.h
getlang e c <compat.h >%1compat.h
if errorlevel 1 goto err
goto exit
:err
echo *** Error!
:help
echo Usage: English destination-path
echo  copies the english version of all sources to the destination path.
echo  Example: english d:\eng\
:exit
