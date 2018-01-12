echo off
if %1. == . goto help
if exist %1con goto goon
echo *** Zielpfad nicht gefunden!
goto help
:goon
echo getlang d c  von exec.c        nach %1exec.c
getlang d c <exec.c >%1exec.c
if errorlevel 1 goto err
echo getlang d p  von exec.pas      nach %1exec.pas
getlang d p <exec.pas >%1exec.pas
if errorlevel 1 goto err
echo getlang d c  von exec.h        nach %1exec.h
getlang d c <exec.h >%1exec.h
if errorlevel 1 goto err
echo getlang d a  von checkpat.asm  nach %1checkpat.asm
getlang d a <checkpat.asm >%1checkpat.asm
if errorlevel 1 goto err
echo getlang d c  von checkpat.h    nach %1checkpat.h
getlang d c <checkpat.h >%1checkpat.h
if errorlevel 1 goto err
echo getlang d p  von checkpat.pas  nach %1checkpat.pas
getlang d p <checkpat.pas >%1checkpat.pas
if errorlevel 1 goto err
echo getlang d c  von extest.c      nach %1extest.c
getlang d c <extest.c >%1extest.c
if errorlevel 1 goto err
echo getlang d p  von extest.pas    nach %1extest.pas
getlang d p <extest.pas >%1extest.pas
if errorlevel 1 goto err
echo getlang d a  von spawn.asm     nach %1spawn.asm
getlang d a <spawn.asm >%1spawn.asm
if errorlevel 1 goto err
echo getlang d c  von compat.h      nach %1compat.h
getlang d c <compat.h >%1compat.h
if errorlevel 1 goto err
goto exit
:err
echo *** Fehler!
:help
echo Benutzung: Deutsch Ziel-Pfad
echo  Kopiert die deutsche Version aller Quellen in den Ziel-Pfad.
echo  Beispiel: deutsch d:\dt\
:exit

