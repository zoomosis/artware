;
;	--- Version 3.3 93-11-29 09:06 ---
;
;	SPAWN.ASM - Main function for memory swapping spawn call.
;
;	Public Domain Software written by
;		Thomas Wagner
;		Ferrari electronic GmbH
;		Beusselstrasse 27
;		D-1000 Berlin 21
;		Germany
;
;>e
; Assemble with
;
; tasm  /DPASCAL spawn,spawnp  		- Turbo Pascal (Tasm only), near
; tasm  /DPASCAL /DFARCALL spawn,spawnp	- Turbo Pascal (Tasm only), far
; ?asm  spawn;		  		- C, default model (small)
; ?asm  /DMODL=large spawn  		- C, large model
;
;	NOTE:	For C, change the 'model' directive below according to your
;		memory model, or define MODL=xxx on the command line.
;
;		For Turbo C Huge model, you must give /DTC_HUGE on the
;		command line, or define it here.
;
;
; Main function:
;
;   PASCAL:
;   	function do_spawn (swapping: integer; 
;   			   execfname: string;
;			   cmdtail: string; 
;   			   envlen: word; 
;			   var envp;
;			   stdin: string; 
;		           stdout: string;
;			   stderr: string): integer;	
;
;   C:
;   	int do_spawn (int swapping,
;		      char *execfname, 
;		      char *cmdtail,
;		      unsigned envlen, 
;		      char *envp,
;		      char *stdin,
;		      char *stdout,
;		      char *stderr)
;
;   Parameters:
;
;	swapping - swap/spawn/exec function:
;			< 0: Exec, don't swap
;		  	  0: Spawn, don't swap
;			> 0: Spawn, swap
;			     in this case, prep_swap must have 
;			     been called beforehand (see below).
;
;	cmdtail - command tail for EXEC.
;
;	execfname - name and path of file to execute.
;
;	envlen - length of environment copy (may be 0).
;
;	envp -  pointer to environment block (must be aligned on
;		paragraph boundary). Unused if envlen is 0.
;
;	'cmdtail' and 'execfname' must be zero terminated, even when
;	calling from Pascal. For Pascal, the length byte of the string
;	is ignored.
;
;   Returns:
;	0000..00ff:	Returncode of EXECed program
;	03xx:		DOS-Error xx calling EXEC
;	0500:		Swapping requested, but prep_swap has not 
;			been called or returned an error
;	0501:		MCBs don't match expected setup
;	0502:		Error while swapping out
;	06xx:		DOS-Error xx on redirection
;
;
; For swapping, the swap method must be prepared before calling do_spawn.
;
;   PASCAL:
;	function prep_swap (method: word; swapfname: string): integer;
;   C:
;	int prep_swap (unsigned method, char *swapfname)
;
;   Parameters:
;
;	method	- bit-map of allowed swap devices:
;			01 - Allow EMS
;			02 - Allow XMS
;			04 - Allow File swap
;			10 - Try XMS first, then EMS
;			40 - Create file as "hidden"
;			80 - Use "create temp" call for file swap
;		       100 - Don't preallocate file
;		       200 - Check for Network, don't preallocate if net
;		      4000 - Environment block will not be swapped
;
;	swapfname - swap file name (may be undefined if the
;		    "method" parameters disallows file swap).
;		    The string must be zero terminated, even
;		    when calling from Pascal. For Pascal, the 
;		    length byte of the string is ignored.
;
;   Returns:
;
;   	A positive integer on success:
;		1 - EMS swap initialized
;		2 - XMS swap initialized
;		4 - File swap initialized
;	A negative integer on failure:
;		-1 - Couldn't allocate swap space
;		-2 - The spawn module is located too low in memory
;<
;>d
; Assemblierung mit
;
; tasm  /DPASCAL spawn,spawnp	  	- Turbo Pascal (nur Tasm), near
; tasm  /DPASCAL /DFARCALL spawn,spawnp	- Turbo Pascal (nur Tasm), far
; ?asm  spawn;		  		- C, default model (small)
; ?asm  /DMODL=large spawn  		- C, large model
;
;	HINWEIS: F�r C k�nnen Sie entweder die folgende 'model'-Direktive
;		Ihrem Speichermodell entsprechend �ndern, oder beim
;		Assemblieren MODL=xxx in der Kommandozeile definieren.
;
;		F�r Turbo C Huge model m�ssen Sie /DTC_HUGE beim Assemblieren
;		angeben, oder das Symbol in dieser Quelle definieren.
;
;
; Haupfunktion:
;
;   PASCAL:
;   	function do_spawn (swapping: integer; 
;   			   execfname: string;
;			   cmdtail: string; 
;   			   envlen: word; 
;			   var envp)
;
;   C:
;   	int do_spawn (int swapping,
;		      char *execfname, 
;		      char *cmdtail,
;		      unsigned envlen, 
;		      char *envp)
;
;   Parameter:
;
;	swapping - swap/spawn/exec Funktion:
;			< 0: Exec, kein Auslagern, keine R�ckkehr
;		  	  0: Spawn, kein Auslagern
;			> 0: Spawn, Auslagern
;			     in diesem Fall mu� vor Aufruf die Funktion
;			     prep_swap aufgerufen worden sein (siehe unten).
;
;	cmdtail - Parameter f�r das aufgerufene Programm.
;
;	execfname - Name und Pfad des auszuf�hrenden Programms.
;
;	envlen - L�nge der Umgebungsvariablen-Kopie (kann 0 sein).
;
;	envp -  Zeiger auf Umgebungsvariablen-Block (mu� auf Paragraph-
;		Grenze adjustiert sein). Wird nicht benutzt wenn der
;		envlen Parameter 0 ist.
;
;	'cmdtail' und 'execfname' m�ssen 0-Terminiert sein, selbst wenn
;	der Aufruf von Pascal erfolgt. Bei Aufruf von Pascal wird das
;	L�ngenbyte am Anfang des Strings ignoriert.
;
;   Liefert:
;
;	0000..00ff:	Returncode des ausgef�hrten Programms
;	03xx:		DOS-Fehler xx beim Aufruf von EXEC
;	0500:		Auslagern angefordert, aber prep_swap wurde
;			nicht aufgerufen oder nicht erfolgreich ausgef�hrt
;	0501:		MCBs sind fehlerhaft bzw. ver�ndert
;	0502:		Fehler bei Auslagerung
;	06xx:		DOS-Fehler xx bei Redirection
;
;
; Um Auslagern zu k�nnen, mu� die Auslagerungs-Methode vor Aufruf von
; do_spawn vorbereitet werden.
;
;   PASCAL:
;	function prep_swap (method: byte; swapfname: string): integer;
;   C:
;	int prep_swap (unsigned char method, char *swapfname)
;
;   Parameter:
;
;	method	- Bit-Map der zul�ssigen Auslagerungsziele und Optionen:
;			01 - EMS zulassen
;			02 - XMS zulassen
;			04 - Datei zulassen
;			10 - Erst XMS, dann EMS versuchen
;			40 - Datei "hidden" erzeugen
;			80 - "create temp" Funktion bei Datei-Auslagerung
;		       100 - Datei nicht pr�allozieren
;		       200 - Netzwerk pr�fen, nicht pr�allozieren wenn Netz
;		      4000 - Environmentblock nicht auslagern
;
;	swapfname - Auslagerungsdateiname bzw. Pfad (kann undefiniert
;		    sein wenn der "Method" Parameter keine Dateiauslagerung
;		    zul�sst).
;		    Der String mu� 0-Terminiert sein, selbst wenn
;		    der Aufruf von Pascal erfolgt. Bei Aufruf von Pascal 
;		    wird das L�ngenbyte am Anfang des Strings ignoriert.
;
;   Liefert:
;
;   	Einen positiven Wert bei Erfolg:
;		1 - EMS Auslagerung initialisiert
;		2 - XMS Auslagerung initialisiert
;		4 - Datei-Auslagerung initialisiert
;	Einen negativen Wert bei Fehler:
;		-1 - Kein Platz f�r Auslagerung
;		-2 - Das "spawn"-Modul ist zu nah am Beginn des Programms
;<
;--------------------------------------------------------------------------
;
;>e
;	Set NO_INHERIT to FALSE if you don't want do_exec to mess with
;	the handle table in the PSP, and/or you do want the child process
;	to inherit all open files.
;	If NO_INHERIT is TRUE, only the first five handles (the standard
;	ones) will be inherited, all others will be hidden. This allows
;	the child to open more files, and also protects you from the child
;	messing with any open handles.
;	NO_INHERIT should always be TRUE if you use functions to extend
;	the handle table (for more than 20 open files).
;
;	Set REDIRECT to FALSE if you do not want do_spawn to support redirection.
;
;	The following defines are for Turbo Pascal only:
;		Set PAS_FREE to TRUE to not swap the unused heap space.
;		Set TPAS_6 to TRUE for Turbo Pascal version 6.
;
;	When PAS_FREE is TRUE, the unused heap, i.e. the space between
;	HeapPtr and FreePtr for TP 5.x, or the space between HeapPtr and
;	HeapEnd for TP6.x, is temporarily chained into the DOS free space 
;	list, and is not swapped out. Depending on your applications heap 
;	usage, this can save a large amount of swap space, and considerably
;	speed up the swap.
;<
;>d
;	Setzen Sie NO_INHERIT auf FALSE wenn Sie nicht wollen, da� do_spawn
;	die Handle-Tabelle im PSP modifiziert, und/oder der Kindprozess
;	alle offenen Dateien erben soll.
;	Wenn NO_INHERIT TRUE ist, werden nur die ersten f�nf Handles
;	(die Standard-Handles) vererbt, alle anderen werden versteckt.
;	Dies erlaubt dem Kindprozess, mehr Dateien zu �ffnen, und sch�tzt
;	Sie au�erdem vor �nderungen an offenen Dateien.
;	NO_INHERIT sollte stets TRUE sein, wenn Sie Funktionen zur
;	Erweiterung der Handle-Tabelle (f�r mehr als 20 offene Dateien)
;	benutzen.
;
;	Setzen Sie REDIRECT auf FALSE wenn do_spawn keine Dateiumleitung 
;	unterst�tzen soll.
;
;	Die folgenden Definitionen werden nur f�r Turbo Pascal ben�tigt:
;		Setzen Sie PAS_FREE auf TRUE wenn der unbenutzte Heap
;		nicht ausgelagert werden soll.
;		Setzen Sie TPAS_6 auf TRUE f�r Turbo Pascal Version 6.
;
;	Wenn PAS_FREE TRUE ist, wird der nicht belegte Heap, d.h. der
;	Bereich zwischen HeapPtr und FreePtr f�r TP 5.x, bzw. zwischen
;	HeapPtr und HeapEnd f�r TP 6.x, tempor�r in die DOS-Freispeicher-
;	Liste eingef�gt, und damit nicht ausgelagert. Abh�ngig von der
;	Heap-Belegung Ihrer Applikation kann dies den Speicherbedarf
;	f�r die Auslagerung wesentlich verringern, und die Auslagerung
;	beschleunigen.
;<
;
FALSE		=	0
TRUE		=	NOT FALSE
;
NO_INHERIT	=	TRUE
REDIRECT =  FALSE
;
PAS_FREE	=	FALSE
TPAS_6		=	FALSE
;
;
	IFDEF	PASCAL
	.model	tpascal
	IFDEF	FARCALL
	%out	Pascal, far calls
	ELSE
	%out	Pascal, near calls
	ENDIF
;
	extrn	prefixseg: word
	IF	PAS_FREE
	extrn	HeapPtr: dword
	IF	TPAS_6
	extrn	HeapEnd: dword
	ELSE
	extrn	FreePtr: dword
	ENDIF
	ENDIF
;
ptrsize	=	1
	ELSE
	IFNDEF	MODL
   .model   large,c
   %out  large model
	ELSE
%	.model	MODL,c
%	%out	MODL model
	ENDIF
;
ptrsize	=	@DataSize
;
	extrn	_psp: word
	ENDIF
;
	public	do_spawn
	public	prep_swap
	IFNDEF	PASCAL
	public	swap_prep
	ENDIF
;
stacklen	=	256		;e local stack
					;d Lokaler Stack
;
;e	"ems_size" is the EMS block size: 16k.
;d	"ems_size" ist die EMS-Blockgr��e: 16k.
;
ems_size	=	16 * 1024	;e EMS block size
					;d EMS-Seiten-Gr��e
ems_parasize	=	ems_size / 16	;e same in paragraphs
					;d desgleichen in Paragraphen
ems_shift	=	10		;e shift factor for paragraphs
					;d Schiebefaktor f�r Paragraphen
ems_paramask	=	ems_parasize-1	;e block mask
					;d Maske f�r Paragraphen
;
;e	"xms_size" is the unit of measurement for XMS: 1k
;d	"xms_size" ist die Blockgr��e f�r XMS: 1k
;
xms_size	=	1024		;e XMS block size
					;d XMS-Block-Gr��e
xms_parasize	=	xms_size / 16	;e same in paragraphs
					;d desgleichen in Paragraphen
xms_shift	=	6		;e shift factor for paragraphs
					;d Schiebefaktor f�r Paragraphen
xms_paramask	=	xms_parasize-1	;e block mask
					;d Maske f�r Paragraphen
;
;e	Method flags
;d	Auslagerungsmethoden-Flags
;
USE_EMS		=	01h
USE_XMS		=	02h
USE_FILE	=	04h
XMS_FIRST	=	10h
HIDE_FILE	=	40h
CREAT_TEMP	=	80h
NO_PREALLOC	=	100h
CHECK_NET	=	200h
DONT_SWAP_ENV	=	4000h
;
;e	Return codes
;d	Resultatcodes
;
RC_TOOLOW	=	0102h
RC_BADPREP	=	0500h
RC_MCBERROR	=	0501h
RC_SWAPERROR	=	0502h
RC_REDIRFAIL	=	0600h
;
EMM_INT		=	67h
;
;e	The EXEC function parameter block
;d	Der Parameterblock f�r die EXEC-Funktion
;
exec_block	struc
envseg	dw	?		;e environment segment
				;d Segmentadresse Umgebungsvariablenblock
ppar	dw	?		;e program parameter string offset
				;d Programmparameterstring Offset
pparseg	dw	?		;e program parameter string segment
				;d Programmparameterstring Segment
fcb1	dw	?		; FCB offset
fcb1seg	dw	?		; FCB segment
fcb2	dw	?		; FCB offset
fcb2seg	dw	?		; FCB segment
exec_block	ends
;
;e	Structure of an XMS move control block
;d	Struktur eines XMS move Kontrollblocks
;
xms_control	struc
lenlo		dw	?	;e length to move (doubleword)
				;d L�nge f�r Move (Doppelwort)
