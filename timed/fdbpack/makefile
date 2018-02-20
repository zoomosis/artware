CC16=wcl
CC=wcl386 
COPT=-zq -oneatx

all: fdbpack.exe fdbpackp.exe fdbpackn.exe

fdbpack.exe: fdbpack.c
	$(CC16) $(COPT) -bcl=dos -fe=fdbpack.exe fdbpack.c

fdbpackp.exe: fdbpack.c
	$(CC) $(COPT) -bcl=os2v2 -fe=fdbpackp.exe fdbpack.c

fdbpackn.exe: fdbpack.c
	$(CC) $(COPT) -bcl=nt -fe=fdbpackn.exe fdbpack.c
