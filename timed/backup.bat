del c:\timed\backup.zip
pkzip2 -ex -P c:\timed\backup.zip d:\timed\*.c
pkzip2 -ex -P c:\timed\backup.zip d:\timed\*.h
pkzip2 -ex -P c:\timed\backup.zip d:\timed\utils\*.c
pkzip2 -ex -P c:\timed\backup.zip d:\timed\utils\*.h
pkzip2 -ex -P c:\timed\backup.zip d:\timed\os2\makefile.*
pkzip2 -ex -P c:\timed\backup.zip d:\timed\os2\*.lnk
pkzip2 -ex -P c:\timed\backup.zip d:\timed\dos\makefile.*
pkzip2 -ex -P c:\timed\backup.zip d:\timed\dos\*.lnk
pkzip2 -ex -P c:\timed\backup.zip d:\timed\d386\makefile.*
pkzip2 -ex -P c:\timed\backup.zip d:\timed\d386\*.lnk
pkzip2 -ex -P c:\timed\backup.zip d:\tc\api\*.c
pkzip2 -ex -P c:\timed\backup.zip d:\tc\api\*.h
copy c:\timed\backup.zip f:\safe\backup.zip