lenhi		dw	?
srchnd		dw	?	;e source handle (0 for standard memory)
				;d Quell-Handle (0 f�r Standardspeicher)
srclo		dw	?	;e source address (doubleword or seg:off)
				;d Quell-Adresse (Doppelwort oder seg:off)
srchi		dw	?
desthnd		dw	?	;e destination handle (0 for standard memory)
				;d Ziel-Handle (0 f�r Standardspeicher)
destlo		dw	?	;e destination address (doubleword or seg:off)
				;d Ziel-Adresse (Doppelwort oder seg:off)
desthi		dw	?
xms_control	ends
;
;e	The structure of the start of an MCB (memory control block)
;d	Die Struktur des Beginns eines MCB (Speicher-Kontrollblock)
;
mcb		struc
id		db	?
owner		dw	?
paras		dw	?
mcb		ends
;>e
;	The structure of an internal MCB descriptor.
;	CAUTION: This structure is assumed to be no larger than 16 bytes
;	in several places in the code, and to be exactly 16 bytes when
;	swapping in from file. Be careful when changing this structure.
;<
;>d
;	Die Struktur eines internen MCB-Descriptors.
;	ACHTUNG: An mehreren Stellen im Code wird angenommen, da� die
;	L�nge dieser Struktur 16 Bytes nicht �berschreitet. Beim
;	Zur�cklesen von Datei wird angenommen, da� die L�nge exakt 16
;	Bytes betr�gt. Bei �nderungen in dieser Struktur ist daher
;	Vorsicht geboten.
;<
mcbdesc		struc
addr		dw	?	;e paragraph address of the MCB
				;d Paragraph-Adresse des MCB
msize		dw	?	;e size in paragraphs (excluding header)
				;d Gr��e in Paragraphen (Ausschlie�lich Header)
swoffset	dw	?	;e swap offset (0 in all blocks except first)
				;d Auslagerungs-Offset (0 in allen Bl�cken 
				;d au�er dem ersten)
swsize		dw	?	;e swap size (= msize + 1 except in first)
				;d Auslagerungsgr��e (= msize + 1 au�er
				;d im ersten Block)
num_follow	dw	?	;e number of following MCBs
				;d Zahl der folgenden MCBs
		dw	3 dup(?) ;e pad to paragraph (16 bytes)
				 ;d Auff�llen auf Paragraphen (16 Bytes)
mcbdesc		ends
;
;e	The variable block set up by prep_swap
;d	Der Variablenblock der durch prep_swap initialisiert wird
;
prep_block	struc
xmm		dd	?		;e XMM entry address
					;d Einsprungadresse XMM
first_mcb	dw	?		;e Segment of first MCB
					;d Segment des ersten MCB
psp_mcb		dw	?		;e Segment of MCB of our PSP
					;d Segment des MCB unseres PSP
env_mcb		dw	?		;e MCB of Environment segment
					;d MCB des Umgebungsvariablenblocks
noswap_mcb	dw	?		;e Env MCB that may not be swapped
					;d Env MCB der nicht Ausgelagert wird
		IF	NOT NO_INHERIT
noswap_mcb2	dw	?		;e Handle MCB that may not be swapped
					;d Handle MCB der nicht Ausgelagert wird
		ENDIF
		IFDEF	PASCAL
		IF	PAS_FREE
pmemid		db	?
pmempar		dw	?
		ENDIF
		ENDIF
ems_pageframe	dw	?		;e EMS page frame address
					;d EMS-Seiten-Adresse
handle		dw	?		;e EMS/XMS/File handle
					;d Handle f�r EMS/XMS/Datei
total_mcbs	dw	?		;e Total number of MCBs
					;d Gesamtzahl MCBs
swapmethod	db	?		;e Method for swapping
					;d Auslagerungsmethode
swapfilename	db	81 dup(?)	;e Swap file name if swapping to file
					;d Auslagerungsdateiname
prep_block	ends
;
;----------------------------------------------------------------------
;>e
;	Since we'll be moving code and data around in memory,
;	we can't address locations in the resident block with
;	normal address expressions. MASM does not support
;	defining variables with a fixed offset, so we have to resort
;	to a kludge, and define the shrunk-down code as a structure.
;	It would also be possible to use an absolute segment for the
;	definition, but this is not supported by the Turbo Pascal linker.
;
;	All references to low-core variables from low-core itself 
;	are made through DS, so we define a text macro "lmem" that 
;	expands to "ds:". When setting up low core from the normal
;	code, ES is used to address low memory, so this can't be used.
;<
;>d
;	Da Code und Daten in andere Speicherbereiche kopiert werden,
;	k�nnen Adressen im residenten Teil nicht mit normalen Adress-
;	ausdr�cken angesprochen werden. MASM unterst�tzt leider nicht
;	die Definition von Variablen mit festen Offsets, soda� wir
;	den residenten Teil als Struktur definieren m�ssen. Eine Definition
;	als absolutes Segment w�re zwar in MASM m�glich, dies wird jedoch
;	vom Turbo Pascal Linker nicht unterst�tzt.
;
;	Alle Referenzen auf Variablen im residenten Teil von diesem
;	Teil selbst geschehen �ber DS, hierf�r kann daher ein Text-Makro
;	"lmem" definiert werden, das zu "ds:" expandiert wird. Beim
;	Aufsetzen des residenten Teils vom normalen Programm geschieht
;	die Adressierung �ber ES, soda� dieses Makro dort nicht verwendet
;	werden darf.
;<
lmem	equ	<ds:>
;>e
;	The memory structure for the shrunk-down code, excluding the
;	code itself. The code follows this block.
;	The start of this block is the PSP.
;<
;>d
;	Die Struktur des residenten Teils, ausschlie�lich des Programm-
;	Codes. Der Programmcode folgt diesem Block.
;	Der Beginn dieses Blocks ist der PSP.
;<
parseg		struc
		db	18h dup(?)
psp_handletab	db	20 dup(?)	; Handle Table
psp_envptr	dw	?		; Environment Pointer
		dd	?
psp_handlenum	dw	?		; Number of Handles (DOS >= 3.3)
psp_handleptro	dw	?		; Handle Table Pointer (DOS >= 3.3)
psp_handleptrs	dw	?		; Handle Table Pointer Segment
		db	5ch-38h dup(?)	;e start after PSP
					;d Start nach PSP
;
save_ss		dw	?		;e 5C - saved global ss
					;d 5C - Sicherung globales SS
save_sp		dw	?		;e 5E - saved global sp
					;d 5E - Sicherung globaler SP
xfcb1		db	16 dup(?)	;e 60..6F - default FCB
					;d 60..6F - Standard-FCB
xfcb2		db	16 dup(?)	;e 70..7F - default FCB
					;d 70..7f - Standard-FCB
zero		dw	?		;e 80 Zero command tail length (dummy)
					;d 80 Null-Kommandozeile (Dummy)
;
expar		db	TYPE exec_block dup (?) ; exec-parameter-block
spx		dw	?		;e saved local sp
					;d Sicherung lokaler SP
div0_off	dw	?		;e divide by zero vector save
					;d Sicherung divide-by-zero Vektor
div0_seg	dw	?
		IF	NO_INHERIT
lhandlesave	db	26 dup(?)	;e saved handle table and pointer
					;d Sicherung Handle-Tabelle und Pointer
		IF	REDIRECT
lredirsav	db	6 dup(?)	;e saved redirection handles
					;d Sicherung Umleitungs-Handles
		ENDIF
		ENDIF
		IF	REDIRECT
lstdinsav	dw	3 dup(?)	;e duped redirection handles
					;d Umleitungs-Handles aus 'dup'
		ENDIF
filename	db	82 dup(?)	;e exec filename
					;d EXEC-Dateiname
progpars	db	128 dup(?)	;e command tail
					;d Kommandozeile
		db	stacklen dup(?)	;e local stack space
					;d Lokaler Stackbereich
mystack		db	?
lprep		db	TYPE prep_block dup(?)	;e the swapping variables
						;d die Auslagerungsvariablen
lcurrdesc	db	TYPE mcbdesc dup(?)	;e the current MCB descriptor
						;d Descriptor aktueller MCB
lxmsctl		db	TYPE xms_control dup(?)
eretcode	dw	?		;e EXEC return code
					;d Resultatcode EXEC
retflags	dw	?		;e EXEC return flags
					;d Resultatflags EXEC
cgetmcb		dw	?		;e address of get_mcb
					;d Adresse von get_mcb
;
parseg	ends
;
param_len	=	((TYPE parseg + 1) / 2) * 2	; make even
codebeg		=	param_len
;
	.code
;
;------------------------------------------------------------------------
;
lowcode_begin:
;>e
;       The following parts of the program code will be moved to
;	low core and executed there, so there must be no absolute 
;	memory references.
;	The call to get_mcb must be made indirect, since the offset
;	from the swap-in routine to get_mcb will not be the same
;	after moving.
;
;
;	get_mcb allocates a block of memory by modifying the MCB chain
;	directly.
;
;	On entry, lcurrdesc has the mcb descriptor for the block to
;		  allocate.
;
;	On exit,  Carry is set if the block couldn't be allocated.
;
;	Uses 	AX, BX, CX, ES
;	Modifies lprep.first_mcb
;<
;>d
;       Der folgende Teil des Programm-Codes wird in den unteren Speicher-
;	bereich kopiert und dort ausgef�hrt, es d�rfen daher keine
;	absoluten Speicheradressen verwendet werden.
;	Der Aufruf von get_mcb mu� indirekt erfolgen, da der Offset
;	der Einlagerungs-Routine zu get_mcb nach dem Kopieren nicht
;	mehr mit der Definition �bereinstimmt.
;
;
;	get_mcb alloziert einen Speicherblock durch direkte Modifikation
;		der MCB-Kette.
;
;	Bei Einsprung enth�lt lcurrdesc den Deskriptor f�r den zu
;		allozierenden Block.
;
;	Bei R�ckkehr ist das Carry-Flag gesetzt wenn der Block nicht
;		alloziert werden konnte.
;
;	Benutzt	AX, BX, CX, ES
;	Modifiziert lprep.first_mcb
;<
get_mcb	proc	near
;
	mov	ax,lmem lprep.first_mcb
	mov	bx,lmem lcurrdesc.addr
;
getmcb_loop:
	mov	es,ax
	cmp	ax,bx
	ja	gmcb_abort		;e halt if MCB > wanted
					;d Abbrechen wenn MCB > gew�nschtem
	je	mcb_found		;e jump if same addr as wanted
					;d jump wenn Adresse gleich gew�nschter
	add	ax,es:paras		;e last addr
					;d Letze Adresse
	inc	ax			; next mcb
	cmp	ax,bx
	jbe	getmcb_loop		;e Loop if next <= wanted
					;d Nochmal wenn n�chster <= gew�nschter
