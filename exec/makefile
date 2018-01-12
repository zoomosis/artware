# sample make file for EXTEST - Borland C++, Tasm, and Borland Make
MODEL=l
AMODEL=large

CC=c:\borlandc\bin\bcc -c -C -w-pia -w-par -m$(MODEL) -d -f- -k- -N -K -G -O1  -v- -vi- -o$* -Ic:\borlandc\include
LINK=c:\borlandc\bin\tlink /m /v c:\borlandc\lib\c0$(MODEL)
OBJ=exec$(MODEL).obj extest$(MODEL).obj checkpc$(MODEL).obj spawnc$(MODEL).obj
LIB=c:\borlandc\lib\c$(MODEL)
ASM=tasm /Zn /MX /W /DMODL=$(AMODEL)


extest.exe: $(OBJ)
   $(LINK) $(OBJ),extest,extest,$(LIB);

spawnc$(MODEL).obj: spawn.asm
   $(ASM) spawn,$*;

checkpc$(MODEL).obj: checkpat.asm
   $(ASM) checkpat,$*;

exec$(MODEL).obj: exec.c exec.h checkpat.h
   $(CC) exec.c

extest$(MODEL).obj: extest.c exec.h
   $(CC) extest.c

