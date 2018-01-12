
   .386
   ifndef   ??version
   endm
   endif
_TEXT segment dword public use32 'CODE'
_TEXT ends
_DATA segment dword public use32 'DATA'
_DATA ends
_BSS  segment dword public use32 'BSS'
_BSS  ends
   .model   FLAT
   assume   cs: FLAT, ds: FLAT, ss: FLAT, es: FLAT
_TEXT segment dword public use32 'CODE'
c@ label byte
NORMALIZE   proc  near
   ;
   ;   pascal normalize(char *s)
   ;
@6@21:
   PUSH  EBP
   MOV   EBP,ESP
   PUSH  ESI
   PUSH  EDI

   MOV   ESI,dword ptr [EBP+8]
   MOV   EDI,ESI

   cld

l1:
   lodsb
   cmp al,8dh
   je l1
   cmp al,0ah
   je l1
   or al,0
   jz l3
   stosb
   jmp l1

l3:     stosb

   pop EDI
   pop ESI
   pop EBP

   ret 4

@16@0:
NORMALIZE   endp
_TEXT ends
_s@   equ   s@
   public   NORMALIZE
_DATA segment dword public use32 'DATA'
d@ label byte
d@w   label word
d@d   label dword
s@ label byte
_DATA ends
_BSS  segment dword public use32 'BSS'
b@ label byte
b@w   label word
b@d   label dword
_BSS  ends
   end