;
;>e
;	The wanted MCB starts within the current MCB. We now have to
;	create a new MCB at the wanted position, which is initially
;	free, and shorten the current MCB to reflect the reduced size.
;<
;>d
;	Der gew�nschte MCB beginnt innerhalb des laufenden MCB. Es mu�
;	nun ein MCB an der gew�nschten Position eingerichtet werden, und
;	der laufende MCB ist zu k�rzen.
;<
	cmp	es:owner,0
	jne	gmcb_abort		;e halt if not free
					;d Abbruch wenn nicht frei
	mov	bx,es			;e current
					;d laufender
	inc	bx			;e + 1 (header doesn't count)
					;d + 1 (Header z�hlt nicht)
	mov	ax,lmem lcurrdesc.addr
	sub	ax,bx			;e paragraphs between MCB and wanted
					;d Paragraphen zwischen MCB 
					;d und gew�nschtem
	mov	bx,es:paras		;e paras in current MCB
					;d Paragraphen in laufendem MCB
	sub	bx,ax			;e remaining paras
					;d Restliche Paragraphen
	dec	bx			;e -1 for header
					;d -1 f�r Header
	mov	es:paras,ax		;e set new size for current
					;d neue Gr��e f�r laufenden Setzen
	mov	cl,es:id		;e old id
					;d Alte ID
	mov	es:id,4dh		;e set id: there is a next
					;d Setze ID: es gibt einen n�chsten
	mov	ax,lmem lcurrdesc.addr
	mov	es,ax
	mov	es:id,cl		;e and init to free
					;d und initialisiere auf Frei
	mov	es:owner,0
	mov	es:paras,bx
;>e
;	We have found an MCB at the right address. If it's not free,
;	abort. Else check the size. If the size is ok, we're done 
;	(more or less).
;<
;>d
;	Wir haben einen MCB an der korrekten Adresse. Falls er nicht
;	frei ist, abbrechen. Sonst Gr��e pr�fen. Falls die Gr��e
;	korrekt ist, sind wir (mehr oder weniger) fertig.
;<
mcb_found:
	mov	es,ax
	cmp	es:owner,0
	je	mcb_check		;e continue if free
					;d weiter wenn Frei
;
gmcb_abort:
	stc
	ret
;
mcb_check:
	mov	ax,es:paras		;e size
					;d Gr��e
	cmp	ax,lmem lcurrdesc.msize	;e needed size
					;d gew�nschte Gr��e
	jae	mcb_ok			;e ok if enough space
					;d OK wenn genug Platz
;>e
;	If there's not enough room in this MCB, check if the next
;	MCB is free, too. If so, coalesce both MCB's and check again.
;<
;>d
;	Wenn in diesem MCB nicht genug Platz ist, den n�chsten MCB
;	pr�fen ob er ebenfalls frei ist. Falls ja, beide Bl�cke
;	kombinieren und nochmals pr�fen.
;<
	cmp	es:id,4dh
	jnz	gmcb_abort		;e halt if no next
					;d Abbruch wenn kein n�chster
	push	es			;e save current
					;d Laufenden sichern
	mov	bx,es
	add	ax,bx
	inc	ax			;e next MCB
					;d n�chter MCB
	mov	es,ax
	cmp	es:owner,0		;e next free ?
					;d ist der n�chste frei?
	jne	gmcb_abort		;e halt if not
					;d Abbruch wenn nein
	mov	ax,es:paras		;e else load size
					;d sonst Gr��e laden
	inc	ax			;e + 1 for header
					;d + 1 f�r Header
	mov	cl,es:id		;e and load ID
					;d und ID laden
	pop	es			;e back to last MCB
					;d zur�ck zum letzten MCB
	add	es:paras,ax		;e increase size
					;d Gr��e erh�hen
	mov	es:id,cl		;e and store ID
					;d und ID abspeichern
	jmp	mcb_check		;e now try again
					;d nochmal versuchen
;>e
;	The MCB is free and large enough. If it's larger than the
;	wanted size, create another MCB after the wanted.
;<
;>d
;	Der MCB ist frei und gro� genug. Wenn er gr��er als die gew�nschte
;	Gr��e ist, mu� ein weiterer MCB nach dem gew�nschten erzeugt werden.
;<
mcb_ok:
	mov	bx,es:paras
	sub	bx,lmem lcurrdesc.msize
	jz	mcb_no_next		;e ok, no next to create
					;d OK, kein neuer einzurichten
	push	es
	dec	bx			;e size of next block
					;d Gr��e des n�chsten Blocks
	mov	ax,es
	add	ax,lmem lcurrdesc.msize
	inc	ax			;e next MCB addr
					;d Adresse des n�chsten MCB
	mov	cl,es:id		;e id of this block
					;d ID dieses Blocks
	mov	es,ax			;e address next
					;d n�chsten adressieren
	mov	es:id,cl		;e store id
					;d ID abspeichern
	mov	es:paras,bx		;e store size
					;d Gr��e abspeichern
	mov	es:owner,0		;e and mark as free
					;d und als frei markieren
	pop	es			;e back to old MCB
					;d zur�ck zum alten MCB
	mov	es:id,4dh		;e mark next block present
					;d markieren da� weiterer existiert
	mov	ax,lmem lcurrdesc.msize	;e and set size to wanted
					;d und Gr��e auf gew�nschte setzen
	mov	es:paras,ax
;
mcb_no_next:
	mov	es:owner,cx		;e set owner to current PSP
					;d owner auf laufenden PSP setzen
;>e
;	Set the 'first_mcb' pointer to the current one, so we don't
;	walk through all the previous blocks the next time.
;	Also, check if the block we just allocated is the environment
;	segment of the program. If so, restore the environment pointer
;	in the PSP.
;<
;>d
;	Der 'first_mcb'-Pointer wird auf den aktuellen gesetzt,
;	damit beim n�chsten Mal nicht wieder alle vorangegangenen
;	Bl�cke abgelatscht werden m�ssen.
;	Au�erdem wird der gerade allozierte Block gepr�ft, ob es der
;	Umgebungsvariablenblock des Programms ist. Wenn ja, wird der
;	entsprechende Pointer im PSP wiederhergestellt.
;<
	mov	ax,es
	mov	lmem lprep.first_mcb,ax
	cmp	lmem lprep.env_mcb,ax
	jne	getmcb_finis
	inc	ax
	mov	lmem psp_envptr,ax
;
getmcb_finis:
	clc
	ret				;e all finished (whew!)
					;d endlich geschafft
;
get_mcb	endp
;
;
ireti:
	iret
;
;>e
;	The actual EXEC call.
;	Registers on entry:
;		BX	= paragraphs to keep (0 if no swap)
;		CX 	= length of environment to copy (words) or zero
;		DS:SI	= environment source
;		ES:DI	= environment destination
;		(ES = our low core code segment)
;
;
;	copy environment buffer down if present
;<
;>d
;	Der eigentliche EXEC Aufruf.
;	Register bei Einsprung:
;		BX	= Residente Paragraphen (0 wenn keine Auslagerung)
;		CX 	= L�nge des Umgebungsvariablenblocks in Worten oder 0
;		DS:SI	= Quelladresse Umgebungsvariablenblock
;		ES:DI	= Zieladresse Umgebungsvariablenblock
;		(ES = Adresse residenter Teil)
;
;
;	Umgebungsvariablenblock nach unten kopieren wenn vorhanden
;<
doexec:
	jcxz	noenvcpy
	rep movsw
;
noenvcpy:
	push	es			; DS = ES = low core = PSP
	pop	ds
	or	bx,bx
	jz	no_shrink
;
;e	first, shrink the base memory block down.
;d	Zuerst den Basisblock reduzieren.
;
        mov	ah,04ah
	int     21h                     ; resize memory block
;>e
;	Again walk all MCBs. This time, all blocks owned by the 
;	current process are released.
;<
;>d
;	Wieder mal alle MCBs durchgehen. Diesmal werden alle Bl�cke 
;	die zu diesem Proze� geh�ren freigegeben.
;<
	mov	si,lmem lprep.first_mcb
	or	si,si
	jz	no_shrink
	mov	dx,lmem lprep.psp_mcb
	mov	bx,dx
	inc	bx			; base PSP (MCB owner)
	mov	di,lmem lprep.noswap_mcb
;
free_loop:
	cmp	si,dx
	je	free_next		;e don't free base block
					;d Basisblock nicht freigeben
	cmp	si,di
	je	free_next

	IF	NOT NO_INHERIT
	cmp	si,lmem lprep.noswap_mcb2
	je	free_next
	ENDIF

	mov	es,si
	cmp	bx,es:owner		;e our process?
					;d unser Proze�?
	jne	free_next		;e next if not
					;d n�chsten wenn nein
	cmp	si,lmem lprep.env_mcb	;e is this the environment block?
					;d ist dies der Umgebungsvariablenblock?
	jne	free_noenv
	mov	ds:psp_envptr,0		;e else clear PSP pointer
					;d sonst PSP-pointer l�schen
;
free_noenv:
	inc	si
	mov	es,si
	dec	si
	mov	ah,049h			;e free memory block
					;d Speicher freigeben
	int	21h
;
free_next:
	mov	es,si
	cmp	es:id,4dh		;e normal block?
					;d Normaler Block?
	jne	free_ready		;e ready if end of chain
					;d Fertig wenn Ende der Kette
	add	si,es:paras		;e start + length
					;d Beginn + L�nge
	inc	si			;e next MCB
					;d N�chster MCB
	jmp	free_loop
;
free_ready:
	mov	ax,ds
	mov	es,ax
;
no_shrink:
	mov	dx,filename		;e params for exec
					;d Parameter f�r EXEC
	mov	bx,expar
	mov	ax,04b00h
	int	21h			; exec
;>e
;	Return from EXEC system call. Don't count on any register except
;	CS to be restored (DOS 2.11 and previous versions killed all regs).
;<
;>d
;	R�ckkehr vom EXEC Aufruf. Alle Register au�er CS k�nnen zerst�rt
;	sein (DOS-Versionen 2.11 und fr�her zerst�rten auch SS und SP).
;<
	mov	bx,cs
	mov	ds,bx
	mov	es,bx
	cli
	mov	ss,bx
	mov	sp,lmem spx
	sti
	cld
	mov	lmem eretcode,ax	;e save return code
					;d Resultatcode sichern
	pushf
	pop	bx
	mov	lmem retflags,bx	;e and returned flags
					;d und die gelieferten Flags
;
;e	Cancel Redirection
;d	Dateiumleitung aufheben
;
	IF	REDIRECT
	IF	NO_INHERIT
	mov	si,lredirsav
	mov	di,psp_handletab+5
	mov	cx,3
	rep movsw
	ENDIF
	mov	si,lstdinsav
	xor	cx,cx
;
lredirclose:
	lodsw
	cmp	ax,-1
	je	lredclosenext
	mov	bx,ax
	mov	ah,46h
	int	21h
;
lredclosenext:
	inc	cx
	cmp	cx,3
	jb	lredirclose
	ENDIF
;
	cmp	lmem lprep.swapmethod,0
	je	exec_memok
	jg	exec_expand
;
;	Terminate.
;
	test	lmem retflags,1		; carry?
	jnz	exec_term		;e use EXEc retcode if set
					;d Resultat von EXEC liefern wenn ja
	mov	ah,4dh			;e else get program return code
					;d Sonst Resultat von Programm holen
	int	21h
;
exec_term:
	mov	ah,4ch
	int	21h
;
;
exec_expand:
	mov	ah,4ah			; expand memory
	mov	bx,lmem lcurrdesc.msize
	int	21h
	jnc	exec_memok
	mov	ax,4cffh
	int	21h			;e terminate on error
					;d Abbrechen bei Fehler
;
;e	Swap memory back
;d	Zur�cklesen Speicher
;
	nop
;
exec_memok:
;
;e	FALL THROUGH to the appropriate swap-in routine
;d	Weiter in der passenden Einlagerungsroutine
;
;
getmcboff	=	offset get_mcb - offset lowcode_begin
iretoff		=	offset ireti - offset lowcode_begin
doexec_entry	=	offset doexec - offset lowcode_begin
base_length	=	offset $ - offset lowcode_begin
;
;-----------------------------------------------------------------------
;>e
;	The various swap in routines follow. Only one of the routines
;	is copied to low memory.
;	Note that the routines are never actually called, the EXEC return
;	code falls through. The final RET thus will return to the restored
;	memory image.
;
;	On entry, DS must point to low core.
;	On exit to the restored code, DS is unchanged.
;
;
;	swapin_ems:	swap in from EMS.
;<
;>d
;	Es folgen die verschiedenen Wiederherstellungsroutinen. Nur 
;	eine der Routinen wird in den residenten Teil kopiert.
;	Beachten Sie, da� die Routinen nicht tats�chlich aufgerufen
;	werden, der Code nach dem EXEC-Aufruf l�uft in sie hinein. Das
;	abschlie�ende RET kehrt daher in das wiederhergestellte Haupt-
;	programm zur�ck.
;
;	Bei Einsprung zeight DS auf den Basis-PSP.
;	Bei R�ckkehr in das wiederhergestellte Programm ist DS unver�ndert.
;
;
;	swapin_ems:	Wiederherstellen von EMS.
;<
swapin_ems	proc	far
;
	xor	bx,bx
	mov	si,ems_parasize
	mov	dx,lmem lprep.handle	; EMS handle
;
swinems_main:
	push	ds
	mov	cx,lmem lcurrdesc.swsize	;e block length in paras
						;d Blockl�nge in Paragraphen
	mov	di,lmem lcurrdesc.swoffset	;e swap offset
						;d Lese-Offset
	mov	es,lmem lcurrdesc.addr		;e segment to swap
						;d Lese-Segment
	mov	ds,lmem lprep.ems_pageframe	; page frame address
;
	mov	ax,ems_parasize		;e max length
					;d Maximale L�nge
	sub	ax,si			;e minus current offset
					;d Minus laufender Offset
	jnz	swinems_ok		;e go copy if nonzero
					;d Kopieren wenn nicht 0
;
swinems_loop:
	mov	ax,4400h		;e map in next page
					;d N�chste EMS-Page einmappen
	int	EMM_INT
	or	ah,ah
	jnz	swinems_error
	mov	si,0			;e reset offset
					;d Offset zur�cksetzen
	inc	bx			;e bump up page number
					;d Seitennummer erh�hen
	mov	ax,ems_parasize		;e max length to copy
					;d Maximale L�nge
;
swinems_ok:
	cmp	ax,cx			;e length to copy
					;d zu kopierende L�nge
	jbe	swinems_doit		;e go do it if <= total length
					;d kopieren wenn <= Gesamtl�nge
	mov	ax,cx			;e else use total length
					;d sonst Gesamtl�nge kopieren
;
swinems_doit:
	sub	cx,ax			;e subtract copy length from total
					;d Gesamtl�nge -= kopierte L�nge
	push	cx			;e and save
					;d sichern
	push	ax			;e save the copy length in paras
					;d Sichern Kopierl�nge in Paragraphen
	push	si
	push	di
	mov	cl,3
	shl	ax,cl			;e convert to number of words (!)
					;d Konvertieren in Anzahl Worte (!)
	inc	cl
	shl	si,cl			;e convert to byte address
					;d In Byte-Adresse konvertieren
	mov	cx,ax
	rep movsw
	pop	di
	pop	si
	pop	cx			;e copy length in paras
					;d Kopierl�nge in Paragraphen
	mov	ax,es
	add	ax,cx			;e add copy length to dest segment
					;d Kopierl�nge auf Zielsegment
	add	si,cx			;e and EMS page offset
					;d und EMS-Seiten-Offset addieren
	mov	es,ax
	pop	cx			;e remaining length
					;d Restl�nge
	or	cx,cx			;e did we copy everything?
					;d Alles kopiert?
	jnz	swinems_loop		;e go loop if not
					;d Nochmal wenn nein
;
	pop	ds
	cmp	lmem lcurrdesc.num_follow,0	;e another MCB?
						;d noch ein MCB?
	je	swinems_complete	;e exit if not
					;d Fertig wenn nein
;
;e	Another MCB follows, read next mcb descriptor into currdesc
;d	Ein weiterer MCB folgt, lesen MCB Deskriptor nach currdesc
;
	cmp	si,ems_parasize
	jb	swinems_nonewpage	;e no new block needed
					;d kein neuer Block n�tig
	mov	ax,4400h		; map page, phys = 0
	int	EMM_INT
	or	ah,ah
	jnz	swinems_error1
	mov	si,0
	inc	bx
;
swinems_nonewpage:
	push	si
	push	ds
	mov	ax,ds
	mov	es,ax
	mov	ds,lmem lprep.ems_pageframe	; page frame address
	mov	cl,4
	shl	si,cl			;e convert to byte address
					;d in Byte-Adresse konvertieren
	mov	cx,TYPE mcbdesc
	mov	di,lcurrdesc
	rep movsb
	pop	ds
	pop	si
	inc	si			;e one paragraph
					;d Einen Paragraphen
;
	push	bx
	call	lmem cgetmcb
	pop	bx
	jc	swinems_error1
	jmp	swinems_main
;
swinems_complete:
	mov	ah,45h			;e release EMS pages
					;d Freigeben EMS-Speicher
	int	EMM_INT
	ret
;
swinems_error:
	pop	ds
swinems_error1:
	mov	ah,45h			;e release EMS pages on error
					;d Bei Fehler EMS freigeben
	int	EMM_INT
	mov	ax,4cffh
	int	21h			; terminate
;
swapin_ems	endp
;
swinems_length	= offset $ - offset swapin_ems
;
;
;e	swapin_xms:	swap in from XMS.
;d	swapin_xms:	Wiederherstellen von XMS.
;
swapin_xms	proc	far
;
	mov	ax,lmem lprep.handle	; XMS handle
	mov	lmem lxmsctl.srchnd,ax 	;e source is XMS
					;d Quelle ist XMS
	mov	lmem lxmsctl.desthnd,0 	;e dest is normal memory
					;d Ziel ist Standardspeicher
	mov	lmem lxmsctl.srclo,0
	mov	lmem lxmsctl.srchi,0
;
swinxms_main:
	mov	ax,lmem lcurrdesc.swsize ;e size in paragraphs
					 ;d Gr��e in Paragraphen
	mov	cl,4
	rol	ax,cl			;e size in bytes + high nibble
					;d Gr��e in Bytes + high nibble
	mov	dx,ax
	and	ax,0fff0h		; low word
	and	dx,0000fh		; high word
	mov	lmem lxmsctl.lenlo,ax	;e into control block
					;d in den Kontrollblock
	mov	lmem lxmsctl.lenhi,dx
	mov	ax,lmem lcurrdesc.swoffset	;e swap offset
						;d Lese-Offset
	mov	lmem lxmsctl.destlo,ax 		;e into control block
						;d in den Kontrollblock
	mov	ax,lmem lcurrdesc.addr		;e segment to swap
						;d Lese-Segment
	mov	lmem lxmsctl.desthi,ax
	mov	si,lxmsctl
	mov	ah,0bh
	call	lmem lprep.xmm		; move it
	or	ax,ax
	jz	swinxms_error
	mov	ax,lmem lxmsctl.lenlo	;e adjust source addr
					;d Quelladresse adjustieren
	add	lmem lxmsctl.srclo,ax
	mov	ax,lmem lxmsctl.lenhi
	adc	lmem lxmsctl.srchi,ax
;
	cmp	lmem lcurrdesc.num_follow,0	;e another MCB?
						;d noch ein MCB?
	je	swinxms_complete
;
	mov	lmem lxmsctl.lenlo,TYPE mcbdesc
	mov	lmem lxmsctl.lenhi,0
	mov	lmem lxmsctl.desthi,ds
	mov	lmem lxmsctl.destlo,lcurrdesc
	mov	si,lxmsctl
	mov	ah,0bh
	call	lmem lprep.xmm		; move it
	or	ax,ax
	jz	swinxms_error
	add	lmem lxmsctl.srclo,16	; one paragraph
	adc	lmem lxmsctl.srchi,0
;
	call	lmem cgetmcb
	jc	swinxms_error
	jmp	swinxms_main
;
swinxms_complete:
	mov	ah,0ah			;e release XMS frame
					;d Freigeben XMS-Speicher
	mov	dx,lmem lprep.handle   	; XMS handle
	call	lmem lprep.xmm
	ret
;
swinxms_error:
	mov	ah,0ah			;e release XMS frame on error
					;d Bei Fehler XMS-Speicher freigeben
	call	lmem lprep.xmm
	mov	ax,4c00h
	int	21h
;
swapin_xms	endp
;
swinxms_length	= offset $ - offset swapin_xms
;
;
;e	swapin_file:	swap in from file.
;d	swapin_file:	Wiederherstellen von Datei.
;
swapin_file	proc	far
;
	mov	dx,lprep.swapfilename
	mov	ax,3d00h			; open file
	int	21h
	jc	swinfile_error2
	mov	bx,ax				; file handle
;
swinfile_main:
	push	ds
	mov	cx,lmem lcurrdesc.swsize	;e size in paragraphs
						;d Blockl�nge in Paragraphen
	mov	dx,lmem lcurrdesc.swoffset	; swap offset
	mov	ds,lmem lcurrdesc.addr		; segment to swap
;
swinfile_loop:
	mov	ax,cx
	cmp	ah,8h			;e above 32k?
					;d mehr als 32k?
	jbe	swinfile_ok		;e go read if not
					;d lesen wenn nein
	mov	ax,800h			;e else read 32k
					;d sonst 32k lesen
;
swinfile_ok:
	sub	cx,ax			;e remaining length
					;d Restl�nge
	push	cx			;e save it
					;d sichern
	push	ax			;e and save paras to read
					;d Sichern L�nge in Paragraphen
	mov	cl,4
	shl	ax,cl			;e convert to bytes
					;d Konvertieren in Anzahl Bytes
	mov	cx,ax
	mov	ah,3fh			; read
	int	21h
	jc	swinfile_error
	cmp	ax,cx
	jne	swinfile_error
	pop	cx			;e paras read
					;d Gelesene Paragraphen
	mov	ax,ds
	add	ax,cx			;e bump up dest segment
					;d Add. Kopierl�nge auf Zielsegment
	mov	ds,ax
	pop	cx			;e remaining length
					;d Restl�nge
	or	cx,cx			;e anything left?
					;d Noch etwas �brig?
	jnz	swinfile_loop		;e go loop if yes
					;d Nochmal wenn ja
;
	pop	ds
	cmp	lmem lcurrdesc.num_follow,0	; another MCB?
						;d noch ein MCB?
	je	swinfile_complete	;e ready if not
					;d Fertig wenn nein
	mov	cx,16			;e read one paragraph
					;d einen Paragraphen lesen
	mov	dx,lcurrdesc
	mov	ah,3fh
	int	21h
	jc	swinfile_error1
	cmp	ax,cx
	jne	swinfile_error1
;
	push	bx
	call	lmem cgetmcb
	pop	bx
	jc	swinfile_error1
	jmp	swinfile_main
;
;
swinfile_complete:
	mov	ah,3eh			; close file
	int	21h
	mov	dx,lprep.swapfilename
	mov	ah,41h			; delete file
	int	21h
	ret
;
swinfile_error:
	pop	cx
	pop	cx
	pop	ds
swinfile_error1:
	mov	ah,3eh			; close file
	int	21h
swinfile_error2:
	mov	dx,lprep.swapfilename
	mov	ah,41h			; delete file
	int	21h
	mov	ax,4cffh
	int	21h
;
swapin_file	endp
;
swinfile_length	= offset $ - offset swapin_file
;
;
;e	swapin_none:	no swap, return immediately.
;d	swapin_none:	Kein Wiederherstellen, nur R�ckkehr.
;
swapin_none	proc	far
;
	ret
;
swapin_none	endp
;
;
	IF	swinems_length GT swinxms_length
swcodelen	=	swinems_length
	ELSE
swcodelen	=	swinxms_length
	ENDIF
	IF	swinfile_length GT swcodelen
swcodelen	=	swinfile_length
	ENDIF
;
swap_codelen	=	((swcodelen + 1) / 2) * 2
;
codelen		=	base_length + swap_codelen
reslen		=	codebeg + codelen
keep_paras	=	(reslen + 15) shr 4	;e paragraphs to keep
						;d Residente Paragraphen
swapbeg		=	keep_paras shl 4	;e start of swap space
						;d Beginn Auslagerungsbereich
savespace	=	swapbeg - 5ch	;e length of overwritten area
					;d L�nge �berschriebener Bereich
;
;--------------------------------------------------------------------
;
	IFDEF	PASCAL
	.data
	extrn	swap_prep: prep_block
	ELSE
	IFDEF	TC_HUGE
	.fardata?	my_data
	assume	ds:my_data
	ELSE
	.data?
	ENDIF
	ENDIF
;
;>e
;	Space for saving the part of the memory image below the
;	swap area that is overwritten by our code.
;<
;>d
;	Platz zum Sichern des Teils des Speichers der vom 
;	Aus-/Einlagerungscode und den Variablen �berschrieben wird.
;<
save_dat	db	savespace dup(?)
;>e
;	Variables used while swapping out.
;	The "swap_prep" structure is initialized by prep_swap.
;<
;>d
;	Variablen zur Benutzung beim Auslagern.
;	Die "swap_prep" Struktur wird durch prep_swap initialisiert.
;<
	IFNDEF	PASCAL
swap_prep	prep_block	<>
	ENDIF
nextmcb		mcbdesc		<>
currdesc	mcbdesc		<>
xmsctl		xms_control	<>
ems_curpage	dw		?	;e current EMS page number
					;d Laufende Nummer EMS-Seite
ems_curoff	dw		?	;e current EMS offset (paragraph)
					;d Laufender EMS Offset (Paragraph)
;
;--------------------------------------------------------------------
;       
	.code
;>e
;	swapout_ems:	swap out an MCB block to EMS.
;
;	Entry:	"currdesc" 	contains description of block to swap
;		"nextmcb"	contains MCB-descriptor of next block
;				if currdesc.num_follow is nonzero
;
;	Exit:	0 if OK, != 0 if error, Zero-flag set accordingly.
;
;	Uses:	All regs excpt DS
;<
;>d
;	swapout_ems:	Auslagern eines MCB nach EMS.
;
;	Ein:	"currdesc" 	Beschreibung des auszulagernden Blocks
;		"nextmcb"	MCB-Deskriptor des n�chsten Blocks
;				wenn currdesc.num_follow nicht 0 ist.
;
;	Aus:	0 wenn OK, != 0 bei Fehler, Zero-flag entsprechend gesetzt.
;
;	Benutzt: Alle Register au�er DS
;<
swapout_ems	proc	near
;
	push	ds
	mov	cx,currdesc.swsize	;e block length in paras
					;d Blockl�nge in Paragraphen
	mov	si,currdesc.swoffset	; swap offset
	mov	dx,swap_prep.handle	; EMS handle
	mov	bx,ems_curpage		;e current EMS page
					;d laufende EMS Seite
	mov	di,ems_curoff		;e current EMS page offset (paras)
					;d laufender EMS Offset (Paragraph)
	mov	es,swap_prep.ems_pageframe	; page frame address
	mov	ds,currdesc.addr	; segment to swap
;
	mov	ax,ems_parasize		;e max length
					;d Maximale L�nge
	sub	ax,di			;e minus current offset
					;d Minus laufender Offset
	jnz	swems_ok		;e go copy if there's room
					;d Kopieren wenn noch Platz ist
;
swems_loop:
	mov	ax,4400h		;e map in next page
					;d N�chste EMS-Page einmappen
	int	EMM_INT
	or	ah,ah
	jnz	swems_error
	mov	di,0			;e reset offset
					;d Offset zur�cksetzen
	inc	bx			;e bump up page number
					;d Seitennummer erh�hen
	mov	ax,ems_parasize		;e max length to copy
					;d Maximale L�nge
;
swems_ok:
	cmp	ax,cx			;e length to copy
					;d zu kopierende L�nge
	jbe	swems_doit		;e go do it if <= total length
					;d kopieren wenn <= Gesamtl�nge
	mov	ax,cx			;e else use total length
					;d sonst Gesamtl�nge kopieren
;
swems_doit:
	sub	cx,ax			;e subtract copy length from total
					;d Gesamtl�nge -= kopierte L�nge
	push	cx			;e and save
					;d sichern
	push	ax			;e save the copy length in paras
					;d Sichern Kopierl�nge in Paragraphen
	push	si
	push	di
	mov	cl,3
	shl	ax,cl			;e convert to number of words (!)
					;d Konvertieren in Anzahl Worte (!)
	inc	cl
	shl	di,cl			;e convert to byte address
					;d In Byte-Adresse konvertieren
	mov	cx,ax
	rep movsw
	pop	di
	pop	si
	pop	cx			;e copy length in paras
					;d Kopierl�nge in Paragraphen
	mov	ax,ds
	add	ax,cx			;e add copy length to source segment
					;d Kopierl�nge auf Zielsegment
	add	di,cx			;e and EMS page offset
					;d und EMS-Seiten-Offset addieren
	mov	ds,ax
	pop	cx			;e remaining length
					;d Restl�nge
	or	cx,cx			;e did we copy everything?
					;d Alles kopiert?
	jnz	swems_loop		;e go loop if not
					;d Nochmal wenn nein
;
	pop	ds
	cmp	currdesc.num_follow,0	;e another MCB?
					;d noch ein MCB?
	je	swems_complete		;e exit if not
					;d Fertig wenn nein
;
;e	Another MCB follows, append nextmcb to save block.
;d	Ein weiterer MCB folgt, nextmcb an Block anf�gen.
;
	cmp	di,ems_parasize
	jb	swems_nonewpage		;e no new block needed
					;d kein neuer Block n�tig
	mov	ax,4400h		; map page, phys = 0
	int	EMM_INT
	or	ah,ah
	jnz	swems_error1
	mov	di,0
	inc	bx
;
swems_nonewpage:
	push	di
	mov	cl,4
	shl	di,cl			;e convert to byte address
					;d in Byte-Adresse konvertieren
	mov	cx,TYPE mcbdesc
	mov	si,offset nextmcb
	rep movsb
	pop	di
	inc	di			;e one paragraph
					;d Einen Paragraphen
;
swems_complete:
	mov	ems_curpage,bx
	mov	ems_curoff,di
	xor	ax,ax
	ret
;
swems_error:
	pop	ds
swems_error1:
	mov	ah,45h			;e release EMS pages on error
					;d Bei Fehler EMS freigeben
	int	EMM_INT
	mov	ax,RC_SWAPERROR
	or	ax,ax
	ret
;
swapout_ems	endp
;
;>e
;	swapout_xms:	swap out an MCB block to XMS.
;
;	Entry:	"currdesc" 	contains description of block to swap
;		"nextmcb"	contains MCB-descriptor of next block
;				if currdesc.num_follow is nonzero
;
;	Exit:	0 if OK, -1 if error, Zero-flag set accordingly.
;
;	Uses:	All regs excpt DS
;<
;>d
;	swapout_xms:	Auslagern eines MCB nach XMS.
;
;	Ein:	"currdesc" 	Beschreibung des auszulagernden Blocks
;		"nextmcb"	MCB-Deskriptor des n�chsten Blocks
;				wenn currdesc.num_follow nicht 0 ist.
;
;	Aus:	0 wenn OK, != 0 bei Fehler, Zero-flag entsprechend gesetzt.
;
;	Benutzt: Alle Register au�er DS
;<
swapout_xms	proc	near
;
	push	ds
	pop	es
	mov	ax,currdesc.swsize	;e size in paragraphs
					;d Gr��e in Paragraphen
	mov	cl,4
	rol	ax,cl			;e size in bytes + high nibble
					;d Gr��e in Bytes + high nibble
	mov	dx,ax
	and	ax,0fff0h		; low word
	and	dx,0000fh		; high word
	mov	xmsctl.lenlo,ax		; into control block
	mov	xmsctl.lenhi,dx
	mov	xmsctl.srchnd,0		;e source is normal memory
					;d Quelle ist Standardspeicher
	mov	ax,currdesc.swoffset	; swap offset
	mov	xmsctl.srclo,ax		; into control block
	mov	ax,currdesc.addr	; segment to swap
	mov	xmsctl.srchi,ax
	mov	ax,swap_prep.handle	; XMS handle
	mov	xmsctl.desthnd,ax
	mov	si,offset xmsctl
	mov	ah,0bh
	call	swap_prep.xmm		; move it
	or	ax,ax
	jz	swxms_error
	mov	ax,xmsctl.lenlo		;e adjust destination addr
					;d Zieladresse adjustieren
	add	xmsctl.destlo,ax
	mov	ax,xmsctl.lenhi
	adc	xmsctl.desthi,ax
;
	cmp	currdesc.num_follow,0	;e another MCB?
					;d noch ein MCB?
	je	swxms_complete
;
	mov	xmsctl.lenlo,TYPE mcbdesc
	mov	xmsctl.lenhi,0
	mov	xmsctl.srchi,ds
	mov	xmsctl.srclo,offset nextmcb
	mov	si,offset xmsctl
	mov	ah,0bh
	call	swap_prep.xmm		; move it
	or	ax,ax
	jz	swxms_error
	add	xmsctl.destlo,16	; one paragraph
	adc	xmsctl.desthi,0
;
swxms_complete:
	xor	ax,ax
	ret
;
swxms_error:
	mov	ah,0ah			;e release XMS frame on error
					;d Bei Fehler XMS-Speicher freigeben
	mov	dx,swap_prep.handle	; XMS handle
	call	swap_prep.xmm
	mov	ax,RC_SWAPERROR
	or	ax,ax
	ret
;
swapout_xms	endp
;
;>e
;	swapout_file:	swap out an MCB block to file.
;
;	Entry:	"currdesc" 	contains description of block to swap
;		"nextmcb"	contains MCB-descriptor of next block
;				if currdesc.num_follow is nonzero
;
;	Exit:	0 if OK, -1 if error, Zero-flag set accordingly.
;
;	Uses:	All regs excpt DS
;<
;>d
;	swapout_file:	Auslagern eines MCB auf Datei.
;
;	Ein:	"currdesc" 	Beschreibung des auszulagernden Blocks
;		"nextmcb"	MCB-Deskriptor des n�chsten Blocks
;				wenn currdesc.num_follow nicht 0 ist.
;
;	Aus:	0 wenn OK, != 0 bei Fehler, Zero-flag entsprechend gesetzt.
;
;	Benutzt: Alle Register au�er DS
;<
swapout_file	proc	near
;
	push	ds
	mov	cx,currdesc.swsize	;e size in paragraphs
					;d Blockl�nge in Paragraphen
	mov	bx,swap_prep.handle	; file handle
	mov	dx,currdesc.swoffset	; swap offset
	mov	ds,currdesc.addr	; segment to swap
;
swfile_loop:
	mov	ax,cx
	cmp	ah,8h			;e above 32k?
					;d mehr als 32k?
	jbe	swfile_ok		;e go write if not
					;d schreiben wenn nein
	mov	ax,800h			;e else write 32k
					;d sonst 32k schreiben
;
swfile_ok:
	sub	cx,ax			;e remaining length
					;d Restl�nge
	push	cx			;e save it
					;d sichern
	push	ax			;e and save paras to write
					;d Sichern L�nge in Paragraphen
	mov	cl,4
	shl	ax,cl			;e convert to bytes
					;d Konvertieren in Anzahl Bytes
	mov	cx,ax
	mov	ah,40h			; write
	int	21h
	jc	swfile_error
	cmp	ax,cx
	jne	swfile_error
	pop	cx			;e paras written
					;d Geschriebene Paragraphen
	mov	ax,ds
	add	ax,cx			;e bump up source segment
					;d Add. Kopierl�nge auf Quellsegment
	mov	ds,ax
	pop	cx			;e remaining length
					;d Restl�nge
	or	cx,cx			;e anything left?
					;d Noch etwas �brig?
	jnz	swfile_loop		;e go loop if yes
					;d Nochmal wenn ja
;
	pop	ds
	cmp	currdesc.num_follow,0	;e another MCB?
					;d noch ein MCB?
	je	swfile_complete		;e ready if not
					;d Fertig wenn nein
	mov	cx,16			;e write one paragraph
					;d einen Paragraphen schreiben
	mov	dx,offset nextmcb
	mov	ah,40h
	int	21h
	jc	swfile_error1
	cmp	ax,cx
	jne	swfile_error1
;
swfile_complete:
	xor	ax,ax
	ret
;
swfile_error:
	pop	cx
	pop	cx
	pop	ds
swfile_error1:
	mov	ah,3eh			; close file
	int	21h
	mov	dx,offset swap_prep.swapfilename
	mov	ah,41h			; delete file
	int	21h
	mov	ax,RC_SWAPERROR
	or	ax,ax
	ret
;
swapout_file	endp
;
;--------------------------------------------------------------------------
;
	IF	REDIRECT
;>e
;	@redirect: Redirect a file.
;
;	Entry:	DS:SI = Filename pointer
;		AX zero if filename is NULL
;		CX    = Handle to redirect
;		ES:DI = Handle save pointer
;
;	Exit:	Carry set on error, then AL has DOS error code
;		ES:DI updated
;
;	Uses:	AX,BX,DX,SI
;<
;>d
;	@redirect: Umleiten einer Datei.
;
;	Entry:	DS:SI = Zeiger auf Dateinamen
;		AX ist 0 wenn Dateinamenszeiger NULL ist
;		CX    = Umzuleitendes Handle
;		ES:DI = Zeiger auf Handle-Sicherungswort
;
;	Exit:	Carry gesetzt bei Fehler, dann ist AL=DOS-Fehlercode
;		ES:DI wird modifiziert.
;
;	Uses:	AX,BX,DX,SI
;<
@redirect	proc	near
		local	doserr
;
	or	ax,ax
	jz	no_redirect
	cmp	byte ptr [si],0
	jne	do_redirect
;
no_redirect:
	mov	ax,-1
	stosw
	ret
;
do_redirect:
	IFDEF	PASCAL
	inc	si			;e skip length byte
					;d L�ngenbyte �berspringen
	ENDIF
	or	cx,cx
	jnz	redir_write
	mov	dx,si
	mov	ax,3d00h	; open file, read only
	int	21h
	mov	doserr,ax
	jc	redir_failed
;
redir_ok:
	mov	dx,ax
	mov	ah,45h		; duplicate handle
	mov	bx,cx
	int	21h
	mov	doserr,ax
	jc	redir_failed_dup
	push	ax
	mov	bx,dx
	mov	ah,46h		; force duplicate handle
	int	21h
	mov	doserr,ax
	pop	ax
	jc	redir_failed_force
	stosw
	mov	ah,3eh		; close file
	int	21h
	clc
	ret
;
redir_failed_force:
	mov	bx,ax
	mov	ah,3eh		; close file
	int	21h
;
redir_failed_dup:
	mov	bx,dx
	mov	ah,3eh		; close file
	int	21h
;
redir_failed:
	mov	ax,doserr
	stc
	ret
;
redir_write:
	cmp	byte ptr [si],'>'
	jne	no_append
	inc	si
	mov	dx,si
	mov	ax,3d02h		; open file, read/write
	int	21h
	jc	no_append
	mov	bx,ax
	push	cx
	mov	ax,4202h		; move file, offset from EOF
	xor	cx,cx
	mov	dx,cx
	int	21h
	mov	doserr,ax
	pop	cx
	mov	ax,bx
	jnc	redir_ok
	mov	dx,ax
	jmp	redir_failed_dup
;
no_append:
	mov	dx,si
	mov	ah,3ch
	push	cx
	xor	cx,cx
	int	21h
	mov	doserr,ax
	pop	cx
	jc	redir_failed
	jmp	redir_ok
;
@redirect	endp
;
	ENDIF
;
;--------------------------------------------------------------------------
;
	IFDEF	PASCAL
	IF	PAS_FREE
;
;e	@freepas: Chain unused Pascal Heap into DOS free list.
;d	@freepas: Den unbenutzten Pascal Heap in die DOS Freiliste einf�gen.
;
@freepas	proc	near
;
	mov	swap_prep.pmemid,0	;e Init to no free block
					;d Init auf keinen freien Block
;>e
;	Load pointer to end of unused heap. For TP 5.x, this is
;	FreePtr (which has to be adjusted if the offset part is 0).
;	For TP 6.x, it is HeapEnd.
;	For both versions, there may be data located after this
;	pointer, so we may have to create an MCB.
;<
;>d
;	Zeiger auf Ende des unbenutzten Heap laden. F�r TP 5.x, der Zeiger
;	ist FreePtr (mu� adjustiert werden wenn der Offset-Teil 0 ist).
;	F�r TP 6.x ist es HeapEnd.
;	F�r beide Versionen k�nnen nach diesem Zeiger noch Daten folgen,
;	soda� eventuell ein MCB erzeugt werden mu�.
;<
	IF	TPAS_6
	mov	ax,word ptr HeapEnd
	mov	dx,word ptr HeapEnd+2
	ELSE
	mov	ax,word ptr FreePtr
	mov	dx,word ptr FreePtr+2
	or	ax,ax			; offset 0?
	jnz	freepok
	add	dx,1000h		; adjust segment
freepok:
	ENDIF
	mov	cl,4
	shr	ax,cl
	add	ax,dx		;e FreeList/HeapEnd segment (rounded down)
				;d Segment von Freiliste/HeapEnd (abgerundet)
	dec	ax		; MCB addr FreeList/HeapEnd segment
	mov	si,ax
;
	mov	bx,word ptr HeapPtr
	mov	dx,word ptr HeapPtr+2
	add	bx,0fh
	shr	bx,cl
	add	bx,dx		; heap end segment (rounded up)
	mov	di,bx
;
	sub	ax,bx		; free heap size
	jc	pnofree
	cmp	ax,1024		;e don't bother if less than 16k
				;d nicht freigeben wenn weniger als 16k
	jbe	pnofree
;
	mov	ax,PrefixSeg	; PSP
	sub	bx,ax		; size of space in use
	dec	ax		; MCB
	mov	es,ax
	inc	ax
;
;e	Modify base MCB to contain only the part up to HeapPtr.
;d	Den Basis-MCB modifizieren, da� er nur den Teil bis HeapPtr enth�lt.
;
	mov	cl,es:id	; save MCB ID
	mov	swap_prep.pmemid,cl
	mov	es:id,'M'
	mov	dx,es:paras
	mov	swap_prep.pmempar,dx
	mov	es:paras,bx
;
;e	Insert new, free, MCB at address HeapPtr.
;d	Neuen, als frei markierten, MCB an Adresse HeapPtr einf�gen.
;
	mov	es,di		; HeapPtr
	sub	dx,bx
	dec	dx
	mov	es:id,'M'
	mov	es:owner,0
	mov	bx,si		; HeapEnd/FreeList
	sub	bx,di		; - HeapPtr
	dec	bx		; - 1 (MCB)
	mov	es:paras,bx
;
	sub	dx,bx
	dec	dx
	jnz	frnotempty
	inc	es:paras
	mov	es:id,cl
	jmp	short pnofree
;>e
;	There is a non-empty block after HeapEnd/FreePtr. Insert an MCB
;	with Owner = PSP.
;<
;>d
;	Es existiert ein nicht leerer Block nach HeapEnd/FreePtr. Einen
;	MCB mit Owner = PSP erzeugen.
;<
frnotempty:
	mov	es,si		; HeapEnd/FreeList
	mov	es:id,cl
	mov	es:owner,ax
	mov	es:paras,dx
;
pnofree:
	ret
;
@freepas	endp
;>e
;	@restpas - restore original memory layout modified by @freepas
;<	
;>d
;	@restpas - von @freepas modifiziertes Original-Speicher-Layout 
;		   wiederherstellen
;<	
@restpas	proc	near
;
	mov	al,swap_prep.pmemid
	or	al,al
	jz	norestpas
	mov	bx,PrefixSeg
	dec	bx
	mov	es,bx
	mov	es:id,al
	mov	ax,swap_prep.pmempar
	mov	es:paras,ax
;
norestpas:
	ret
;
@restpas	endp
	ENDIF
	ENDIF
;
;--------------------------------------------------------------------------
;--------------------------------------------------------------------------
;
;
	IFDEF	PASCAL
	IFDEF	FARCALL
do_spawn	PROC	far swapping: word, execfname: dword, params: dword, envlen: word, envp: dword, stdin: dword, stdout: dword, stderr: dword
	ELSE
do_spawn	PROC	near swapping: word, execfname: dword, params: dword, envlen: word, envp: dword, stdin: dword, stdout: dword, stderr: dword
	ENDIF
	ELSE
do_spawn	PROC	uses si di,swapping: word, execfname:ptr byte,params:ptr byte,envlen:word,envp:ptr byte,stdin:ptr byte, stdout:ptr byte, stderr:ptr byte
	ENDIF
	local	datseg,pspseg,currmcb,dos33
;
	IFDEF	TC_HUGE
	mov	ax,SEG my_data
	mov	ds,ax
	ENDIF
;
	mov	datseg,ds		;e save default DS
					;d Default-DS sichern
;
	mov	dos33,0
	mov	ax,3000h		; get DOS version
	int	21h
	cmp	al,3
	jb	notdos33
	ja	isdos33
	cmp	ah,33
	jb	notdos33
isdos33:
	mov	dos33,1
notdos33:
;
	IFDEF	PASCAL
	cld
	mov	bx,prefixseg
	ELSE
	IFDEF	TC_HUGE
	mov	ax,SEG _psp
	mov	es,ax
	mov	bx,es:_psp
	ELSE
	mov	bx,_psp
	ENDIF
	ENDIF
	mov	pspseg,bx
;
;e	Check if spawn is too low in memory
;d	Pr�fen ob dieses Modul zu weit unten im Speicher liegt
;
	mov	ax,cs
	mov	dx,offset lowcode_begin
	mov	cl,4
	shr	dx,cl
	add	ax,dx			;e normalized start of this code
					;d Normalisierter Beginn des Codes
	mov	dx,keep_paras		;e the end of the modified area
					;d Ende des modifizierten Bereichs
	add	dx,bx			;e plus PSP = end paragraph
					;d plus PSP = letzer Paragraph
	cmp	ax,dx
	ja	doswap_ok	;e ok if start of code > end of low mem
				;d OK wenn Code-Beginn > Ende residenter Teil
	mov	ax,RC_TOOLOW
	ret
;
doswap_ok:
	cmp	swapping,0
	jle	method_ok
;
;e	check the swap method, to make sure prep_swap has been called
;d	Pr�fen Auslagerungsmethode um sicherzustellen da� prep_swap
;d	aufgerufen wurde.
;
	mov	al,swap_prep.swapmethod
	cmp	al,USE_EMS
	je	method_ok
	cmp	al,USE_XMS
	je	method_ok
	cmp	al,USE_FILE
	je	method_ok
	mov	ax,RC_BADPREP
	ret
;>e
;	Save the memory below the swap space.
;	We must do this before swapping, so the saved memory is
;	in the swapped out image.
;	Anything else we'd want to save on the stack or anywhere
;	else in "normal" memory also has to be saved here, any
;	modifications done to memory after the swap will be lost.
;
;	Note that the memory save is done even when not swapping,
;	because we use some of the variables in low core for
;	simplicity.
;<
;>d
;	Sichern des Speichers unterhalb des Auslagerungsbereichs.
;	Dies mu� vor dem Auslagern geschehen damit der gesicherte
;	Bereich mit im Ausgelagerten Abbild ist.
;	Auch alles Andere das auf dem Stack oder sonst im "normalen"
;	Speicher gesichert werden soll mu� hier gesichert werden,
;	da alle Modifikationen des Speichers die nach dem Auslagern
;	erfolgen verloren gehen.
;
;	Das Sichern des Speichers ist auch notwendig wenn keine Auslagerung
;	stattfindet, da Teile des sonst residenten Bereichs im unteren
;	Speicher in jedem Fall verwendet werden.
;<
method_ok:
	mov	es,datseg
	mov	ds,pspseg		;e DS points to PSP
					;d DS zeigt auf PSP
	mov	si,5ch
	mov	di,offset save_dat
	mov	cx,savespace / 2	;e NOTE: savespace is always even
					;d HINWEIS: savespace ist stets gerade
	rep movsw
;
	mov	ds,datseg
	IFDEF	PASCAL
	IF	PAS_FREE
	call	@freepas
	ENDIF
	ENDIF
;
	mov	ax,swapping
	cmp	ax,0
	jg	begin_swap
;>e
;	not swapping, prep_swap wasn't called. Init those variables in
;  	the 'swap_prep' block we need in any case.
;<
;>d
;	Keine Auslagerung, prep_swap wurde nicht aufgerufen. Die Variablen
;	im 'swap_prep'-Block initialisieren die in jedem Fall gebraucht werden.
;<
	mov	swap_prep.swapmethod,al
	je	no_reduce
;
	mov	ax,pspseg
	dec	ax
	mov	swap_prep.psp_mcb,ax
	mov	swap_prep.first_mcb,ax
	inc	ax
	mov	es,ax
	mov	bx,es:psp_envptr
	mov	swap_prep.env_mcb,bx
	mov	swap_prep.noswap_mcb,0
	cmp	envlen,0
	jne	swp_can_swap_env
	mov	swap_prep.noswap_mcb,bx
;
swp_can_swap_env:
	IF	NOT NO_INHERIT
	mov	swap_prep.noswap_mcb2,0
	cmp	dos33,0
	je	swp_no_exthandle
	mov	ax,es:psp_handleptrs
	mov	bx,es
	cmp	ax,bx
	je	swp_no_exthandle
	dec	ax
	mov	swap_prep.noswap_mcb2,ax
;
swp_no_exthandle:
	ENDIF
	xor	bx,bx
	mov	es,bx
	mov	ah,52h			; get list of lists
	int	21h
	mov	ax,es
	or	ax,bx
	jz	no_reduce
	mov	es,es:[bx-2]		; first MCB
	cmp	es:id,4dh		; normal ID?
	jne	no_reduce
	mov	swap_prep.first_mcb,es
;
no_reduce:
	jmp	no_swap1
;
;e	set up first block descriptor
;d	Ersten Block-Deskriptor aufsetzen
;
begin_swap:
	mov	ax,swap_prep.first_mcb
	mov	currmcb,ax
	mov	es,swap_prep.psp_mcb	;e let ES point to base MCB
					;d ES zeigt auf Basis-MCB
	mov	ax,es:paras
	mov	currdesc.msize,ax
	sub	ax,keep_paras
	mov	currdesc.swsize,ax
	mov	currdesc.addr,es
	mov	currdesc.swoffset,swapbeg + 16
;e		NOTE: swapbeg is 1 para higher when seen from MCB
;d		HINWEIS: swapbeg ist 1 Paragraph h�her vom MCB aus gesehen
	mov	ax,swap_prep.total_mcbs
	mov	currdesc.num_follow,ax
;
;e	init other vars
;d	andere Variablen initialisieren
;
	mov	xmsctl.destlo,0
	mov	xmsctl.desthi,0
	mov	ems_curpage,0
	mov	ems_curoff,ems_parasize
;>e
;	Do the swapping. Each MCB block (except the last) has an 
;	"mcbdesc" structure appended that gives location and size 
;	of the next MCB.
;<
;>d
;	Auslagerung ausf�hren. An jeden MCB-Block (au�er den letzten)
;	wird eine "mcbdesc" Struktur zur Beschreibung von Adresse und
;	Gr��e des n�chsten Blocks angef�gt.
;<
swapout_main:
	cmp	currdesc.num_follow,0	;e next block?
					;d Gibt es einen n�chsten?
	je	swapout_no_next		;e ok if not
					;d OK wenn nein
;>e
;	There is another MCB block to be saved. So we don't have
;	to do two calls to the save routine with complicated
;	parameters, we set up the next MCB descriptor beforehand.
;	Walk the MCB chain starting at the current MCB to find
;	the next one belonging to this process.
;<
;>d
;	Es gibt einen weiteren auszulagernden MCB-Block. Um die 
;	Auslagerungsroutine nicht getrennt f�r den MCB-Deskriptor
;	aufrufen zu m�ssen (mit einer entsprechend aufwendigeren
;	Parameterversorgung), wird der Deskriptor f�r den Folgeblock
;	vor Aufruf erstellt.
;	Dazu mu� die MCB-Kette, beginnend beim derzeitigen Block,
;	durchgegangen werden um den n�chsten zum Proze� geh�renden
;	Block zu finden.
;<
	mov	ax,currmcb
	mov	bx,pspseg
	mov	cx,swap_prep.psp_mcb
	mov	dx,swap_prep.noswap_mcb
;
swm_mcb_walk:
	mov	es,ax
	cmp	ax,cx
	je	swm_next_mcb
	cmp	ax,dx
	je	swm_next_mcb
	IF	NOT NO_INHERIT
	cmp	ax,swap_prep.noswap_mcb2
	je	swm_next_mcb
	ENDIF
;
	cmp	bx,es:owner		;e our process?
					;d Dieser Proze�?
	je	swm_mcb_found		;e found it if yes
					;d gefunden wenn ja
;
swm_next_mcb:
	cmp	es:id,4dh		;e normal block?
					;d Normaler Block?
	jne	swm_mcb_error		;e error if end of chain
					;d Fehler wenn Ende der Kette
	add	ax,es:paras		; start + length
	inc	ax			; next MCB
	jmp	swm_mcb_walk
;
;e	MCB found, set up an mcbdesc in the "nextmcb" structure
;d	MCB gefunden, aufsetzen mcbdesc in der "nextmcb" Struktur
;
swm_mcb_found:
	mov	nextmcb.addr,es
	mov	ax,es:paras		;e get number of paragraphs
					;d Anzahl Paragraphen
	mov	nextmcb.msize,ax	;e and save
					;d sichern
	inc	ax
	mov	nextmcb.swsize,ax
	mov	bx,es
	add	bx,ax
	mov	currmcb,bx
	mov	nextmcb.swoffset,0
	mov	ax,currdesc.num_follow
	dec	ax
	mov	nextmcb.num_follow,ax
;
swapout_no_next:
	cmp	swap_prep.swapmethod,USE_EMS
	je	swm_ems
	cmp	swap_prep.swapmethod,USE_XMS
	je	swm_xms
	call	swapout_file
	jmp	short swm_next
;
swm_ems:
	call	swapout_ems
	jmp	short swm_next
;
swm_xms:
	call	swapout_xms
;
swm_next:
	jnz	swapout_error
	cmp	currdesc.num_follow,0
	je	swapout_complete
;>e
;	next MCB exists, copy the "nextmcb" descriptor into
;	currdesc, and loop.
;<
;>d
;	Es gibt einen Folgeblock, der "nextmcb" Deskriptor wird in den
;	"currdesc" Deskriptor kopiert, und das ganze noch mal von vorn.
;<
	mov	es,datseg
	mov	si,offset nextmcb
	mov	di,offset currdesc
	mov	cx,TYPE mcbdesc
	rep movsb
	jmp	swapout_main
;
;
swm_mcb_error:
	mov	ax,RC_MCBERROR
;
swapout_kill:
	cmp	swapping,0
	jl	swapout_error
	push	ax
	cmp	swap_prep.swapmethod,USE_FILE
	je	swm_mcberr_file
	cmp	swap_prep.swapmethod,USE_EMS
	je	swm_mcberr_ems
;
	mov	ah,0ah			;e release XMS frame on error
					;d Bei Fehler XMS-Block freigeben
	mov	dx,swap_prep.handle	; XMS handle
	call	swap_prep.xmm
	pop	ax
	jmp	short swapout_error
;
swm_mcberr_ems:
	mov	dx,swap_prep.handle	; EMS handle
	mov	ah,45h			;e release EMS pages on error
					;d Bei Fehler EMS freigeben
	int	EMM_INT
	pop	ax
	jmp	short swapout_error
;
swm_mcberr_file:
	mov	bx,swap_prep.handle
	cmp	bx,-1
	je	swm_noclose
	mov	ah,3eh			; close file
	int	21h
swm_noclose:
	mov	dx,offset swap_prep.swapfilename
	mov	ah,41h			; delete file
	int	21h
	pop	ax
;
swapout_error:
	IFDEF	PASCAL
	IF	PAS_FREE
	call	@restpas
	ENDIF
	ENDIF
	ret
;
;>e
;	Swapout complete. Close the handle (EMS/file only),
;	then set up low memory.
;<
;>d
;	Auslagerung abgeschlossen. Die Datei bzw. die EMS-Seite wird
;	geschlossen, und der residente Bereich wird initialisiert.
;<
swapout_complete:
	cmp	swap_prep.swapmethod,USE_FILE
	jne	swoc_nofile
;
;e	File swap: Close the swap file to make the handle available
;d	Auslagerung auf Datei: Datei schlie�en um den Handle freizumachen
;
	mov	bx,swap_prep.handle
	mov	swap_prep.handle,-1
	mov	ah,3eh
	int	21h			; close file
	mov	si,offset swapin_file
	jnc	swoc_ready
	mov	ax,RC_SWAPERROR
	jmp	swapout_kill
;
swoc_nofile:
	cmp	swap_prep.swapmethod,USE_EMS
	jne	swoc_xms
;
;e	EMS: Unmap page
;d	EMS: Seite unzug�nglich machen
;
	mov	ax,4400h
	mov	bx,-1
	mov	dx,swap_prep.handle
	int	EMM_INT
	mov	si,offset swapin_ems
	jmp	short swoc_ready
;
swoc_xms:
	mov	si,offset swapin_xms
	jmp	short swoc_ready
;
no_swap1:
	mov	si,offset swapin_none
;	
;e	Copy the appropriate swap-in routine to low memory.
;d	Kopieren der der Auslagerungsmethode entsprechenden Routine
;d	in den residenten Teil.
;
swoc_ready:
	mov	es,pspseg
	mov	cx,swap_codelen / 2
	mov	di,codebeg + base_length
	push	ds
	mov	ax,cs
	mov	ds,ax
	rep movsw
;>e
;	And while we're at it, copy the MCB allocation routine (which
;	also includes the initial MCB release and exec call) down.
;<
;>d
;	Au�erdem die MCB-Allozierungsroutine und den EXEC-Aufruf
;	nach unten kopieren.
;<
	mov	cx,base_length / 2
	mov	di,param_len
	mov	si,offset lowcode_begin
	rep movsw
;
	pop	ds
	mov	bx,es
	dec	bx
	mov	es,bx		;e let ES point to base MCB
				;d ES zeigt jetzt auf den Basisblock
;>e
;	Again set up the base MCB descriptor, and copy it as well as
;	the variables set up by prep_swap to low memory.
;	This isn't too useful if we're not swapping, but it doesn't
;	hurt, either. The only variable used when not swapping is
;	lprep.swapmethod.
;<
;>d
;	Der Deskriptor f�r den Basisblock wird erneut aufgesetzt und,
;	zusammen mit dem Variablenblock der durch prep_swap initialisiert
;	wurde, nach unten kopieren.
;	Wenn gar nicht ausgelagert wird, ist dies eigentlich nicht nicht
;	notwendig, es schadet aber auch nicht. Die einzige Variable
;	die in diesem Fall ben�tigt wird ist lprep.swapmethod.
;<
	mov	ax,es:paras
	mov	currdesc.msize,ax
	sub	ax,keep_paras
	mov	currdesc.swsize,ax
	mov	currdesc.addr,es
	mov	currdesc.swoffset,swapbeg + 16
	mov	ax,swap_prep.total_mcbs
	mov	currdesc.num_follow,ax
;
	mov	es,pspseg		;e ES points to PSP again
					;d ES zeigt wieder auf PSP
;
	mov	cx,TYPE prep_block
	mov	si,offset swap_prep
	mov	di,lprep
	rep movsb
	mov	cx,TYPE mcbdesc
	mov	si,offset currdesc
	mov	di,lcurrdesc
	rep movsb
;
;e	now set up other variables in low core
;d	Nun werden die weiteren Variablen im residenten Tail initialisiert.
;
	mov	ds,pspseg
	mov	ds:cgetmcb,getmcboff + codebeg
	mov	ds:eretcode,0
	mov	ds:retflags,0
;
;>e
;	If 'NO_INHERIT' is nonzero, save the entries of the 
;	handle table, and set the last 15 to 0ffh (unused).
;<
;>d
;	Wenn 'NO_INHERIT' nicht 0 ist, Retten der Handle-Tabelle
;	und Setzen der letzten 15 Eintr�ge auf 0ffh (unbenutzt).
;<
	IF	NO_INHERIT
	mov	si,psp_handletab
	mov	di,lhandlesave
	mov	cx,10
	rep movsw
	mov	si,psp_handlenum	;e Length of handle table
					;d L�nge Handle-Tabelle
	mov	ax,[si]
	stosw
	mov	word ptr [si],20	;e set to default to be safe
					;d Auf Standardwert setzten
	add	si,2
	lodsw				;e Handle table pointer
					;d Zeiger auf Handle-Tabelle
	mov	bx,ax
	stosw
	lodsw
	stosw
	cmp	dos33,0
	je	no_handlecopy
	cmp	ax,pspseg
	jne	copy_handles
	cmp	bx,psp_handletab
	je	no_handlecopy
;>e
;	if the handle table pointer in the PSP does not point to
;	the default PSP location, copy the first five entries from
;	this table into the PSP - but only if we have DOS >= 3.3.
;<
;>d
;	Wenn der Handle-Tabellen-Pointer im PSP nicht auf den
;	Standard-Platz im PSP zeigt, die ersten f�nf Eintr�ge
;	aus dieser Tabelle in den PSP kopieren - aber nur bei
;	einer DOS-Version >= 3.3.
;<
copy_handles:
	mov	ds,ax
	mov	si,bx
	mov	di,psp_handletab
	mov	es:psp_handleptro,di
	mov	es:psp_handleptrs,es
	movsw
	movsw
	movsb
;
no_handlecopy:
	mov	di,psp_handletab+5
	mov	ax,0ffffh
	stosb
	mov	cx,7
	rep stosw
;
	ENDIF
;
;e	Handle Redirection
;d	Dateiumleitung behandeln
;
	IF	REDIRECT
	mov	es,pspseg
	mov	di,lstdinsav
	mov	ax,-1
	stosw
	stosw
	stosw
	mov	di,lstdinsav
	xor	cx,cx
	IF	ptrsize
	lds	si,stdin
	mov	ax,ds
	or	ax,si
	ELSE
	mov	si,stdin
	mov	ds,datseg
	or	si,si
	ENDIF
	call	@redirect
	jc	failed_redir
	inc	cx
	IF	ptrsize
	lds	si,stdout
	mov	ax,ds
	or	ax,si
	ELSE
	mov	si,stdout
	or	si,si
	ENDIF
	call	@redirect
	jc	failed_redir
	inc	cx
	IF	ptrsize
	lds	si,stderr
	mov	ax,ds
	or	ax,si
	ELSE
	mov	si,stderr
	or	si,si
	ENDIF
	call	@redirect
	jnc	redir_complete
;
failed_redir:
	push	ax
;
;e	restore handle table and pointer
;d	Wiederherstellen Handle-Tabelle und Pointer
;
	mov	ds,pspseg
	mov	si,lstdinsav
	xor	cx,cx
;
redirclose:
	lodsw
	cmp	ax,-1
	je	redclosenext
	mov	bx,ax
	mov	ah,46h
	int	21h
;
redclosenext:
	inc	cx
	cmp	cx,3
	jb	redirclose
;
	IF	NO_INHERIT
	mov	ds,pspseg
	mov	es,pspseg
	mov	si,lhandlesave
	mov	di,psp_handletab
	mov	cx,10
	rep movsw
	mov	di,psp_handlenum
	movsw
	movsw
	movsw
	ENDIF
;
;e	Restore overwritten part of program
;d	Den �berschriebenen Teil des Programms wiederherstellen
;
	mov	ds,datseg
	mov	es,pspseg
	mov	si,offset save_dat
	mov	di,5ch
	mov	cx,savespace
	rep movsb
;
	pop	ax
	mov	ah,RC_REDIRFAIL SHR 8
	jmp	swapout_kill
;
redir_complete:
	IF	NO_INHERIT
	mov	ds,pspseg
	mov	es,pspseg
	mov	si,psp_handletab+5
	mov	di,lredirsav
	mov	cx,3
	rep movsw
	mov	di,psp_handletab+5
	mov	cx,3
	mov	ax,0ffffh
	rep stosw
	ENDIF
	ENDIF
;
;e	Prepare exec parameter block
;d	Parameterblock f�r EXEC-Aufruf vorbereiten
;
	mov	ax,es
	mov	es:expar.fcb1seg,ax
	mov	es:expar.fcb2seg,ax
	mov	es:expar.pparseg,ax
	mov	es:expar.envseg,0
;>e
;	The 'zero' word is located at 80h in the PSP, the start of
;	the command line. So as not to confuse MCB walking programs,
;	a command line length of zero is inserted here.
;<
;>d
;	Das 'zero'-Wort ist an Adresse 80h im PSP, dem Beginn der
;	Kommandozeile. Um Programme die die MCB-Kette abarbeiten
;	und dabei m�glicherweise auf diese Kommandozeile zugreifen
;	nicht zu verwirren, wird eine leere Kommandozeile eingef�gt.
;<
	mov	es:zero,0d00h		;e 00h,0dh = empty command line
					;d 00h,0dh = Leere Kommandozeile
;
;e	Init default fcb's by parsing parameter string
;d	Default FCB-Bl�cke aus dem Parameter-String f�llen
;
	IF	ptrsize
	lds	si,params
	ELSE
	mov	si,params
	mov	ds,datseg
	ENDIF
	IFDEF	PASCAL
	inc	si			;e skip length byte
					;d L�ngenbyte �berspringen
	ENDIF
	push	si
	mov	di,xfcb1
	mov	es:expar.fcb1,di
	push	di
	mov	cx,16
	xor	ax,ax
	rep stosw			;e init both fcb's to 0
					;d Beide FCBs mit 0 vorbesetzen
	pop	di
	mov	ax,2901h
	int	21h
	mov	di,xfcb2
	mov	es:expar.fcb2,di
	mov	ax,2901h
	int	21h
	pop	si
;
;e	move command tail string into low core
;d	Kommandozeile in residenten Teil transferieren
;
	mov	di,progpars
	mov	es:expar.ppar,di
	xor	cx,cx
	inc	di
cmdcpy:
	lodsb
	or	al,al
	jz	cmdcpy_end
	stosb
	inc	cx
	jmp	cmdcpy
;
cmdcpy_end:
	mov	al,0dh
	stosb
	mov	es:progpars,cl
;
;e	move filename string into low core
;d	Dateinamen in residenten Teil transferieren
;
	IF	ptrsize
	lds	si,execfname
	ELSE
	mov	si,execfname
	ENDIF
	IFDEF	PASCAL
	inc	si
	ENDIF
	mov	di,filename
fncpy:
	lodsb
	stosb
	or	al,al
	jnz	fncpy
;
;e	Setup environment copy
;d	Umgebungsvariablenblock-Kopie aufsetzen
;
	mov	bx,keep_paras		;e paras to keep
					;d Residente Paragraphen
	mov	cx,envlen		;e environment size
					;d Gr��e Umgebungsvariablen
	jcxz	no_environ		;e go jump if no environment
					;d Fertig wenn keine Umgebung
	cmp	swapping,0
	jne	do_envcopy
;>e
;	Not swapping, use the environment pointer directly.
;	Note that the environment copy must be paragraph aligned.
;<
;>d
;	Keine Auslagerung, der Umgebungs-Zeiger kann direkt verwendet
;	werden. Dazu mu� die Kopie der Umgebungsvariablen auf Paragraphen-
;	Grenze adjustiert sein.
;<
	IF	ptrsize
	mov	ax,word ptr (envp)+2
	mov	bx,word ptr (envp)
	ELSE
	mov	ax,ds
	mov	bx,envp
	ENDIF
	add	bx,15			;e make sure it's paragraph aligned
					;d auf Paragraphengrenze bringen
	mov	cl,4
	shr	bx,cl			;e and convert to segment addr
					;d in Segment-Adresse konvertieren
	add	ax,bx
	mov	es:expar.envseg,ax	;e new environment segment
					;d Neues Umgebungs-Segment
	xor	cx,cx			;e mark no copy
					;d Markieren da� keine Kopie n�tig
	xor	bx,bx			;e and no shrink
					;d und keine Speicherreduzierung
	jmp	short no_environ
;>e
;	Swapping or EXECing without return. Set up the pointers for
;	an environment copy (we can't do the copy yet, it might overwrite
;	this code).
;<
;>d
;	Auslagerung, oder EXEC ohne R�ckkehr. Die Zeiger f�r das
;	kopieren der Umgebungsvariablen aufsetzen. Der Block darf
;	jetzt noch nicht kopiert werden, da dies eventuell den gerade
;	ausgef�hrten Code �berschreiben k�nnte.
;<
do_envcopy:
	inc	cx
	shr	cx,1			;e words to copy
					;d Zu kopierende Worte
	mov	ax,cx			;e convert envsize to paras
					;d in Paragraphen konvertieren
	add	ax,7
	shr	ax,1
	shr	ax,1
	shr	ax,1
	add	bx,ax			;e add envsize to paras to keep
					;d Gr��e zu residenter addieren
	IF	ptrsize
	lds	si,envp
	ELSE
	mov	si,envp
	ENDIF
;
	mov	ax,es			;e low core segment
					;d Segmentadresse residenter Teil
	add	ax,keep_paras		;e plus fixed paras
					;d plus residente Paragraphen
	mov	es:expar.envseg,ax	;e = new environment segment
					;d = neues Umgebungs-Segment
;
;e	Save stack regs, switch to local stack
;d	Sichern Stack-Register, Umschalten auf lokalen Stack
;
no_environ:
	mov	es:save_ss,ss
	mov	es:save_sp,sp
	mov	ax,es
	cli
	mov	ss,ax
	mov	sp,mystack
	sti
;
	push	cx			; save env length
	push	si			; save env pointer
	push	ds			; save env segment
;
;e	save and patch INT0 (division by zero) vector
;d	Sichern und �berschreiben INT0 (division by zero) Vektor
;
	xor	ax,ax
	mov	ds,ax
	mov	ax,word ptr ds:0
	mov	es:div0_off,ax
	mov	ax,word ptr ds:2
	mov	es:div0_seg,ax
	mov	word ptr ds:0,codebeg + iretoff
	mov	word ptr ds:2,es
;
	pop	ds			; pop environment segment
	pop	si			; pop environment offset
	pop	cx			; pop environment length
	mov	di,swapbeg		;e environment destination
					;d Zieladresse Umgebungsblock
;
;e	Push return address on local stack
;d	R�ckkehradresse auf lokalen Stack bringen
;
	push	cs			;e push return segment
					;d R�ckkehr-Segment pushen
	mov	ax,offset exec_cont
	push	ax			;e push return offset
					;d R�ckkehr-Offset pushen
	mov	es:spx,sp		;e save stack pointer
					;d Stack-Zeiger sichern
;
;e	Goto low core code
;d	In den residenten Teil springen
;
	push	es			;e push entry segment
					;d Einsprungssegment pushen
        mov	ax,codebeg + doexec_entry
        push	ax			;e push entry offset
					;d Einsprungsoffset pushen
;e	ret	far			; can't use RET here because
;d	ret	far			; RET kann hier wegen der .model-
	db	0cbh			;e of .model
					;d Direktive nicht verwendet werden
;
;----------------------------------------------------------------
;>e
;	Low core code will return to this location, with DS set to
;	the PSP segment.
;<
;>d
;	Der residente Teil kehrt hierher zur�ck, mit DS = PSP-Segment.
;<
exec_cont:
	push	ds
	pop	es
	cli
	mov	ss,ds:save_ss		;e reload stack
					;d Stack zur�ckladen
	mov	sp,ds:save_sp
	sti
;
;e	restore handle table and pointer
;d	Wiederherstellen Handle-Tabelle und Pointer
;
	IF	NO_INHERIT
	mov	si,lhandlesave
	mov	di,psp_handletab
	mov	cx,10
	rep movsw
	mov	di,psp_handlenum
	movsw
	movsw
	movsw
	ENDIF
;
;e	restore INT0 (division by zero) vector
;d	INT0 (division by zero) Vektor wiederherstellen
;
	xor	cx,cx
	mov	ds,cx
	mov	cx,es:div0_off
	mov	word ptr ds:0,cx
	mov	cx,es:div0_seg
	mov	word ptr ds:2,cx
;
	mov	ds,datseg
;
	IFDEF	PASCAL
	IF	PAS_FREE
	push	es
	call	@restpas
	pop	es
	ENDIF
	ENDIF
;
	mov	ax,es:eretcode
	mov	bx,es:retflags
;
;e	Restore overwritten part of program
;d	Den �berschriebenen Teil des Programms wiederherstellen
;
	mov	si,offset save_dat
	mov	di,5ch
	mov	cx,savespace
	rep movsb
;
	test	bx,1			;e carry set?
					;d Carry-Flag gesetzt?
	jnz	exec_fault		;e return EXEC error code if fault
					;d EXEC Fehler-code liefern wenn ja
	mov	ah,4dh			;e else get program return code
					;d Sonst Programm-R�ckgabewert holen
	int	21h
	ret
;
exec_fault:
	mov	ah,3			;e return error as 03xx
					;d EXEC-Fehler als 03xx liefern
	ret
;	
do_spawn	ENDP
;
;----------------------------------------------------------------------------
;----------------------------------------------------------------------------
;
emm_name	db	'EMMXXXX0'
;>e
;	prep_swap - prepare for swapping.
;
;	This routine checks all parameters necessary for swapping,
;	and attempts to set up the swap-out area in EMS/XMS, or on file.
;	In detail:
;
;	     1) Check whether the do_spawn routine is located
;		too low in memory, so it would get overwritten.
;		If this is true, return an error code (-2).
;
;	     2) Walk the memory control block chain, adding up the
;		paragraphs in all blocks assigned to this process.
;
;	     3) Check EMS (if the method parameter allows EMS):
;		- is an EMS driver installed?
;		- are sufficient EMS pages available?
;		if all goes well, the EMS pages are allocated, and the
;		routine returns success (1).
;
;	     4) Check XMS (if the method parameter allows XMS):
;		- is an XMS driver installed?
;		- is a sufficient XMS block available?
;		if all goes well, the XMS block is allocated, and the
;		routine returns success (2).
;
;	     5) Check file swap (if the method parameter allows it):
;		- try to create the file
;		- pre-allocate the file space needed by seeking to the end
;		  and writing a byte.
;		If the file can be written, the routine returns success (4).
;
;	     6) Return an error code (-1).
;<
;>d
;	prep_swap - Auslagerung vorbereiten.
;
;	Diese Routine pr�ft die zur Auslagerung n�tigen Parameter,
;	und versucht den Auslagerungsbereich in EMS, XMS, oder auf Datei,
;	vorzubereiten.
;	Im einzelnen:
;
;	     1) Pr�fen ob die do_spawn-Routine so niedrig liegt da�
;		sie durch die Auslagerung �berschrieben w�rde.
;		Ist dies der Fall, Fehler (-2) liefern.
;
;	     2) Die MCB-Kette durchlaufen, dabei die L�nge in Paragraphen
;		aller Bl�cke addieren die zu diesem Proze� geh�ren.
;
;	     3) EMS pr�fen (wenn der method Parameter EMS erlaubt):
;		- ist ein EMS Treiber installiert?
;		- sind gen�gend EMS-Seiten verf�gbar?
;		Wenn dies erf�llt ist, werden die EMS-Seiten alloziert,
;		und die Routine liefert einen Erfolgscode (1).
;
;	     4) XMS pr�fen (wenn der method Parameter XMS erlaubt):
;		- ist ein XMS Treiber installiert?
;		- ist ein ausreichend gro�er XMS-Bereich verf�gbar?
;		Wenn dies erf�llt ist, wird der XMS-Bereich alloziert,
;		und die Routine liefert einen Erfolgscode (2).
;
;	     5) Dateiauslagerung pr�fen (wenn der method Parameter es erlaubt):
;		- die Datei erzeugen,
;		- den ben�tigten Platz auf Datei pr�-allozieren indem
;		  an das Ende positioniert und ein Byte geschrieben wird.
;		Konnte die Datei geschrieben werden, liefert die Routine
;		einen Erfolgscode (4).
;
;	     6) Fehlercode liefern (-1).
;<
	IFDEF	PASCAL
	IFDEF	FARCALL
prep_swap	PROC	far pmethod: word, swapfname: dword
	ELSE
prep_swap	PROC	near pmethod: word, swapfname: dword
	ENDIF
	ELSE
prep_swap	PROC	uses si di,pmethod:word,swapfname:ptr byte
	ENDIF
	LOCAL	totparas: word
;
	IFDEF	TC_HUGE
	mov	ax,SEG my_data
	mov	ds,ax
	ENDIF
;
	IFDEF	PASCAL
	cld
	mov	ax,prefixseg
	ELSE
	IFDEF	TC_HUGE
	mov	ax,SEG _psp
	mov	es,ax
	mov	ax,es:_psp
	ELSE
	mov	ax,_psp
	ENDIF
	ENDIF
;
	dec	ax
	mov	swap_prep.psp_mcb,ax
	mov	swap_prep.first_mcb,ax	;e init first MCB to PSP
					;d ersten MCB auf PSP initialisieren
;
;e	Make a copy of the environment pointer in the PSP
;d	Eine Kopie vom Umgebungsvariablenblock-Pointer des PSP sichern
;
	inc	ax
	mov	es,ax
	mov	bx,es:psp_envptr
	dec	bx
	mov	swap_prep.env_mcb,bx
	mov	swap_prep.noswap_mcb,0
	test	pmethod,DONT_SWAP_ENV
	jz	can_swap_env
	mov	swap_prep.noswap_mcb,bx
;
can_swap_env:
	IF	NOT NO_INHERIT
	push	ax
	mov	swap_prep.noswap_mcb2,0
	mov	ax,3000h
	int	21h
	cmp	al,3
	jb	no_exthandle
	ja	check_exthandle
	cmp	ah,33
	jb	no_exthandle
;
check_exthandle:
	mov	ax,es:psp_handleptrs
	mov	bx,es
	cmp	ax,bx
	je	no_exthandle
	dec	ax
	mov	swap_prep.noswap_mcb2,ax
;
no_exthandle:
	pop	ax
	ENDIF
;
;e	Check if spawn is too low in memory
;d	Pr�fen ob dieses Modul zu weit unten im Speicher liegt
;
	mov	bx,cs
	mov	dx,offset lowcode_begin
	mov	cl,4
	shr	dx,cl
	add	bx,dx			;e normalized start of this code
					;d Normalisierter Beginn des Codes
	mov	dx,keep_paras		;e the end of the modified area
					;d Ende des modifizierten Bereichs
	add	dx,ax			;e plus PSP = end paragraph
					;d plus PSP = letzer Paragraph
	cmp	bx,dx
	ja	prepswap_ok	;e ok if start of code > end of low mem
				;d OK wenn Code-Beginn > Ende residenter Teil
	mov	ax,-2
	mov	swap_prep.swapmethod,al
	ret
;>e
;	Walk the chain of memory blocks, adding up the paragraphs
;	in all blocks belonging to this process.
;	We try to find the first MCB by getting DOS's "list of lists",
;	and fetching the word at offset -2 of the returned address.
;	If this fails, we use our PSP as the starting point.
;<
;>d
;	Die Speicherbl�cke durchgehen und die Gr��e aller zu diesem
;	Proze� geh�renden Bl�cke zusammenz�hlen.
;	Der Anfang der MCB-Liste wird aus der DOS "list of lists"
;	abgeleitet. Das Wort an Offset -2 der gelieferten Adresse
;	enth�lt die erste MCB-Adresse. Falls dies fehlschl�gt, wird
;	der PSP als Ausgangspunkt benutzt.
;<
prepswap_ok:
	IFDEF	PASCAL
	IF	PAS_FREE
	call	@freepas
	ENDIF
	ENDIF
	xor	bx,bx
	mov	es,bx
	mov	ah,52h			; get list of lists
	int	21h
	mov	ax,es
	or	ax,bx
	jz	prep_no_first
	mov	es,es:[bx-2]		; first MCB
	cmp	es:id,4dh		; normal ID?
	jne	prep_no_first
	mov	swap_prep.first_mcb,es
;
prep_no_first:
	mov	es,swap_prep.psp_mcb	;e ES points to base MCB
					;d ES zeigt auf Basis-Block
	mov	cx,es			;e save this value
					;d diesen Wert sichern
	mov	bx,es:owner		;e the current process
					;d Der aktuelle Proze�
	mov	dx,es:paras		;e memory size in the base block
					;d Speichergr��e des Basisblocks
	sub	dx,keep_paras		;e minus resident paragraphs
					;d Abz�glich residente Paragraphen
	mov	si,0			;e number of MCBs except base
					;d Z�hler f�r MCBs au�er Basis
	mov	di,swap_prep.noswap_mcb
	mov	ax,swap_prep.first_mcb
	mov	swap_prep.first_mcb,0
;
prep_mcb_walk:
	mov	es,ax
	cmp	ax,cx			;e base block?
					;d Basisblock?
	je	prep_walk_upd_first	;e then don't count again, but update first
					;d dann nicht nochmal z�hlen, aber first setzen
	cmp	ax,di			;e Non-swap MCB?
	je	prep_walk_next		;e then don't count
					;d dann nicht z�hlen
	IF	NOT NO_INHERIT
	cmp	ax,swap_prep.noswap_mcb2
	je	prep_walk_next
	ENDIF
;
	cmp	bx,es:owner		;e our process?
					;d aktueller Proze�?
	jne	prep_walk_next		;e next if not
					;d n�chsten wenn nein
	inc	si
	mov	ax,es:paras		;e else get number of paragraphs
					;d sonst Gr��e in Paragraphen laden
	add	ax,2			; + 1 for descriptor + 1 for MCB
	add	dx,ax			;e total number of paras
					;d Gesamtzahl Paragraphen
prep_walk_upd_first:
	cmp	swap_prep.first_mcb,0
	jne	prep_walk_next
	mov	swap_prep.first_mcb,es
;
prep_walk_next:
	cmp	es:id,4dh		;e normal block?
					;d normaler Block?
	jne	prep_mcb_ready		;e ready if end of chain
					;d Fertig wenn ende der Kette
	mov	ax,es
	add	ax,es:paras		; start + length
	inc	ax			; next MCB
	jmp	prep_mcb_walk
;
prep_mcb_ready:
	mov	totparas,dx
	mov	swap_prep.total_mcbs,si
;
	IFDEF	PASCAL
	IF	PAS_FREE
	call	@restpas
	ENDIF
	ENDIF
;
	test	pmethod,XMS_FIRST
	jnz	check_xms
;
;e	Check for EMS swap
;d	EMS-auslagerung pr�fen
;
check_ems:
	test	pmethod,USE_EMS
	jz	prep_no_ems
;
	push	ds
	mov	al,EMM_INT
	mov	ah,35h
	int	21h			;e get EMM int vector
					;d EMM-Interrupt Vektor laden
	mov	ax,cs
	mov	ds,ax
	mov	si,offset emm_name
	mov	di,10
	mov	cx,8
	repz cmpsb			;e EMM name present?
					;d ist der EMM-Name vorhanden?
	pop	ds
	jnz	prep_no_ems
;
	mov	ah,40h			;e get EMS status
					;d EMS-Status abfragen
	int	EMM_INT
	or	ah,ah			; EMS ok?
	jnz	prep_no_ems
;
	mov	ah,46h			;e get EMS version
					;d EMS-Version abfragen
	int	EMM_INT
	or	ah,ah			; AH must be 0
	jnz	prep_no_ems
;
	cmp	al,30h			; >= version 3.0?
	jb	prep_no_ems
;
	mov	ah,41h			;e Get page frame address
					;d EMS-Frame-Adresse holen
	int	EMM_INT
	or	ah,ah
	jnz	prep_no_ems
;
;e	EMS present, try to allocate pages
;d	EMS vorhanden, versuche Seiten zu allozieren
;
	mov	swap_prep.ems_pageframe,bx
	mov	bx,totparas
	add	bx,ems_paramask
	mov	cl,ems_shift
	shr	bx,cl
	mov	ah,43h			; allocate handle and pages
	int	EMM_INT
	or	ah,ah			;e success?
					;d erfolgreich?
	jnz	prep_no_ems
;
;e	EMS pages allocated, swap to EMS
;d	EMS-Seiten alloziert, auslagern auf EMS
;
	mov	swap_prep.handle,dx
	mov	ax,USE_EMS
	mov	swap_prep.swapmethod,al
	ret
;
;e	No EMS allowed, or EMS not present/full. Try XMS.
;d	EMS nicht erlaubt, oder EMS nicht vorhanden/voll. XMS versuchen.
;
prep_no_ems:
	test	pmethod,XMS_FIRST
	jnz	check_file		;e don't try again
					;d nicht nochmal versuchen
;
check_xms:
	test	pmethod,USE_XMS
	jz	prep_no_xms
;
	mov	ax,4300h		;e check if XMM driver present
					;d pr�fen ob XMM-Treiber vorhanden
	int	2fh
	cmp	al,80h			;e is XMM installed?
					;d ist XMM installiert?
	jne	prep_no_xms
	mov	ax,4310h		;e get XMM entrypoint
					;d XMM-Einsprungadresse holen
	int	2fh
	mov	word ptr swap_prep.xmm,bx	;e save entry address
					;d Einsprungadresse sichern
	mov	word ptr swap_prep.xmm+2,es
;
	mov	dx,totparas
	add	dx,xms_paramask		;e round to nearest multiple of 1k
					;d Auf volle 1k aufrunden
	mov	cl,xms_shift
	shr	dx,cl			;e convert to k
					;d konvertiern in k
	mov	ah,9			;e allocate extended memory block
					;d Extended memory block allozieren
	call	swap_prep.xmm
	or	ax,ax
	jz	prep_no_xms
;
;e	XMS block allocated, swap to XMS
;d	XMS-Block alloziert, Auslagern auf XMS.
;
	mov	swap_prep.handle,dx
	mov	ax,USE_XMS
	mov	swap_prep.swapmethod,al
	ret
;
;e	No XMS allowed, or XMS not present/full. Try File swap.
;d	XMS nicht erlaubt, oder XMS nicht vorhanden/voll. Datei versuchen.
;
prep_no_xms:
	test	pmethod,XMS_FIRST
	jz	check_file
	jmp	check_ems
;
check_file:
	test	pmethod,USE_FILE
	jnz	prep_do_file
	jmp	prep_no_file
;
prep_do_file:
	push	ds
	IF	ptrsize
	lds	dx,swapfname
	ELSE
	mov	dx,swapfname
	ENDIF
	IFDEF	PASCAL
	inc	dx			;e skip length byte
					;d L�ngenbyte �berspringen
	ENDIF
	mov	cx,2			; hidden attribute
	test	pmethod,HIDE_FILE
	jnz	prep_hide
	xor	cx,cx			; normal attribute
;
prep_hide:
	mov	ah,3ch			; create file
	test	pmethod,CREAT_TEMP
	jz	prep_no_temp
	mov	ah,5ah
;
prep_no_temp:
	int	21h			; create/create temp
	jnc	prep_got_file
	jmp	prep_no_file
;
prep_got_file:
	mov	bx,ax			; handle
;
;e	save the file name
;d	Dateinamen sichern
;
	pop	es
	push	es
	mov	di,offset swap_prep.swapfilename
	mov	cx,81
	mov	si,dx
	rep movsb
;
	pop	ds
	mov	swap_prep.handle,bx
;
;e	preallocate the file
;d	Datei-Speicherplatz pr�-allozieren
;
	test	pmethod,NO_PREALLOC
	jnz	prep_noprealloc
	test	pmethod,CHECK_NET
	jz	prep_nonetcheck
;
;e	check whether file is on a network drive, and don't preallocate
;e	if so. preallocation can slow down swapping significantly when
;e	running on certain networks (Novell)
;d	Pr�fen ob Datei auf einem Netwerk-Laufwerk liegt, und nicht
;d	pr�allozieren wenn ja. Ein Pr�allozieren kann den Swap-Vorgang
;d	erheblich verlangsamen wenn es auf Novell-Drives ausgef�hrt wird.
;
	mov	ax,440ah	; check if handle is remote
	int	21h
	jc	prep_nonetcheck	;e assume not remote if function fails
				;d kein Netz wenn Funktion Fehler liefert
	test	dh,80h		;e DX bit 15 set ?
				;d Ist Bit 15 von DX gesetzt?
	jnz	prep_noprealloc	;e remote if yes
				;d Netzwerk-Datei wenn ja
;
prep_nonetcheck:
	mov	dx,totparas
	mov	cl,4
	rol	dx,cl
	mov	cx,dx
	and	dx,0fff0h
	and	cx,0000fh
	sub	dx,1
	sbb	cx,0
	mov	si,dx			; save
	mov	ax,4200h		; move file pointer, absolute
	int	21h
	jc	prep_file_err
	cmp	dx,cx
	jne	prep_file_err
	cmp	ax,si
	jne	prep_file_err
	mov	cx,1			;e write 1 byte
					;d 1 Byte schreiben
	mov	ah,40h
	int	21h
	jc	prep_file_err
	cmp	ax,cx
	jne	prep_file_err
;
	mov	ax,4200h		; move file pointer, absolute
	xor	dx,dx
	xor	cx,cx			;e rewind to beginning
					;d Auf Anfang zur�ckpositionieren
	int	21h
	jc	prep_file_err
;
prep_noprealloc:
	mov	ax,USE_FILE
	mov	swap_prep.swapmethod,al
	ret
;
prep_file_err:
	mov	ah,3eh			; close file
	int	21h
	mov	dx,offset swap_prep.swapfilename
	mov	ah,41h			; delete file
	int	21h
;
prep_no_file:
	mov	ax,-1
	mov	swap_prep.swapmethod,al
	ret
;
prep_swap	endp
;
	end

